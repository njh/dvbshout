/* 

	dvbstream - RTP-ize a DVB transport stream.
	(C) Dave Chapman <dave@dchapman.com> 2001, 2002.
	
	The latest version can be found at http://www.linuxstb.org/dvbstream
	
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


// Linux includes:
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <resolv.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <values.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

// DVB includes:
#include <linux/dvb/dmx.h>
#include <linux/dvb/frontend.h>

#include "transform.h"
#include "remux.h"
#include "tune.h"


#define PACKET_SIZE 188
#define BUFFER_SIZE 1024
enum {RTP_PS,RTP_TS,RTP_NONE,MAP_TS};


// There seems to be a limit of 16 simultaneous filters in the driver
#define MAX_CHANNELS 16


// How often (in seconds) to update the "now" variable
#define ALARM_TIME 5

int pids[MAX_CHANNELS];
unsigned char hi_mappids[8192];
unsigned char lo_mappids[8192];
int fd_frontend;
int pid,pid2;
int fromlen;
int npids = 0;
int fd[MAX_CHANNELS];
int to_stdout = 0; /* to stdout instead of rtp stream */






/* Signal handling code shamelessly copied from VDR by Klaus Schmidinger 
   - see http://www.cadsoft.de/people/kls/vdr/index.htm */

char* frontenddev[4]={"/dev/dvb/adapter0/frontend0","/dev/dvb/adapter1/frontend0","/dev/dvb/adapter2/frontend0","/dev/dvb/adapter3/frontend0"};
char* dvrdev[4]={"/dev/dvb/adapter0/dvr0","/dev/dvb/adapter1/dvr0","/dev/dvb/adapter2/dvr0","/dev/dvb/adapter3/dvr0"};
char* demuxdev[4]={"/dev/dvb/adapter0/demux0","/dev/dvb/adapter1/demux0","/dev/dvb/adapter2/demux0","/dev/dvb/adapter3/demux0"};

int card=0;
long now;
long real_start_time;
int Interrupted=0;
int output_type=RTP_TS;
int do_analyse=0;
fe_settings_t *fe_set = NULL;
unsigned int secs = 0;

typedef struct {
  char *filename;
  int fd;
  int pids[MAX_CHANNELS];
  int num;
  long start_time; // in seconds
  long end_time;   // in seconds
} pids_map_t;

pids_map_t *pids_map;
int map_cnt;




static fe_settings_t * init_fe_settings( ) {
	fe_settings_t *set = malloc( sizeof(fe_settings_t));
	
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




static void signal_handler(int signum) {
  struct timeval tv;

  if (signum == SIGALRM) {
    gettimeofday(&tv,(struct timezone*) NULL);
    now=tv.tv_sec-real_start_time;
    //    fprintf(stderr,"now=%ld\n",now);
    alarm(ALARM_TIME);
  } else if (signum != SIGPIPE) {
    Interrupted=signum;
  }
  signal(signum,signal_handler);
}

long getmsec() {
	struct timeval tv;
	gettimeofday(&tv,(struct timezone*) NULL);
	return(tv.tv_sec%1000000)*1000 + tv.tv_usec/1000;
}



void set_ts_filter(int fd, unsigned short pid)
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





void usage() 
{
    fprintf(stderr,"Usage: dvbshout [OPTIONS]\n\n");
    fprintf(stderr,"\nStandard tuning options:\n\n");
    fprintf(stderr,"-f freq     absolute Frequency (DVB-S in Hz or DVB-T in Hz)\n");
    fprintf(stderr,"            or L-band Frequency (DVB-S in Hz or DVB-T in Hz)\n");
    fprintf(stderr,"-p [H,V]    Polarity (DVB-S only)\n");
    fprintf(stderr,"-s N        Symbol rate (DVB-S or DVB-C)\n");

    fprintf(stderr,"\nAdvanced tuning options:\n\n");
    fprintf(stderr,"-c [0-3]    Use DVB card #[0-3]\n");
    fprintf(stderr,"-qam X      DVB-T modulation - 16%s, 32%s, 64%s, 128%s or 256%s\n",(CONSTELLATION_DEFAULT==QAM_16 ? " (default)" : ""),(CONSTELLATION_DEFAULT==QAM_32 ? " (default)" : ""),(CONSTELLATION_DEFAULT==QAM_64 ? " (default)" : ""),(CONSTELLATION_DEFAULT==QAM_128 ? " (default)" : ""),(CONSTELLATION_DEFAULT==QAM_256 ? " (default)" : ""));
    fprintf(stderr,"-gi N       DVB-T guard interval 1_N (N=32%s, 16%s, 8%s or 4%s)\n",(GUARD_INTERVAL_DEFAULT==GUARD_INTERVAL_1_32 ? " (default)" : ""),(GUARD_INTERVAL_DEFAULT==GUARD_INTERVAL_1_16 ? " (default)" : ""),(GUARD_INTERVAL_DEFAULT==GUARD_INTERVAL_1_8 ? " (default)" : ""),(GUARD_INTERVAL_DEFAULT==GUARD_INTERVAL_1_4 ? " (default)" : ""));
    fprintf(stderr,"-cr N       DVB-T code rate. N=AUTO%s, 1_2%s, 2_3%s, 3_4%s, 5_6%s, 7_8%s\n",(CODERATE_DEFAULT==FEC_AUTO ? " (default)" : ""),(CODERATE_DEFAULT==FEC_1_2 ? " (default)" : ""),(CODERATE_DEFAULT==FEC_2_3 ? " (default)" : ""),(CODERATE_DEFAULT==FEC_3_4 ? " (default)" : ""),(CODERATE_DEFAULT==FEC_5_6 ? " (default)" : ""),(CODERATE_DEFAULT==FEC_7_8 ? " (default)" : ""));
    fprintf(stderr,"-bw N       DVB-T bandwidth (Mhz) - N=6%s, 7%s or 8%s\n",(BANDWIDTH_DEFAULT==BANDWIDTH_6_MHZ ? " (default)" : ""),(BANDWIDTH_DEFAULT==BANDWIDTH_7_MHZ ? " (default)" : ""),(BANDWIDTH_DEFAULT==BANDWIDTH_8_MHZ ? " (default)" : ""));
    fprintf(stderr,"-tm N       DVB-T transmission mode - N=2%s or 8%s\n",(TRANSMISSION_MODE_DEFAULT==TRANSMISSION_MODE_2K ? " (default)" : ""),(TRANSMISSION_MODE_DEFAULT==TRANSMISSION_MODE_8K ? " (default)" : ""));

    fprintf(stderr,"\n-analyse    Perform a simple analysis of the bitrates of the PIDs in the transport stream\n");

    fprintf(stderr,"\n");
    fprintf(stderr,"NOTE: Use pid1=8192 to broadcast whole TS stream from a budget card\n");
    
    exit(-1);
}

int parse_args( int argc, char **argv)
{
	long start_time=-1;
	long end_time=-1;
	int i;
	char *ch;
	
    npids=0;
    
    fe_set = init_fe_settings();
    
    
    for (i=1;i<argc;i++) {
      if (strcmp(argv[i],"-ps")==0) {
        output_type=RTP_PS;
      } else if (strcmp(argv[i],"-analyse")==0) {
        do_analyse=1;
        output_type=RTP_NONE;
        if (secs==0) { secs=10; }
      } else if (strcmp(argv[i],"-f")==0) {
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
      } else if (strstr(argv[i], "-o:")==argv[i]) {
        if (strlen(argv[i]) > 3) {
          int pid_cnt = 0, pid, j;

          map_cnt++;
          pids_map = (pids_map_t*) realloc(pids_map, sizeof(pids_map_t) * map_cnt);
          pids_map[map_cnt-1].start_time=start_time;
          pids_map[map_cnt-1].end_time=end_time;
          for(j=0; j < MAX_CHANNELS; j++) pids_map[map_cnt-1].pids[j] = -1;
          pids_map[map_cnt-1].filename = (char *) malloc(strlen(argv[i]) - 2);
          strcpy(pids_map[map_cnt-1].filename, &argv[i][3]);
          i++;

          while(i < argc) {
            int found;
            if (sscanf(argv[i], "%d", &pid) == 0) {
              i--;
              break;
            }

            // block for the map
            found = 0;
            for (j=0;j<MAX_CHANNELS;j++) {
              if(pids_map[map_cnt-1].pids[j] == pid) found = 1;
            }
            if (found == 0) {
              pids_map[map_cnt-1].pids[pid_cnt] = pid;
              pid_cnt++;
            }

            // block for the list of pids to demux
            found = 0;
            for (j=0;j<npids;j++) {
              if(pids[j] == pid) found = 1;
            }
            if (found==0) {
              pids[npids++] = pid;
            }
            i++;
          }

          if (pid_cnt > 0) {
             FILE *f;
             f = fopen(pids_map[map_cnt-1].filename, "w+b");
             if (f != NULL) {
               pids_map[map_cnt-1].fd = fileno(f);
               fprintf(stderr, "Open file %s\n", pids_map[map_cnt-1].filename);
             } else {
               pids_map[map_cnt-1].fd = -1;
               fprintf(stderr, "Couldn't open file %s, errno:%d\n", pids_map[map_cnt-1].filename, errno);
             }
	   }
           output_type = MAP_TS;
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
	



int main(int argc, char **argv)
{
	int fd_dvr=-1;
	int i,j;
	unsigned char buf[BUFFER_SIZE];
	int bytes_read;
	unsigned char* free_bytes;
	int64_t counts[8192];
	double f;
	struct timeval tv;
	
	/* Output: {uni,multi,broad}cast socket */
	//char ipOut[20];
	
	pids_map = NULL;
	map_cnt = 0;
	
	fprintf(stderr,"dvbstream v0.5 - (C) Dave Chapman 2001-2004\n");
	
	/* Initialise PID map */
	for (i=0;i<8192;i++) {
		hi_mappids[i]=(i >> 8);
		lo_mappids[i]=(i&0xff);
		counts[i]=0;
	}
	
	if (argc<=1) usage();
	else parse_args( argc, argv );
	
	
	if (signal(SIGHUP, signal_handler) == SIG_IGN) signal(SIGHUP, SIG_IGN);
	if (signal(SIGINT, signal_handler) == SIG_IGN) signal(SIGINT, SIG_IGN);
	if (signal(SIGTERM, signal_handler) == SIG_IGN) signal(SIGTERM, SIG_IGN);
	if (signal(SIGALRM, signal_handler) == SIG_IGN) signal(SIGALRM, SIG_IGN);
	alarm(ALARM_TIME);
	

	// Open the Frontend
	if((fd_frontend = open(frontenddev[card],O_RDWR)) < 0){
		perror("Failed to open frontend device ");
		return -1;
	}

    // Tune in the frontend
	if ((fe_set->freq!=0) && (fe_set->polarity!=0)) {
		int err =tune_it(fd_frontend, fe_set);
		if (err<0) { exit(err); }
	} else {
		fprintf(stderr,"Not tuning in frontend.");
	}

	

  for (i=0;i<map_cnt;i++) {
    fprintf(stderr,"MAP %d, file %s: From %ld secs, To %ld secs, %d PIDs - ",i,pids_map[i].filename,pids_map[i].start_time,pids_map[i].end_time,pids_map[i].num);
    for (j=0;j<MAX_CHANNELS;j++) { if (pids_map[i].pids[j]!=-1) fprintf(stderr," %d",pids_map[i].pids[j]); }
    fprintf(stderr,"\n");
  }

  for (i=0;i<npids;i++) {  
    if((fd[i] = open(demuxdev[card],O_RDWR)) < 0){
      fprintf(stderr,"FD %i: ",i);
      perror("DEMUX DEVICE: ");
      return -1;
    }
  }

  if((fd_dvr = open(dvrdev[card],O_RDONLY)) < 0){
    perror("DVR DEVICE: ");
    return -1;
  }

  /* Now we set the filters */
  for (i=0;i<npids;i++) set_ts_filter(fd[i],pids[i]);

  gettimeofday(&tv,(struct timezone*) NULL);
  real_start_time=tv.tv_sec;
  now=0;

  if (do_analyse) {
    fprintf(stderr,"Analysing PIDS\n");
  } else {
    if (to_stdout) {
      fprintf(stderr,"Output to stdout\n");
    }
    else {
		// Fix me

    }
    fprintf(stderr,"Streaming %d stream%s\n",npids,(npids==1 ? "" : "s"));
  }

    
  
  
  
  

  /* Read packets */
  free_bytes = buf;



  /* Set up timer */
  while ( !Interrupted) {

    if (output_type==RTP_TS) {
      /* Attempt to read 188 bytes from /dev/ost/dvr */
      if ((bytes_read = read(fd_dvr,free_bytes,PACKET_SIZE)) > 0) {
        if (bytes_read!=PACKET_SIZE) {
          fprintf(stderr,"No bytes left to read - aborting\n");
          break;
        }

        pid=((free_bytes[1]&0x1f) << 8) | (free_bytes[2]);
        free_bytes[1]=(free_bytes[1]&0xe0)|hi_mappids[pid];
        free_bytes[2]=lo_mappids[pid];
        free_bytes+=bytes_read;

        /* // If there isn't enough room for 1 more packet, then send it.
        if ((free_bytes+PACKET_SIZE-buf)>MAX_RTP_SIZE) {
          hdr.timestamp = getmsec()*90;
          if (to_stdout) {
            write(1, buf, free_bytes-buf);
          } else {
            sendrtp2(socketOut,&sOut,&hdr,buf,free_bytes-buf);
          }
          free_bytes = buf;
        }
        count++;
        */
        
      }
    } else {
      if (do_analyse) {
        if (read(fd_dvr,buf,PACKET_SIZE) > 0) {
          pid=((buf[1]&0x1f) << 8) | (buf[2]);
          //fprintf(stderr,"pid=%d\n",pid);
          counts[pid]++;
        }
      }
    }
  }

	if (Interrupted) {
		fprintf(stderr,"Caught signal %d - closing cleanly.\n",Interrupted);
	}
	
	for (i=0;i<npids;i++) close(fd[i]);
	close(fd_dvr);
	close(fd_frontend);

	if (do_analyse) {
		for (i=0;i<8192;i++) {
			if (counts[i]) {
				f=(counts[i]*184.0*8.0)/(secs*1024.0*1024.0);
				if (f >= 1.0)	fprintf(stdout,"%d:  %.3f Mbit/s\n",i,f);
				else			fprintf(stdout,"%d:  %.0f kbit/s\n",i,f*1024);
			}
		}
	}
	

	return(0);
}

