/* 

	parse_config.c
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "dvbshout.h"


#define READ_BUFFER		(1024)



int parse_config( char *filepath )
{

	FILE* file = NULL;
	char line[READ_BUFFER];
	char* section=NULL;
	
	
	// Open the input file
	file = fopen( filepath, "r" );
	if (file==NULL) {
		perror("Failed to open config file");
		exit(-1);
	}
	
	
	// Parse it line by line
	while( !feof( file ) ) {
		fgets( line, READ_BUFFER, file );
		
		// Ignore lines starting with a #
		if (line[0] == '#') continue;
		
		// Remove newline from end
		if (line[strlen(line)-1] == '\n')
			line[strlen(line)-1] = '\0';

		// Ignore empty lines
		if (strlen(line) == 0) continue;
		
		printf("line: %s\n", line);
	}
	

	fclose( file );

	return 0;
}

