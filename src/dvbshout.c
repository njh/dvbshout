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


static void set_ts_filters()
{
	struct dmx_pes_filter_params pesFilterParams;
	int i;
	
	fprintf(stderr,"Setting PID filters:\n");

	for (i=0;i<channel_count;i++) {
	
		fprintf(stderr,"  %d: %s\n", channels[i]->apid, channels[i]->name);
		pesFilterParams.pid     = channels[i]->apid;
		pesFilterParams.input   = DMX_IN_FRONTEND;
		pesFilterParams.output  = DMX_OUT_TS_TAP;
		pesFilterParams.pes_type = DMX_PES_OTHER;
		pesFilterParams.flags   = DMX_IMMEDIATE_START;
		
		if (ioctl(channels[i]->fd, DMX_SET_PES_FILTER, &pesFilterParams) < 0)  {
			fprintf(stderr,"Failed to set filter for %i: ",channels[i]->apid);
			perror("DMX SET PES FILTER");
		}
	}
	
	fprintf(stderr,"\n");
}


static const char* shout_protocol_name(int protocol)
{
	switch(protocol) {
		case SHOUT_PROTOCOL_HTTP: return "Icecast 2";
		case SHOUT_PROTOCOL_XAUDIOCAST: return "Icecast 1";
		case SHOUT_PROTOCOL_ICY: return "ShoutCast";
		default: return "Unknown";
	}
}


static void connect_shout_channel( shout_channel_t *chan )
{
	shout_t *shout =  chan->shout;
	int result;
	

	// Set server parameters
	shout_set_host( shout, shout_server.host );
	shout_set_port( shout, shout_server.port );
	shout_set_user( shout, shout_server.user );
	shout_set_password( shout, shout_server.password );
	shout_set_protocol( shout, shout_server.protocol );
	shout_set_format( shout, SHOUT_FORMAT_MP3 );
	
	// Connect!
	fprintf(stderr, "  http://%s:%d%s\n",
		shout_get_host( shout ), shout_get_port( shout ), shout_get_mount( shout ));
		
	result = shout_open( shout );
	if (result != SHOUTERR_SUCCESS) {
		fprintf(stderr,"  Failed to connect to server: %s.\n", shout_get_error(shout));
		exit(-1);
	}
		
}


static void parse_args(int argc, char **argv) 
{
	if (argc>1) {
	
 		parse_config( argv[1] );

 	} else {
		fprintf(stderr,"%s version %s\n", PACKAGE_NAME, PACKAGE_VERSION);
		fprintf(stderr,"Usage: dvbshout <configfile>\n\n");
		exit(-1);
	}
}

void process_ts_packets( int fd_dvr )
{
	unsigned char buf[TS_PACKET_SIZE];
	unsigned char* pes_ptr=NULL;
	size_t pes_len;
	int bytes_read;

	while ( !Interrupted ) {
		unsigned int pid=0;
		
		bytes_read = read(fd_dvr,buf,TS_PACKET_SIZE);
		if (bytes_read==0) continue;
		if (bytes_read!=TS_PACKET_SIZE) {
			fprintf(stderr,"No bytes left to read - aborting\n");
			break;
		}
		
		// Check the sync-byte
		if (TS_PACKET_SYNC_BYTE(buf) != 0x47) {
			fprintf(stderr,"Lost syncronisation - aborting\n");
			break;
		}
		
		// Get the PID of this TS packet
		pid = TS_PACKET_PID(buf);
		
		// Transport error?
		if ( TS_PACKET_TRANS_ERROR(buf) ) {
			fprintf(stderr, "Transport error in PID %d.\n", pid);
		}			

		// Scrambled?
		if ( TS_PACKET_SCRAMBLING(buf) ) {
			fprintf(stderr, "Error: PID %d is scrambled.\n", pid);
			break;
		}	

		// Continuity check
		// FIXME:
		//fprintf(stderr, "TS_PACKET_CONT_COUNT: %d.\n", TS_PACKET_CONT_COUNT(buf) );

		// Location of and size of PES payload
		pes_ptr = &buf[4];
		pes_len = TS_PACKET_SIZE - 4;

		// Check for adaptation field?
		if (TS_PACKET_ADAPTATION(buf)==0x1) {
			// Payload only, no adaptation field
		} else if (TS_PACKET_ADAPTATION(buf)==0x2) {
			// Adaptation field only, no payload
			continue;
		} else if (TS_PACKET_ADAPTATION(buf)==0x3) {
			// Adaptation field AND payload
			fprintf(stderr, "** Adaptation field AND payload **\n" );
			// FIXME:
			// pes_ptr += 
			// pes_len -= 
		}

		// Output the TS
		//if (pid == 2314) {
		//	fwrite( buf, TS_PACKET_SIZE, 1, stdout );
		//}

		// Check we know about the payload
		if (channel_map[ pid ]) {
			shout_channel_t *chan = channel_map[ pid ];
			unsigned char* es_ptr=NULL;
			size_t es_len=0;
			
			
			// Start of a PES header?
			if ( TS_PACKET_PAYLOAD_START(buf) ) {
				
				// Parse the PES header
				es_ptr = parse_pes( pes_ptr, pes_len, &es_len, chan );
									
			} else if (chan->stream_id) {
			
				// Don't output any data until we have seen a PES header
				es_ptr = pes_ptr;
				es_len = pes_len;
			
				// Are we are the end of the PES packet?
				if (es_len>chan->pes_remaining) {
					es_len=chan->pes_remaining;
				}
				
			}
			
			// Subtract the amount remaining in current PES packet
			chan->pes_remaining -= es_len;

			
			// Got some data to write out?
			if (es_ptr) {
			
				// Try and sync MPEG audio stream
				while (!chan->synced && es_len>=4) {
				
					// Valid header?
					if (mpa_header_parse(es_ptr, &chan->mpah)) {
					
						fprintf(stderr, "Found start of MPEG audio header for pid %d\n", pid );
						chan->synced = 1;
						mpa_header_print( &chan->mpah );
						
					} else {
						es_len--;
						es_ptr++;
					}
				}
				
				
				// If stream is syced then put data info FIFO
				if (chan->synced) {
				
					// Create FIFO
					if (chan->fifo==NULL) {
						chan->fifo = malloc( sizeof( sfifo_t ) );
						if (chan->fifo == NULL) {
							perror("failed to allocate memory for sfifo_t");
							exit(-1);
						}
						
						// Enough for two frames of MPEG audio
						if (sfifo_init( chan->fifo, chan->mpah.framesize * 2 ))
						{
							fprintf(stderr, "failed to initialise FIFO\n");
							exit(-1);
						}
					}
				
					// Write data to FIFO
					if (sfifo_write( chan->fifo, es_ptr, es_len) != es_len) {
						fprintf(stderr, "Failed to write to FIFO.\n" );
					}
				}
			}
			
			
			// Got enough to send packet?
			if (chan->fifo && sfifo_used( chan->fifo ) > chan->mpah.framesize ) {
				unsigned char mybuf[2048];
				int framesize = chan->mpah.framesize;
				int result;
				
				//fprintf(stderr, "Got enough for a frame of audio.\n");

				if (sfifo_read( chan->fifo, mybuf, framesize ) != framesize)
				{
					fprintf(stderr, "Failed to read from FIFO.\n" );
				}
				
				// Does server need connecting ?
				if (shout_get_connected( chan->shout ) == SHOUTERR_UNCONNECTED) {
					connect_shout_channel( chan );
				}
			
				
				result = shout_send_raw( chan->shout, mybuf, framesize );
				if (result < 0) {
					fprintf(stderr, "Error: failed to send data to server for PID %d.\n", pid);
					fprintf(stderr, "  libshout: %s.\n", shout_get_error(chan->shout));
					break;
				}

				//fwrite( es_ptr, es_len, 1, stdout );
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
	set_ts_filters();
	


  	// Read and process TS packets from DVR device
  	fprintf(stderr,"Running.\n");
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

