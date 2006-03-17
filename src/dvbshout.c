/* 

	dvbshout.c
	(C) Dave Chapman <dave@dchapman.com> 2001, 2002.
	(C) Nicholas J Humfrey <njh@aelius.com> 2006.
	
	Copyright notice:
	
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
    
*/


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include "dvbshout.h"
#include "config.h"



int Interrupted=0;

int channel_count=0;
fe_settings_t *fe_set = NULL;
shout_channel_t *channel_map[MAX_PID_COUNT];
shout_channel_t *channels[MAX_CHANNEL_COUNT];
shout_server_t shout_server;


char* frontenddev[4]={"/dev/dvb/adapter0/frontend0","/dev/dvb/adapter1/frontend0","/dev/dvb/adapter2/frontend0","/dev/dvb/adapter3/frontend0"};
char* dvrdev[4]={"/dev/dvb/adapter0/dvr0","/dev/dvb/adapter1/dvr0","/dev/dvb/adapter2/dvr0","/dev/dvb/adapter3/dvr0"};
char* demuxdev[4]={"/dev/dvb/adapter0/demux0","/dev/dvb/adapter1/demux0","/dev/dvb/adapter2/demux0","/dev/dvb/adapter3/demux0"};




static fe_settings_t * init_fe_settings()
{
	fe_settings_t *set = malloc( sizeof(fe_settings_t));
	
	set->card = 0;
	set->freq = 0;
	set->srate = 27500000;
	set->polarity = 'V';
	set->tone = -1;
	
	set->diseqc=0;
	set->spec_inv=INVERSION_AUTO;
	set->modulation=CONSTELLATION_DEFAULT;
	set->transmission_mode=TRANSMISSION_MODE_DEFAULT;
	set->bandwidth=BANDWIDTH_DEFAULT;
	set->guard_interval=GUARD_INTERVAL_DEFAULT;
	set->code_rate=CODERATE_DEFAULT;
	
	return set;
}




static void signal_handler(int signum)
{
	if (signum != SIGPIPE) {
		Interrupted=signum;
	}
	signal(signum,signal_handler);
}


static void set_ts_filter(int fd, unsigned short pid)
{
	struct dmx_pes_filter_params pesFilterParams;
	
	fprintf(stderr,"Setting filter for PID %d\n",pid);
	pesFilterParams.pid     = pid;
	pesFilterParams.input   = DMX_IN_FRONTEND;
	pesFilterParams.output  = DMX_OUT_TS_TAP;
	pesFilterParams.pes_type = DMX_PES_OTHER;
	pesFilterParams.flags   = DMX_IMMEDIATE_START;
	
	if (ioctl(fd, DMX_SET_PES_FILTER, &pesFilterParams) < 0)  {
		fprintf(stderr,"FILTER %i: ",pid);
		perror("DMX SET PES FILTER");
	}
}


static void connect_shout_channels()
{
	int i;
	
	for( i=0; i< channel_count; i++ ) {
		shout_channel_t *chan =  channels[ i ];
		shout_t *shout =  chan->shout;
		
		// Invalidate the file descriptor for the channel
		chan->fd = -1;

			
		shout_set_host( shout, shout_server.host );
		shout_set_port( shout, shout_server.port );
		shout_set_user( shout, shout_server.user );
		shout_set_password( shout, shout_server.password );
		shout_set_protocol( shout, shout_server.protocol );
		shout_set_format( shout, SHOUT_FORMAT_MP3 );
		
	}

}

static unsigned char* parse_pes( unsigned char* buf, int size, size_t *payload_size, shout_channel_t *chan) 
{
	size_t pes_len = (buf[4] << 8) + buf[5];
	size_t pes_header_len = buf[8];
	unsigned char pes_flag_scrambled = (buf[6] & 0x30) >> 4;
	unsigned char pes_flag_priority = (buf[6] & 0x08) >> 3;
	unsigned char pes_flag_alignment = (buf[6] & 0x04) >> 2;
	unsigned char pes_flag_copyright = (buf[6] & 0x02) >> 1;
	unsigned char pes_flag_original = (buf[6] & 0x01) >> 0;
	
	unsigned char pes_flag_pts_dts = (buf[7] & 0xC0) >> 6;
	unsigned char pes_flag_escr = (buf[7] & 0x20) >> 5;
	unsigned char pes_flag_esr = (buf[7] & 0x10) >> 4;
	unsigned char pes_flag_dsm_trick = (buf[7] & 0x8) >> 3;
	unsigned char pes_flag_add_copy = (buf[7] & 0x4) >> 2;
	unsigned char pes_flag_crc = (buf[7] & 0x2) >> 1;
	unsigned char pes_flag_exten = (buf[7] & 0x1) >> 0;
	int stream_id = buf[3];
	
	
	if( buf[0] != 0 || buf[1] != 0 || buf[2] != 1 )
	{
		fprintf(stderr, "Invalid PES header (pid: %d).\n", chan->apid);
		return 0;
	}
	
	// 0xC0 = First MPEG-2 audio steam
	if( stream_id != 0xC0 )
	{
		fprintf(stderr, "Ignoring stream with ID 0x%x (pid: %d).\n", stream_id, chan->apid);
		return 0;
	}
	
	// only keep the first stream we see
	chan->stream_id = stream_id;
	
	// Check PES Extension header 
	if( ( buf[6]&0xC0 ) != 0x80 )
	{
		fprintf(stderr, "Error: invalid MPEG-2 PES extension header (pid: %d).\n", chan->apid);
		return 0;
	}

	if( pes_flag_scrambled )
	{
		fprintf(stderr, "Error: PES payload is scrambled (pid: %d).\n", chan->apid);
		return 0;
	}

/*	
	fprintf(stderr, "pid: %d, length: %d header_len: %d\n", chan->apid, pes_len, pes_header_len);
	fprintf(stderr, "  pes_flag_priority=%d\n", pes_flag_priority );
	fprintf(stderr, "  pes_flag_alignment=%d\n", pes_flag_alignment );
	fprintf(stderr, "  pes_flag_copyright=%d\n", pes_flag_copyright );
	fprintf(stderr, "  pes_flag_original=%d\n", pes_flag_original );
	fprintf(stderr, "  pes_flag_pts_dts=0x%x\n", pes_flag_pts_dts );
	fprintf(stderr, "  pes_flag_escr=%d\n", pes_flag_escr );
	fprintf(stderr, "  pes_flag_esr=%d\n", pes_flag_esr );
	fprintf(stderr, "  pes_flag_dsm_trick=%d\n", pes_flag_dsm_trick );
	fprintf(stderr, "  pes_flag_add_copy=%d\n", pes_flag_add_copy );
	fprintf(stderr, "  pes_flag_crc=%d\n", pes_flag_crc );
	fprintf(stderr, "  pes_flag_exten=%d\n", pes_flag_exten );
*/

	// Make note of the amount of PES payload remaining
	chan->pes_remaining = pes_len-(size-6-pes_header_len);
	chan->ts_count = 1;

	// Return pointer and length of payload
	*payload_size = size-(6+pes_header_len);
	
	return buf+(6+pes_header_len);
}



static void parse_args(int argc, char **argv) 
{
	if (1) {
	
 		parse_config( "/home/njh/dvbshout/dvbshout.conf" );

 	} else {
		fprintf(stderr,"%s version %s\n", PACKAGE_NAME, PACKAGE_VERSION);
		fprintf(stderr,"Usage: dvbshout [OPTIONS]\n\n");
		fprintf(stderr,"\t-c <config> Location of configuration file.\n");
		
		exit(-1);
	}
}

void process_ts_packets( int fd_dvr )
{
	unsigned char buf[TS_PACKET_SIZE];
	unsigned char* pes_ptr=NULL;
	unsigned int pes_len;
	int bytes_read;

	while ( !Interrupted) {
		unsigned int pid=0, offset=0;
		
		bytes_read = read(fd_dvr,buf,TS_PACKET_SIZE);
		if (bytes_read==0) continue;
		if (bytes_read!=TS_PACKET_SIZE) {
			fprintf(stderr,"No bytes left to read - aborting\n");
			break;
		}
		
		// Check the sync-byte
		if (buf[0] != 0x47) {
			fprintf(stderr,"Lost syncronisation - aborting\n");
			break;
		}
		
		// Get the PID of this TS packet
		pid = (buf[1] & 0x1F)<<8;
		pid |= buf[2];
		
		// Check there is a payload
		if (!(buf[3]&0x10))
			continue;
			

		// Location of and size of PES payload
		pes_ptr = &buf[4];
		pes_len = TS_PACKET_SIZE - 4;

		// Check for adaptation field?
		if ( buf[3] & 0x20) {
			pes_ptr += buf[4] + 1;
			pes_len -= buf[4] + 1;
			fprintf(stderr,"offset=%d\n", offset);
		}

		// Transport error?
		if ( buf[1]&0x90) {
			fprintf(stderr, "Transport error in PID %d.\n", pid);
		}

		// Check we know about the payload
		if (channel_map[ pid ]) {
			shout_channel_t *chan = channel_map[ pid ];
			unsigned char* es_ptr=NULL;
			size_t es_len=0;
			
	
			// Start of a PES header?
			if ( buf[1]&0x40) {
			
				es_ptr = parse_pes( pes_ptr, pes_len, &es_len, chan );
					
			} else {
			
				if (chan->stream_id) {
					// Don't output any data until we have seen a PES header
					es_ptr = pes_ptr;
					es_len = TS_PACKET_SIZE-4;
					chan->ts_count++;
					
					if (es_len>chan->pes_remaining) {
						es_len-=3;
						fprintf(stderr, "ts_count=%d es_len=%d pes_remaining=%d\n", chan->ts_count, es_len-3, chan->pes_remaining);
					}
				}
			}
			
			// Got some data to write out?
			if (es_ptr && es_len) {
				chan->pes_remaining -= es_len;
				fwrite( es_ptr, es_len, 1, stdout );
			}
			
		} else {
			fprintf(stderr, "Error: don't know anything about PID %d.\n", pid);
		}

	}
}



int main(int argc, char **argv)
{
	int fd_frontend=-1;
	int fd_dvr=-1;
	int i;
	
	
	// Initialise data structures
	fe_set = init_fe_settings();
	for (i=0;i<MAX_PID_COUNT;i++) channel_map[i]=NULL;
	for (i=0;i<MAX_CHANNEL_COUNT;i++) channels[i]=NULL;
	memset( &shout_server, 0, sizeof(shout_server_t) );
	shout_server.protocol = SHOUT_PROTOCOL_HTTP;


	// Initialise libshout
	shout_init();
	
	
	// Parse command line arguments
	parse_args( argc, argv );
	
	
	if (signal(SIGHUP, signal_handler) == SIG_IGN) signal(SIGHUP, SIG_IGN);
	if (signal(SIGINT, signal_handler) == SIG_IGN) signal(SIGINT, SIG_IGN);
	if (signal(SIGTERM, signal_handler) == SIG_IGN) signal(SIGTERM, SIG_IGN);

	

	// Open the Frontend
	if((fd_frontend = open(frontenddev[fe_set->card],O_RDWR)) < 0){
		perror("Failed to open frontend device");
		return -1;
	}

    // Tune in the frontend
	if ((fe_set->freq!=0) && (fe_set->polarity!=0)) {
		int err =tune_it(fd_frontend, fe_set);
		if (err<0) { exit(err); }
	} else {
		fprintf(stderr,"Not tuning-in frontend.\n");
	}

	
	// Open demux device for each of the channels
	for (i=0;i<channel_count;i++) {  
		if((channels[i]->fd = open(demuxdev[fe_set->card],O_RDWR)) < 0){
			fprintf(stderr,"FD %i: ",i);
			perror("Failed to open demux device");
			return -1;
		}
	}
	
	// Open the DRV device
	if((fd_dvr = open(dvrdev[fe_set->card],O_RDONLY)) < 0){
		perror("Failed to open DVR device");
		return -1;
	}

	// Now we set the filters
	for (i=0;i<channel_count;i++) {
		set_ts_filter(channels[i]->fd,channels[i]->apid);
	}

	fprintf(stderr,"Streaming %d channel%s.\n",channel_count,(channel_count==1 ? "" : "s"));
  
  
  	// Connect to the Icecast server
 	connect_shout_channels();
  
  
  	// Read and process TS packets from DVR device
	process_ts_packets( fd_dvr );
	

	if (Interrupted) {
		fprintf(stderr,"Caught signal %d - closing cleanly.\n",Interrupted);
	}
	
	
	// Clean up
	for (i=0;i<channel_count;i++) {
		if (channels[i]->fd != -1) close(channels[i]->fd);
		if (channels[i]->shout) {
			shout_close( channels[i]->shout );
			shout_free( channels[i]->shout );
		}
		free( channels[i] );
	}
	close(fd_dvr);
	close(fd_frontend);
	if (fe_set) free( fe_set );

	// Shutdown libshout
	shout_shutdown();	

	return(0);
}

