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




unsigned char* parse_pes( unsigned char* buf, int size, size_t *payload_size, dvbshout_channel_t *chan) 
{
	size_t pes_len = PES_PACKET_LEN(buf);
	size_t pes_header_len = PES_PACKET_HEAD_LEN(buf);
	unsigned char stream_id = PES_PACKET_STREAM_ID(buf);

	if( PES_PACKET_SYNC_BYTE1(buf) != 0x00 ||
	    PES_PACKET_SYNC_BYTE2(buf) != 0x00 ||
	    PES_PACKET_SYNC_BYTE3(buf) != 0x01 )
	{
		fprintf(stderr, "Invalid PES header (pid: %d).\n", chan->pid);
		return 0;
	}
	
	// Stream IDs in range 0xC0-0xDF are MPEG audio
	if( stream_id != chan->pes_stream_id )
	{
		if (stream_id < 0xC0 || stream_id > 0xDF) {
			fprintf(stderr, "Ignoring non-mpegaudio stream ID 0x%x (pid: %d).\n", stream_id, chan->pid);
			return 0;
		}
	
		if (chan->pes_stream_id == 0) {
			// keep the first stream we see
			chan->pes_stream_id = stream_id;	
		} else {
			fprintf(stderr, "Ignoring additional audio stream ID 0x%x (pid: %d).\n", stream_id, chan->pid);
			return 0;
		}
	
	}
	
	
	// Check PES Extension header 
	if( PES_PACKET_SYNC_CODE(buf) != 0x2 )
	{
		fprintf(stderr, "Error: invalid sync code PES extension header (pid: %d).\n", chan->pid);
		return 0;
	}

	// Reject scrambled packets
	if( PES_PACKET_SCRAMBLED(buf) )
	{
		fprintf(stderr, "Error: PES payload is scrambled (pid: %d).\n", chan->pid);
		return 0;
	}

	// Store the presentation timestamp of this PES packet
	if (PES_PACKET_PTS_DTS(buf) & 0x2) {
		chan->pes_ts=PES_PACKET_PTS(buf);
	}
	
	// Store Display Time Stamp of this PES packet
	//if (PES_PACKET_PTS_DTS(buf) & 0x3) {
	//	chan->pes_dts=PES_PACKET_DTS(buf);
	//}
	
	// Check for flag to see if MPEG audio starts at begining for this PES packet
	if (!chan->synced && PES_PACKET_ALIGNMENT(buf)==0) {
		fprintf(stderr, "Warning: PES_PACKET_ALIGNMENT=0 while trying to sync to audio.\n" );
		return 0;
	}
	

/*
	fprintf(stderr, "PES Packet:   pid: %d,  length: %d\n", chan->pid, pes_len);

	fprintf(stderr, "  PES_PACKET_STREAM_ID=%d\n", PES_PACKET_STREAM_ID(buf) );
	fprintf(stderr, "  PES_PACKET_PRIORITY=%d\n", PES_PACKET_PRIORITY(buf) );
	fprintf(stderr, "  PES_PACKET_ALIGNMENT=%d\n", PES_PACKET_ALIGNMENT(buf) );
	fprintf(stderr, "  PES_PACKET_COPYRIGHT=%d\n", PES_PACKET_COPYRIGHT(buf) );
	fprintf(stderr, "  PES_PACKET_ORIGINAL=%d\n", PES_PACKET_ORIGINAL(buf) );
	fprintf(stderr, "  PES_PACKET_PTS_DTS=0x%x\n", PES_PACKET_PTS_DTS(buf) );
	fprintf(stderr, "  PES_PACKET_ESCR=%d\n", PES_PACKET_ESCR(buf) );
	fprintf(stderr, "  PES_PACKET_ESR=%d\n", PES_PACKET_ESR(buf) );
	fprintf(stderr, "  PES_PACKET_DSM_TRICK=%d\n", PES_PACKET_DSM_TRICK(buf) );
	fprintf(stderr, "  PES_PACKET_ADD_COPY=%d\n", PES_PACKET_ADD_COPY(buf) );
	fprintf(stderr, "  PES_PACKET_CRC=%d\n", PES_PACKET_CRC(buf) );
	fprintf(stderr, "  PES_PACKET_EXTEN=%d\n", PES_PACKET_EXTEN(buf) );
	
	fprintf(stderr, "  PES_PACKET_PTS=%d\n", PES_PACKET_PTS(buf) );
	fprintf(stderr, "  PES_PACKET_DTS=%d\n", PES_PACKET_DTS(buf) );
*/

	// Store the length of the PES packet payload
	chan->pes_remaining = pes_len - (2+pes_header_len);

	// Return pointer and length of payload in this TS packet
	*payload_size = size-(9+pes_header_len);
	return buf+(9+pes_header_len);
}


