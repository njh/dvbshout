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
shout_multicast_t shout_multicast;

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
	
		fprintf(stderr,"  %d: %s\n", channels[i]->pid, channels[i]->name);
		pesFilterParams.pid     = channels[i]->pid;
		pesFilterParams.input   = DMX_IN_FRONTEND;
		pesFilterParams.output  = DMX_OUT_TS_TAP;
		pesFilterParams.pes_type = DMX_PES_OTHER;
		pesFilterParams.flags   = DMX_IMMEDIATE_START;
		
		if (ioctl(channels[i]->fd, DMX_SET_PES_FILTER, &pesFilterParams) < 0)  {
			fprintf(stderr,"Failed to set filter for %i: ",channels[i]->pid);
			perror("DMX SET PES FILTER");
		}
	}
	
	fprintf(stderr,"\n");
}



static void connect_server_channel( shout_channel_t *chan )
{
	shout_t *shout =  chan->shout;
	char string[STR_BUF_SIZE];
	int result;


	// Don't connect?
	if (strlen(shout_server.host)==0) {
		return;
	}

	// Already connected to server?
	if (shout_get_connected( shout ) == SHOUTERR_CONNECTED) {
		fprintf(stderr, "  Disconnecting from server.\n");
		shout_close( shout );
	}


	// Set server parameters
	shout_set_agent( shout, PACKAGE_STRING );
	shout_set_host( shout, shout_server.host );
	shout_set_port( shout, shout_server.port );
	shout_set_user( shout, shout_server.user );
	shout_set_password( shout, shout_server.password );
	shout_set_protocol( shout, shout_server.protocol );
	shout_set_format( shout, SHOUT_FORMAT_MP3 );


	// Add information about the audio format
	snprintf(string, STR_BUF_SIZE, "%d", chan->mpah.bitrate );
	shout_set_audio_info( shout, SHOUT_AI_BITRATE, string);
	
	snprintf(string, STR_BUF_SIZE, "%d", chan->mpah.samplerate );
	shout_set_audio_info( shout, SHOUT_AI_SAMPLERATE, string);
	
	snprintf(string, STR_BUF_SIZE, "%d", chan->mpah.channels );
	shout_set_audio_info( shout, SHOUT_AI_CHANNELS, string);
	
	
	// Connect!
	fprintf(stderr, "  Connecting to: http://%s:%d%s\n",
		shout_get_host( shout ), shout_get_port( shout ), shout_get_mount( shout ));
		
	result = shout_open( shout );
	if (result != SHOUTERR_SUCCESS) {
		fprintf(stderr,"  Failed to connect to server: %s.\n", shout_get_error(shout));
		exit(-1);
	}
		
}


static RtpSession * create_rtp_session( shout_channel_t *chan )
{
	RtpSession *session;
	
	// Don't setup multicast if it isnt desired
	if (strlen(chan->multicast_ip)==0) return NULL;
	
	// Connect!
	fprintf(stderr, "  Multicast session: %s/%d/%d\n",
		chan->multicast_ip, chan->multicast_port, chan->multicast_ttl );	
	
	// Create new session
	session=rtp_session_new(RTP_SESSION_SENDONLY);
	rtp_session_set_remote_addr(session, chan->multicast_ip, chan->multicast_port);
	rtp_session_set_payload_type(session, RTP_MPEG_AUDIO_PT);

	return session;	
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

static void ts_continuity_check( shout_channel_t *chan, int ts_cc ) 
{
	if (chan->continuity_count != ts_cc) {
	
		// Only display an error after we gain sync
		if (chan->synced) {
			fprintf(stderr, "Warning: TS continuity error (pid: %d)\n", chan->pid );
		}
		chan->continuity_count = ts_cc;
	}

	chan->continuity_count++;
	if (chan->continuity_count==16)
		chan->continuity_count=0;
}

static void extract_pes_payload( unsigned char *pes_ptr, size_t pes_len, shout_channel_t *chan, int start_of_pes ) 
{
	unsigned char* es_ptr=NULL;
	size_t es_len=0;
	
	
	// Start of a PES header?
	if ( start_of_pes ) {
		
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
	
		// Scan through Elementary Stream (ES) 
		// and try and find MPEG audio stream header
		while (!chan->synced && es_len>=4) {
		
			// Valid header?
			if (mpa_header_parse(es_ptr, &chan->mpah)) {

				// Now we know bitrate etc, set things up
				fprintf(stderr, "Synced to MPEG audio for '%s' (pid: %d)\n",  chan->name, chan->pid );
				mpa_header_print( &chan->mpah );
				chan->synced = 1;
				
				// Work out how big payload will be
				if (chan->multicast_mtu < chan->mpah.framesize) {
					fprintf(stderr, "Error: audio frame size is bigger than packet MTU.\n");
					exit(-1);
				} else {
					chan->frames_per_packet = ( chan->multicast_mtu / chan->mpah.framesize );
					chan->payload_size = chan->frames_per_packet * chan->mpah.framesize;
					fprintf(stderr, "  RTP payload size: %d (%d frames of audio)\n", 
						chan->payload_size, chan->frames_per_packet );
				}
				
				
				// Allocate buffer to store packet in
				chan->buf_size = chan->payload_size + TS_PACKET_SIZE + 4;
				chan->buf = realloc( chan->buf, chan->buf_size );
				chan->buf[0] = 0x00;
				chan->buf[1] = 0x00;
				chan->buf[2] = 0x00;
				chan->buf[3] = 0x00;
				chan->buf += 4;
				if (chan->buf==NULL) {
					perror("Error: Failed to allocate memory for MPEG Audio buffer");
					exit(-1);
				}
				
				
				// (re-)connect to the server
				connect_server_channel( chan );
				
				// Create multicast session
				if (!chan->sess) 
					chan->sess = create_rtp_session( chan );
				
				fprintf(stderr, "\n");
				
			} else {
				// Skip byte
				es_len--;
				es_ptr++;
			}
		}
		
		
		// If stream is synced then put data info buffer
		if (chan->synced) {
		
			// Check that there is space
			if (chan->buf_used + es_len > chan->buf_size) {
				fprintf(stderr, "Error: MPEG Audio buffer overflow\n" );
				exit(-1);
			}
		
			// Copy data into the buffer
			memcpy( chan->buf + chan->buf_used, es_ptr, es_len);
			chan->buf_used += es_len;
		}
	}
	
	
	// Got enough to send packet?
	if (chan->buf_used > chan->payload_size ) {
		int result;
		
		// Ensure a MPEG Audio frame starts here
		if (chan->buf[0] != 0xFF) {
			fprintf(stderr, "Warning: lost MPEG Audio sync for PID %d.\n", chan->pid);
			chan->synced = 0;
			chan->buf_used = 0;
			return;
		}
		
	
		// Send the data to the shoutcast server
		if (1) {
			result = shout_send_raw( chan->shout, chan->buf, chan->payload_size );
			if (result < 0) {
				fprintf(stderr, "Error: failed to send data to server for PID %d.\n", chan->pid);
				fprintf(stderr, "  libshout: %s.\n", shout_get_error(chan->shout));
				exit(-1);
			}
		}
		
		// Send to multicast session
		if( chan->sess ) {
			rtp_session_send_with_ts(chan->sess, (char*)chan->buf-4, chan->payload_size+4, chan->multicast_ts);
			
			// Timestamp for MPEG Audio is based on fixed 90kHz clock rate
			chan->multicast_ts += ((chan->mpah.samples * 90000) / chan->mpah.samplerate)
			                * chan->frames_per_packet;

		}
		
		// Move any remaining memory to the start of the buffer
		chan->buf_used -= chan->payload_size;
		memmove( chan->buf, chan->buf+chan->payload_size, chan->buf_used );
		
	}
}



void process_ts_packets( int fd_dvr )
{
	unsigned char buf[TS_PACKET_SIZE];
	unsigned char* pes_ptr=NULL;
	unsigned int pid=0;
	size_t pes_len;
	int bytes_read;

	while ( !Interrupted ) {
		
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
			pes_ptr += (TS_PACKET_ADAPT_LEN(buf) + 1);
			pes_len -= (TS_PACKET_ADAPT_LEN(buf) + 1);
		}


		// Check we know about the payload
		if (channel_map[ pid ]) {
			// Continuity check
			ts_continuity_check( channel_map[ pid ], TS_PACKET_CONT_COUNT(buf) );
		
			// Extract PES payload and put it in FIFO
			extract_pes_payload( pes_ptr, pes_len, channel_map[ pid ], TS_PACKET_PAYLOAD_START(buf) );
			
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
	
	// Default server settings
	shout_server.port = SERVER_PORT_DEFAULT;
	strcpy(shout_server.user, SERVER_USER_DEFAULT);
	strcpy(shout_server.password, SERVER_PASSWORD_DEFAULT);
	shout_server.protocol = SERVER_PROTOCOL_DEFAULT;
	
	// Default settings defaults
	shout_multicast.ttl = MULTICAST_TTL_DEFAULT;
	shout_multicast.port = MULTICAST_PORT_DEFAULT;
	strcpy(shout_multicast.interface, MULTICAST_INTERFACE_DEFAULT);
	shout_multicast.mtu = MULTICAST_MTU_DEFAULT;


	// Initialise libshout
	shout_init();

	// Initialise ortp
	ortp_init();
	
	// Parse command line arguments
	parse_args( argc, argv );
	

	

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
	

	// Setup signal handlers
	if (signal(SIGHUP, signal_handler) == SIG_IGN) signal(SIGHUP, SIG_IGN);
	if (signal(SIGINT, signal_handler) == SIG_IGN) signal(SIGINT, SIG_IGN);
	if (signal(SIGTERM, signal_handler) == SIG_IGN) signal(SIGTERM, SIG_IGN);


  	// Read and process TS packets from DVR device
	process_ts_packets( fd_dvr );
	

	if (Interrupted) {
		fprintf(stderr,"Caught signal %d - closing cleanly.\n",Interrupted);
	}
	
	
	// Clean up
	for (i=0;i<channel_count;i++) {
		if (channels[i]->fd != -1) close(channels[i]->fd);
		if (channels[i]->buf) free( channels[i]->buf );
		if (channels[i]->sess) rtp_session_destroy(channels[i]->sess);
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

	// Shutdown oRTP
	ortp_exit();	

	return(0);
}

