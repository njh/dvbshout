/* 

	pes.c
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
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include "dvbshout.h"
#include "config.h"




unsigned char* parse_pes( unsigned char* buf, int size, size_t *payload_size, shout_channel_t *chan) 
{
	size_t pes_len = ((buf[4]) << 8) | (buf[5]);
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

	fprintf(stderr, "pid: %d, length: %d header_len: %d\n", chan->apid, pes_len, pes_header_len);
/*	
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

	// Return pointer and length of payload
	*payload_size = size-(6+pes_header_len);
	
	return buf+(6+pes_header_len);
}




