/* 

	ts2es.c
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



void process_ts_packets( FILE* file_in, shout_channel_t *chan )
{
	unsigned char buf[TS_PACKET_SIZE];
	unsigned char* pes_ptr=NULL;
	size_t pes_len;
	int bytes_read;

	while ( 1 ) {
		unsigned int pid=0, offset=0;
		
		bytes_read = fread(buf,1,TS_PACKET_SIZE, file_in);
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

		// Transport error?
		if ( buf[1]&0x90) {
			fprintf(stderr, "Transport error in PID %d.\n", pid);
		}			



		// Location of and size of PES payload
		pes_ptr = &buf[4];
		pes_len = TS_PACKET_SIZE - 4;

		// Check for adaptation field?
		if ( buf[3] & 0x20) {
			pes_ptr += buf[4] + 1;
			pes_len -= buf[4] + 1;
			fprintf(stderr,"offset=%d\n", offset);
		}

		// Check we know about the payload
		if (pid == chan->apid) {
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
					
					if (es_len>chan->pes_remaining) {
						es_len-=3;
						fprintf(stderr, "es_len=%d pes_remaining=%d\n", es_len-3, chan->pes_remaining);
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
	shout_channel_t channel;
	FILE* in;
	
	
	// Initialise data structures
	memset( &channel, 0, sizeof(shout_channel_t) );
	channel.apid = 2313;


	in = fopen( "dump.ts", "rb" );
	if (!in) perror("Failed to open input file");
	
	process_ts_packets( in, &channel );
	
	
	fclose( in );


	return(0);
}



