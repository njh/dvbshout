/* 

	parse_config.c
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

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include "dvbshout.h"


static void process_statement_server( char* name, char* value, int line_num )
{

	if (strcmp( "host", name ) == 0) {
		strncpy( shout_server.host, value, STR_BUF_SIZE);
	} else if (strcmp( "port", name ) == 0) {
		shout_server.port = atoi( value );
	} else if (strcmp( "user", name ) == 0) { 
		strncpy( shout_server.user, value, STR_BUF_SIZE);
	} else if (strcmp( "password", name ) == 0) { 
		strncpy( shout_server.password, value, STR_BUF_SIZE);
	} else if (strcmp( "protocol", name ) == 0) {
		if (strcmp( "http", value )==0 || strcmp( "icecast2", value )==0) {
			shout_server.protocol = SHOUT_PROTOCOL_HTTP;
		} else if (strcmp( "xaudiocast", value )==0 || strcmp( "icecast1", value )==0) {
			shout_server.protocol = SHOUT_PROTOCOL_XAUDIOCAST;
		} else if (strcmp( "icq", value )==0 || strcmp( "shoutcast", value )==0) {
			shout_server.protocol = SHOUT_PROTOCOL_ICY;
		} else {
			fprintf(stderr, "Error parsing configuation line %d: invalid protocol.\n", line_num);
			exit(-1);
		}
	} else {
		fprintf(stderr, "Error parsing configuation line %d: invalid statement in section 'server'.\n", line_num);
		exit(-1);
	}
	
}

static void process_statement_multicast( char* name, char* value, int line_num )
{

	if (strcmp( "ttl", name ) == 0) {
		shout_multicast.ttl = atoi( value );
	} else if (strcmp( "port", name ) == 0) {
		shout_multicast.port = atoi( value );
	} else if (strcmp( "user", name ) == 0) { 
		shout_multicast.port = atoi( value );
	} else if (strcmp( "interface", name ) == 0) { 
		strncpy( shout_multicast.interface, value, STR_BUF_SIZE);
	} else if (strcmp( "mtu", name ) == 0) {
		shout_multicast.mtu = atoi( value );
	} else {
		fprintf(stderr, "Error parsing configuation line %d: invalid statement in section 'multicast'.\n", line_num);
		exit(-1);
	}
	
}


static void process_statement_tuning( char* name, char* value, int line_num )
{

	if (strcmp( "card", name ) == 0) {
		fe_set->card = atoi( value );
	} else if (strcmp( "type", name ) == 0) { 
		fe_set->type = tolower(value[strlen(value)-1]);
		if (fe_set->type != 't' && fe_set->type != 's' && fe_set->type != 'c') {
			fprintf(stderr,"Error parsing configuation line %d: invalid DVB card type (%c)\n", line_num, fe_set->type);
			exit(-1);
		}
	} else if (strcmp( "frequency", name ) == 0) { 
		fe_set->freq = atoi( value );
	} else if (strcmp( "polarity", name ) == 0) { 
		fe_set->polarity = tolower(value[0]);
	} else if (strcmp( "symbol_rate", name ) == 0) { 
		fe_set->srate = atoi( value );
	} else if (strcmp( "modulation", name ) == 0) { 
		switch( atoi( value ) ) {
			case 16:  fe_set->modulation=QAM_16; break;
			case 32:  fe_set->modulation=QAM_32; break;
			case 64:  fe_set->modulation=QAM_64; break;
			case 128: fe_set->modulation=QAM_128; break;
			case 256: fe_set->modulation=QAM_256; break;
			default:
				fprintf(stderr,"Error parsing configuation line %d: invalid QAM rate\n", line_num);
				exit(-1);
			break;
        }
        
	} else if (strcmp( "guard_interval", name ) == 0) { 
		switch( atoi( value ) ) {
			case 32:  fe_set->guard_interval=GUARD_INTERVAL_1_32; break;
			case 16:  fe_set->guard_interval=GUARD_INTERVAL_1_16; break;
			case 8:   fe_set->guard_interval=GUARD_INTERVAL_1_8; break;
			case 4:   fe_set->guard_interval=GUARD_INTERVAL_1_4; break;
			default:
				fprintf(stderr,"Error parsing configuation line %d: invalid Guard Interval\n", line_num);
				exit(-1);
			break;
        }
        
	} else if (strcmp( "code_rate", name ) == 0) { 
        if (!strcmp(value,"auto")) {
          fe_set->code_rate=FEC_AUTO;
        } else if (!strcmp(value,"1_2")) {
          fe_set->code_rate=FEC_1_2;
        } else if (!strcmp(value,"2_3")) {
          fe_set->code_rate=FEC_2_3;
        } else if (!strcmp(value,"3_4")) {
          fe_set->code_rate=FEC_3_4;
        } else if (!strcmp(value,"5_6")) {
          fe_set->code_rate=FEC_5_6;
        } else if (!strcmp(value,"7_8")) {
          fe_set->code_rate=FEC_7_8;
        } else {
			fprintf(stderr,"Error parsing configuation line %d: invalid code rate\n", line_num);
			exit(-1);
        }

	} else if (strcmp( "bandwidth", name ) == 0) { 
		switch(atoi(value)) {
			case 8:   fe_set->bandwidth=BANDWIDTH_8_MHZ; break;
			case 7:   fe_set->bandwidth=BANDWIDTH_7_MHZ; break;
			case 6:   fe_set->bandwidth=BANDWIDTH_6_MHZ; break;
			default:
				fprintf(stderr,"Error parsing configuation line %d: invalid DVB-T bandwidth\n", line_num);
				exit(-1);
			break;
		}

	} else if (strcmp( "transmission_mode", name ) == 0) { 
		switch(atoi(value)) {
			case 8:   fe_set->transmission_mode=TRANSMISSION_MODE_8K; break;
			case 2:   fe_set->transmission_mode=TRANSMISSION_MODE_2K; break;
			default:
				fprintf(stderr,"Error parsing configuation line %d: invalid transmission mode\n", line_num);
				exit(-1);
			break;
		}
        
	} else {
		fprintf(stderr, "Error parsing configuation line %d: invalid statement in section 'tuning'.\n", line_num);
		exit(-1);
	}

}


static void process_statement_channel( char* name, char* value, int line_num )
{
	shout_channel_t *chan =  channels[ channel_count-1 ];
	shout_t *shout = chan->shout;
	
	if (strcmp( "name", name ) == 0) {
		if (shout_set_name( shout, value ) != SHOUTERR_SUCCESS) {
			fprintf(stderr,"Error on configuation line %d: %s\n", line_num, shout_get_error( shout ));
			exit(-1);
		}
		
		strncpy( chan->name, value, STR_BUF_SIZE);
		
	} else if (strcmp( "mount", name ) == 0) { 
		if (shout_set_mount( shout, value ) != SHOUTERR_SUCCESS) {
			fprintf(stderr,"Error on configuation line %d: %s\n", line_num, shout_get_error( shout ));
			exit(-1);
		}
		
	
	} else if (strcmp( "pid", name ) == 0) { 
	
		// Check PID is valid
		chan->pid = atoi( value );
		if (chan->pid == 0) {
			fprintf(stderr,"Error parsing configuation line %d: invalid PID\n", line_num);
			exit(-1);
		}
		
		// Add channel to the channel map
		if( channel_map[ chan->pid ] ) {
			fprintf(stderr,"Error parsing configuation line %d: duplicate PID\n", line_num);
			exit(-1);
		} else {
			channel_map[ chan->pid ] = chan;
		}
		
	} else if (strcmp( "genre", name ) == 0) { 
		if (shout_set_genre( shout, value ) != SHOUTERR_SUCCESS) {
			fprintf(stderr,"Error on configuation line %d: %s\n", line_num, shout_get_error( shout ));
			exit(-1);
		}
	
	} else if (strcmp( "public", name ) == 0) { 
		if (shout_set_public( shout, atoi(value) ) != SHOUTERR_SUCCESS) {
			fprintf(stderr,"Error on configuation line %d: %s\n", line_num, shout_get_error( shout ));
			exit(-1);
		}
	
	} else if (strcmp( "url", name ) == 0) { 
		if (shout_set_url( shout, value ) != SHOUTERR_SUCCESS) {
			fprintf(stderr,"Error on configuation line %d: %s\n", line_num, shout_get_error( shout ));
			exit(-1);
		}
	
	} else if (strcmp( "description", name ) == 0) { 
		if (shout_set_description( shout, value ) != SHOUTERR_SUCCESS) {
			fprintf(stderr,"Error on configuation line %d: %s\n", line_num, shout_get_error( shout ));
			exit(-1);
		}

	} else if (strcmp( "multicast_ip", name ) == 0) { 
		strncpy( chan->multicast_ip, value, STR_BUF_SIZE);
		
	} else if (strcmp( "multicast_port", name ) == 0) { 
		chan->multicast_port = atoi(value);
		
	} else if (strcmp( "multicast_ttl", name ) == 0) { 
		chan->multicast_ttl = atoi(value);
		
	} else if (strcmp( "multicast_mtu", name ) == 0) { 
		chan->multicast_mtu = atoi(value);
		
	} else {
		fprintf(stderr, "Error parsing configuation line %d: invalid statement in section 'channel'.\n", line_num);
		exit(-1);
	}
	
		
}

int parse_config( char *filepath )
{

	FILE* file = NULL;
	char line[STR_BUF_SIZE];
	char section[STR_BUF_SIZE];
	char* ptr;
	int i, line_num=0;
	
	// Initialize strings
	line[0] = '\0';
	section[0] = '\0';
	
	
	// Open the input file
	file = fopen( filepath, "r" );
	if (file==NULL) {
		perror("Failed to open config file");
		exit(-1);
	}
	
	
	// Parse it line by line
	while( !feof( file ) ) {
		line_num++;
		
		fgets( line, STR_BUF_SIZE, file );
		
		// Ignore lines starting with a #
		if (line[0] == '#') continue;
		
		// Remove newline from end
		if (line[strlen(line)-1] == '\n')
			line[strlen(line)-1] = '\0';

		// Ignore empty lines
		if (strlen(line) == 0) continue;
		
		// Is it the start of a section?
		if (line[0]=='[') {
			ptr = &line[1];
			for(i=0; i<strlen(ptr); i++) {
				if (ptr[i] == ']') ptr[i] = '\0';
			}
			
			if (strcmp( ptr, "server")==0) {
				strcpy( section, ptr );
				
			} else if (strcmp( ptr, "multicast")==0) {
				strcpy( section, ptr );
				
			} else if (strcmp( ptr, "tuning")==0) {
				strcpy( section, ptr );
			
			} else if (strcmp( ptr, "channel")==0) {
				shout_channel_t* chan = calloc( 1, sizeof(shout_channel_t) );
				if (!chan) {
					perror("Failed to allocate memory for new channel");
					exit(-1);
				}

				chan->num = channel_count;
				chan->shout = shout_new();
				chan->stream_id = 0;
				chan->fd = -1;
				chan->continuity_count = -1;
				
				chan->multicast_port = shout_multicast.port;
				chan->multicast_ttl = shout_multicast.ttl;
				chan->multicast_mtu = shout_multicast.mtu;
				
				
				strcpy( section, ptr );
				channels[ channel_count ] = chan;
				channel_count++;
				
			} else {
				fprintf(stderr, "Error parsing configuation line %d: unknown section '%s'\n", line_num, ptr);
				exit(-1);
			}
			
			
			
		} else {
			char* name = line;
			char* value = NULL;
			
			// Split up the name and value
			for(i=0; i<strlen(name); i++) {
				if (name[i] == ':') {
					name[i] = '\0';
					value = &name[i+1];
					if (value[0] == ' ') { value++; }
					if (strlen(value)==0) { value=NULL; }
				}
			}
			
			//fprintf(stderr, "%s: %s=%s.\n", section, name, value);
			
			// Ignore empty values
			if (value==NULL) continue;
			
			if (strcmp( section, "server")==0) {	
				process_statement_server( name, value, line_num );
			} else if (strcmp( section, "multicast")==0) {	
				process_statement_multicast( name, value, line_num );
			} else if (strcmp( section, "tuning")==0) {	
				process_statement_tuning( name, value, line_num );
			} else if (strcmp( section, "channel")==0) {	
				process_statement_channel( name, value, line_num );
			} else {
				fprintf(stderr, "Error parsing configuation line %d: missing section.\n", line_num);
				exit(-1);
			}

		}
		
		
	}
	

	fclose( file );
	
	return 0;
}

