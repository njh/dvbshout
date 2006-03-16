/* 

	dvbshout.c
	(C) Dave Chapman <dave@dchapman.com> 2001, 2002.
	(C) Nichoals J Humfrey <njh@aelius.com> 2006.
	
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


#include "transform.h"
#include "remux.h"
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

static long getmsec()
{
	struct timeval tv;
	gettimeofday(&tv,(struct timezone*) NULL);
	return(tv.tv_sec%1000000)*1000 + tv.tv_usec/1000;
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





static void usage() 
{
    fprintf(stderr,"%s version %s\n", PACKAGE_NAME, PACKAGE_VERSION);
    fprintf(stderr,"Usage: dvbshout [OPTIONS]\n\n");
    fprintf(stderr,"\t-c <config> Location of configuration file.\n");
    
    exit(-1);
}

/*
static int parse_args( int argc, char **argv)
{
	long start_time=-1;
	long end_time=-1;
	int i;
	char *ch;
	
    npids=0;
    
    fe_set = init_fe_settings();
    
    
    for (i=1;i<argc;i++) {
		if (strcmp(argv[i],"-f")==0) {
        i++;
        fe_set->freq=atoi(argv[i])*1000UL;
      } else if (strcmp(argv[i],"-p")==0) {
        i++;
        if (argv[i][1]==0) {
          if (tolower(argv[i][0])=='v') {
            fe_set->polarity='V';
          } else if (tolower(argv[i][0])=='h') {
            fe_set->polarity='H';
          }
        }
      } 
      else if (strcmp(argv[i],"-s")==0) {
        i++;
        fe_set->srate=atoi(argv[i])*1000UL;
      } 
      else if (strcmp(argv[i],"-D")==0) 
      {
        i++;
        fe_set->diseqc=atoi(argv[i]);
        if(fe_set->diseqc < 0 || fe_set->diseqc > 4) fe_set->diseqc = 0;
      } 
      else if (strcmp(argv[i],"-o")==0) {
        to_stdout = 1;
      } else if (strcmp(argv[i],"-n")==0) {
        i++;
        secs=atoi(argv[i]);
      } else if (strcmp(argv[i],"-c")==0) {
        i++;
        card=atoi(argv[i]);
        if ((card < 0) || (card > 3)) {
          fprintf(stderr,"ERROR: card parameter must be between 0 and 4\n");
        }
      } else if (strcmp(argv[i],"-qam")==0) {
        i++;
        switch(atoi(argv[i])) {
          case 16:  fe_set->modulation=QAM_16; break;
          case 32:  fe_set->modulation=QAM_32; break;
          case 64:  fe_set->modulation=QAM_64; break;
          case 128: fe_set->modulation=QAM_128; break;
          case 256: fe_set->modulation=QAM_256; break;
          default:
            fprintf(stderr,"Invalid QAM rate: %s\n",argv[i]);
            exit(0);
        }
      } else if (strcmp(argv[i],"-gi")==0) {
        i++;
        switch(atoi(argv[i])) {
          case 32:  fe_set->guard_interval=GUARD_INTERVAL_1_32; break;
          case 16:  fe_set->guard_interval=GUARD_INTERVAL_1_16; break;
          case 8:   fe_set->guard_interval=GUARD_INTERVAL_1_8; break;
          case 4:   fe_set->guard_interval=GUARD_INTERVAL_1_4; break;
          default:
            fprintf(stderr,"Invalid Guard Interval: %s\n",argv[i]);
            exit(0);
        }
      } else if (strcmp(argv[i],"-tm")==0) {
        i++;
        switch(atoi(argv[i])) {
          case 8:   fe_set->transmission_mode=TRANSMISSION_MODE_8K; break;
          case 2:   fe_set->transmission_mode=TRANSMISSION_MODE_2K; break;
          default:
            fprintf(stderr,"Invalid Transmission Mode: %s\n",argv[i]);
            exit(0);
        }
      } else if (strcmp(argv[i],"-bw")==0) {
        i++;
        switch(atoi(argv[i])) {
          case 8:   fe_set->bandwidth=BANDWIDTH_8_MHZ; break;
          case 7:   fe_set->bandwidth=BANDWIDTH_7_MHZ; break;
          case 6:   fe_set->bandwidth=BANDWIDTH_6_MHZ; break;
          default:
            fprintf(stderr,"Invalid DVB-T bandwidth: %s\n",argv[i]);
            exit(0);
        }
      } else if (strcmp(argv[i],"-cr")==0) {
        i++;
        if (!strcmp(argv[i],"AUTO")) {
          fe_set->code_rate=FEC_AUTO;
        } else if (!strcmp(argv[i],"1_2")) {
          fe_set->code_rate=FEC_1_2;
        } else if (!strcmp(argv[i],"2_3")) {
          fe_set->code_rate=FEC_2_3;
        } else if (!strcmp(argv[i],"3_4")) {
          fe_set->code_rate=FEC_3_4;
        } else if (!strcmp(argv[i],"5_6")) {
          fe_set->code_rate=FEC_5_6;
        } else if (!strcmp(argv[i],"7_8")) {
          fe_set->code_rate=FEC_7_8;
        } else {
          fprintf(stderr,"Invalid Code Rate: %s\n",argv[i]);
          exit(0);
        }
      } else if (strcmp(argv[i],"-from")==0) {
        i++;
        if (map_cnt) {
          pids_map[map_cnt-1].start_time=atoi(argv[i]);
        } else {
          start_time=atoi(argv[i]);
        }
      } else if (strcmp(argv[i],"-to")==0) {
        i++;
        if (map_cnt) {
          pids_map[map_cnt-1].end_time=atoi(argv[i]);
        } else {
          end_time=atoi(argv[i]);
        }
     } else {
          if ((ch=(char*)strstr(argv[i],":"))!=NULL) {
            pid2=atoi(&ch[1]);
            ch[0]=0;
          } else {
            pid2=-1;
          }
          pid=atoi(argv[i]);
          if (pid) {
            if (npids == MAX_CHANNELS) {
              fprintf(stderr,"\nSorry, you can only set up to %d filters.\n\n",MAX_CHANNELS);
              return(-1);
            } else {
              pids[npids++]=pid;
              if (pid2!=-1) {
                hi_mappids[pid]=pid2>>8;
                lo_mappids[pid]=pid2&0xff;
                fprintf(stderr,"Mapping %d to %d\n",pid,pid2);
              }
            }
          }
        }
      }
 
 	return 0;
 }
*/	



int main(int argc, char **argv)
{
	unsigned char buf[TS_PACKET_SIZE];
	int fd_frontend=-1;
	int fd_dvr=-1;
	int bytes_read;
	int i;
	
	
	// Initialise data structures
	fe_set = init_fe_settings();
	for (i=0;i<MAX_PID_COUNT;i++) channel_map[i]=NULL;
	for (i=0;i<MAX_CHANNEL_COUNT;i++) channels[i]=NULL;
	memset( &shout_server, 0, sizeof(shout_server_t) );

	
	//if (argc<=1) usage();
	//else
	parse_config( "/home/njh/dvbshout/dvbshout.conf" );
	
	
	if (signal(SIGHUP, signal_handler) == SIG_IGN) signal(SIGHUP, SIG_IGN);
	if (signal(SIGINT, signal_handler) == SIG_IGN) signal(SIGINT, SIG_IGN);
	if (signal(SIGTERM, signal_handler) == SIG_IGN) signal(SIGTERM, SIG_IGN);


	// Initialise libshout
	shout_init();
	

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
	for (i=0;i<channel_count;i++) set_ts_filter(channels[i]->fd,channels[i]->apid);


	fprintf(stderr,"Streaming %d channel%s.\n",channel_count,(channel_count==1 ? "" : "s"));
  
  
  
  

	while ( !Interrupted) {
		int pid;
		
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
		
      
      	// Write to STDOUT
      	fwrite( buf, TS_PACKET_SIZE, 1, stdout );


        pid=((buf[1]&0x1f) << 8) | (buf[2]);
        //fprintf(stderr,"pid=%d\n",pid);
        //counts[pid]++;
        
	}
	

	if (Interrupted) {
		fprintf(stderr,"Caught signal %d - closing cleanly.\n",Interrupted);
	}
	
	
	// Clean up
	for (i=0;i<channel_count;i++) {
		close(channels[i]->fd);
		free( channels[i] );
	}
	close(fd_dvr);
	close(fd_frontend);
	if (fe_set) free( fe_set );


	// Shutdown libshout
	shout_shutdown();	

	return(0);
}

