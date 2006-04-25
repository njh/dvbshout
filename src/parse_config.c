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
		strncpy( dvbshout_server.host, value, STR_BUF_SIZE);
	} else if (strcmp( "port", name ) == 0) {
		dvbshout_server.port = atoi( value );
	} else if (strcmp( "user", name ) == 0) { 
		strncpy( dvbshout_server.user, value, STR_BUF_SIZE);
	} else if (strcmp( "password", name ) == 0) { 
		strncpy( dvbshout_server.password, value, STR_BUF_SIZE);
	} else if (strcmp( "protocol", name ) == 0) {
		if (strcmp( "http", value )==0 || strcmp( "icecast2", value )==0) {
			dvbshout_server.protocol = SHOUT_PROTOCOL_HTTP;
		} else if (strcmp( "xaudiocast", value )==0 || strcmp( "icecast1", value )==0) {
			dvbshout_server.protocol = SHOUT_PROTOCOL_XAUDIOCAST;
		} else if (strcmp( "icq", value )==0 || strcmp( "shoutcast", value )==0) {
			dvbshout_server.protocol = SHOUT_PROTOCOL_ICY;
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
		dvbshout_multicast.ttl = atoi( value );
	} else if (strcmp( "port", name ) == 0) {
		dvbshout_multicast.port = atoi( value );
	} else if (strcmp( "user", name ) == 0) { 
		dvbshout_multicast.port = atoi( value );
	} else if (strcmp( "mtu", name ) == 0) {
		dvbshout_multicast.mtu = atoi( value );
	} else if (strcmp( "loopback", name ) == 0) {
		dvbshout_multicast.loopback = atoi( value );
	} else {
		fprintf(stderr, "Error parsing configuation line %d: invalid statement in section 'multicast'.\n", line_num);
		exit(-1);
	}
	
}

static int parse_fec( const char* value, int line_num ) {
	if (!strcmp(value,"auto")) {
		return FEC_AUTO;
	} else if (!strcmp(value,"1_2")) {
		return FEC_1_2;
	} else if (!strcmp(value,"2_3")) {
		return FEC_2_3;
	} else if (!strcmp(value,"3_4")) {
		return FEC_3_4;
	} else if (!strcmp(value,"5_6")) {
		return FEC_5_6;
	} else if (!strcmp(value,"7_8")) {
		return FEC_7_8;
	} else {
		fprintf(stderr,"Error parsing configuation line %d: invalid FEC code\n", line_num);
		exit(-1);
	}
}


static void process_statement_tuning( char* name, char* value, int line_num )
{

	if (strcmp( "card", name ) == 0) {
		dvbshout_tuning->card = atoi( value );
	} else if (strcmp( "type", name ) == 0) { 
		dvbshout_tuning->type = tolower(value[strlen(value)-1]);
		if (dvbshout_tuning->type != 't' && dvbshout_tuning->type != 's' && dvbshout_tuning->type != 'c') {
			fprintf(stderr,"Error parsing configuation line %d: invalid DVB card type (%c)\n", line_num, dvbshout_tuning->type);
			exit(-1);
		}
		
		
	} else if (strcmp( "frequency", name ) == 0) { 
		dvbshout_tuning->frequency = atoi( value );
	} else if (strcmp( "polarity", name ) == 0) { 
		dvbshout_tuning->polarity = tolower(value[0]);
	} else if (strcmp( "symbol_rate", name ) == 0) { 
		dvbshout_tuning->symbol_rate = atoi( value );
	} else if (strcmp( "tone", name ) == 0) { 
		dvbshout_tuning->tone = atoi( value );
		

	} else if (strcmp( "inversion", name ) == 0) { 
		if (!strcmp(value,"off")) {
			dvbshout_tuning->inversion=INVERSION_OFF;
		} else if (!strcmp(value,"on")) {
			dvbshout_tuning->inversion=INVERSION_ON;
		} else if (!strcmp(value,"auto")) {
			dvbshout_tuning->inversion=INVERSION_AUTO;
		} else {
			fprintf(stderr,"Error parsing configuation line %d: invalid inversion mode\n", line_num);
			exit(-1);
		}

	} else if (strcmp( "bandwidth", name ) == 0) { 
		switch(atoi(value)) {
			case 8:   dvbshout_tuning->bandwidth=BANDWIDTH_8_MHZ; break;
			case 7:   dvbshout_tuning->bandwidth=BANDWIDTH_7_MHZ; break;
			case 6:   dvbshout_tuning->bandwidth=BANDWIDTH_6_MHZ; break;
			default:
				fprintf(stderr,"Error parsing configuation line %d: invalid DVB-T bandwidth\n", line_num);
				exit(-1);
			break;
		}

	} else if (strcmp( "code_rate_hp", name ) == 0) { 
		dvbshout_tuning->code_rate_hp = parse_fec( value, line_num );

	} else if (strcmp( "code_rate_lp", name ) == 0) { 
		dvbshout_tuning->code_rate_lp = parse_fec( value, line_num );

	} else if (strcmp( "fec_inner", name ) == 0) { 
		dvbshout_tuning->fec_inner = parse_fec( value, line_num );
		
	} else if (strcmp( "modulation", name ) == 0) { 
		switch( atoi( value ) ) {
			case 16:  dvbshout_tuning->modulation=QAM_16; break;
			case 32:  dvbshout_tuning->modulation=QAM_32; break;
			case 64:  dvbshout_tuning->modulation=QAM_64; break;
			case 128: dvbshout_tuning->modulation=QAM_128; break;
			case 256: dvbshout_tuning->modulation=QAM_256; break;
			default:
				fprintf(stderr,"Error parsing configuation line %d: invalid modulation QAM rate\n", line_num);
				exit(-1);
			break;
        }

	} else if (strcmp( "hierarchy", name ) == 0) { 
		if (!strcmp(value,"none")) {
			dvbshout_tuning->hierarchy=HIERARCHY_NONE;
		} else if (!strcmp(value,"1")) {
			dvbshout_tuning->hierarchy=HIERARCHY_1;
		} else if (!strcmp(value,"2")) {
			dvbshout_tuning->hierarchy=HIERARCHY_2;
		} else if (!strcmp(value,"3")) {
			dvbshout_tuning->hierarchy=HIERARCHY_4;
		} else if (!strcmp(value,"auto")) {
			dvbshout_tuning->hierarchy=HIERARCHY_AUTO;
		} else {
			fprintf(stderr,"Error parsing configuation line %d: invalid hierarchy mode\n", line_num);
			exit(-1);
		}

	} else if (strcmp( "transmission_mode", name ) == 0) { 
		switch(atoi(value)) {
			case 8:   dvbshout_tuning->transmission_mode=TRANSMISSION_MODE_8K; break;
			case 2:   dvbshout_tuning->transmission_mode=TRANSMISSION_MODE_2K; break;
			default:
				fprintf(stderr,"Error parsing configuation line %d: invalid transmission mode\n", line_num);
				exit(-1);
			break;
		}

	} else if (strcmp( "guard_interval", name ) == 0) { 
		if (!strcmp(value,"1_32")) {
			dvbshout_tuning->guard_interval=GUARD_INTERVAL_1_32;
		} else if (!strcmp(value,"1_16")) {
			dvbshout_tuning->guard_interval=GUARD_INTERVAL_1_16;
		} else if (!strcmp(value,"1_8")) {
			dvbshout_tuning->guard_interval=GUARD_INTERVAL_1_8;
		} else if (!strcmp(value,"1_4")) {
			dvbshout_tuning->guard_interval=GUARD_INTERVAL_1_4;
		} else {
			fprintf(stderr,"Error parsing configuation line %d: invalid guard interval\n", line_num);
			exit(-1);
        }
        
	} else {
		fprintf(stderr, "Error parsing configuation line %d: invalid statement in section 'tuning'.\n", line_num);
		exit(-1);
	}

}


static void process_statement_channel( char* name, char* value, int line_num )
{
	dvbshout_channel_t *chan =  channels[ channel_count-1 ];
	
	if (strcmp( "name", name ) == 0) {
		strncpy( chan->name, value, STR_BUF_SIZE);
		
	} else if (strcmp( "mount", name ) == 0) { 
		strncpy( chan->mount, value, STR_BUF_SIZE);
		
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
		strncpy( chan->genre, value, STR_BUF_SIZE);
	
	} else if (strcmp( "public", name ) == 0) { 
		chan->is_public = atoi( value );
	
	} else if (strcmp( "description", name ) == 0) { 
		strncpy( chan->description, value, STR_BUF_SIZE);

	} else if (strcmp( "url", name ) == 0) { 
		strncpy( chan->url, value, STR_BUF_SIZE);

	} else if (strcmp( "multicast_ip", name ) == 0) { 
		strncpy( chan->multicast_ip, value, STR_BUF_SIZE);
		
	} else if (strcmp( "multicast_port", name ) == 0) { 
		chan->multicast_port = atoi(value);
		
	} else if (strcmp( "multicast_ttl", name ) == 0) { 
		chan->multicast_ttl = atoi(value);
		
	} else if (strcmp( "multicast_mtu", name ) == 0) { 
		chan->multicast_mtu = atoi(value);

	} else if (strcmp( "multicast_loopback", name ) == 0) { 
		chan->multicast_loopback = atoi(value);
		
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
				dvbshout_channel_t* chan=NULL;
				
				if (channel_count >= MAX_CHANNEL_COUNT) {
					fprintf(stderr, "Error: reached maximum number of channels allowed at line %d\n", line_num);
					return 1;
				}
				
				// Allocate memory for channel structure
				chan = calloc( 1, sizeof(dvbshout_channel_t) );
				if (!chan) {
					perror("Failed to allocate memory for new channel");
					exit(-1);
				}

				chan->num = channel_count;
				chan->shout = NULL;
				chan->pes_stream_id = 0;
				chan->fd = -1;
				chan->continuity_count = -1;
				
				chan->multicast_port = dvbshout_multicast.port;
				chan->multicast_ttl = dvbshout_multicast.ttl;
				chan->multicast_mtu = dvbshout_multicast.mtu;
				chan->multicast_loopback = dvbshout_multicast.loopback;
				
				
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

