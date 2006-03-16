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


void process_statement_server( char* name, char* value, int line_num )
{

	if (strcmp( "host", name ) == 0) {
		strncpy( shout_server.host, value, STR_BUF_SIZE);
	} else if (strcmp( "port", name ) == 0) { 
		strncpy( shout_server.port, value, STR_BUF_SIZE);
	} else if (strcmp( "user", name ) == 0) { 
		strncpy( shout_server.user, value, STR_BUF_SIZE);
	} else if (strcmp( "pass", name ) == 0) { 
		strncpy( shout_server.pass, value, STR_BUF_SIZE);
	} else if (strcmp( "protocol", name ) == 0) { 
		strncpy( shout_server.protocol, value, STR_BUF_SIZE);
	} else {
		fprintf(stderr, "Error parsing configuation line %d: invalid statement.\n", line_num);
		exit(-1);
	}
	
}

void process_statement_tuning( char* name, char* value, int line_num )
{

	if (strcmp( "card", name ) == 0) {
		fe_set->card = atoi( value );
	} else if (strcmp( "frequency", name ) == 0) { 
		fe_set->freq = atoi( value )*1000UL;
	} else if (strcmp( "polarity", name ) == 0) { 
		fe_set->polarity = tolower(value[0]);
	} else if (strcmp( "symbol_rate", name ) == 0) { 
		fe_set->srate = atoi( value )*1000UL;
	} else {
		fprintf(stderr, "Error parsing configuation line %d: invalid statement.\n", line_num);
		exit(-1);
	}

}

void process_statement_channel( char* name, char* value, int line_num )
{

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

