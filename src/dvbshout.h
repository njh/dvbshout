/* dvb_defaults.h

   Copyright (C) Nicholas Humfrey 2006, Dave Chapman 2002

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
   Or, point your browser to http://www.gnu.org/copyleft/gpl.html

*/

#ifndef _DVB_SHOUT_H
#define _DVB_SHOUT_H


// DVB includes:
#include <linux/dvb/dmx.h>
#include <linux/dvb/frontend.h>

// Libshout include
#include <shout/shout.h>

#include "mpa_header.h"



/* DVB-S */

// With a diseqc system you may need different values per LNB.  I hope
// no-one ever asks for that :-)

#define SLOF (11700*1000UL)
#define LOF1 (9750*1000UL)
#define LOF2 (10600*1000UL)

/* DVB-T */

/* Defaults are for the United Kingdom */
#define BANDWIDTH_DEFAULT           BANDWIDTH_8_MHZ
#define CODERATE_DEFAULT        	FEC_AUTO
#define CONSTELLATION_DEFAULT       QAM_64
#define TRANSMISSION_MODE_DEFAULT   TRANSMISSION_MODE_2K
#define GUARD_INTERVAL_DEFAULT      GUARD_INTERVAL_1_32
#define HIERARCHY_DEFAULT           HIERARCHY_NONE



#if HIERARCHY_DEFAULT == HIERARCHY_NONE && !defined (LP_CODERATE_DEFAULT)
#define LP_CODERATE_DEFAULT (0) /* unused if HIERARCHY_NONE */
#endif

// The size of MPEG2 TS packets
#define TS_PACKET_SIZE			188

// There seems to be a limit of 16 simultaneous filters in the driver
#define MAX_CHANNEL_COUNT		16

// Maximum allowed PID value
#define MAX_PID_COUNT			8192

// Maximum allowed PID value
#define STR_BUF_SIZE			1025



/*
	Macros for accessing MPEG-2 TS packet headers
*/
#define TS_PACKET_SYNC_BYTE(b)		(b[0])
#define TS_PACKET_TRANS_ERROR(b)	((b[1]&0x80)>>7)
#define TS_PACKET_PAYLOAD_START(b)	((b[1]&0x40)>>6)
#define TS_PACKET_PRIORITY(b)		((b[1]&0x20)>>4)
#define TS_PACKET_PID(b)			(((b[1]&0x1F)<<8) | b[2])

#define TS_PACKET_SCRAMBLING(b)		((b[3]&0xC0)>>6)
#define TS_PACKET_ADAPTATION(b)		((b[3]&0x30)>>4)
#define TS_PACKET_CONT_COUNT(b)		((b[3]&0x0F)>>0)



/*
	Macros for accessing MPEG-2 PES packet headers
*/
#define PES_PACKET_SYNC_BYTE1(b)	(b[0])
#define PES_PACKET_SYNC_BYTE2(b)	(b[1])
#define PES_PACKET_SYNC_BYTE3(b)	(b[2])
#define PES_PACKET_STREAM_ID(b)		(b[3])
#define PES_PACKET_LEN(b)			((b[4]) << 8) | (b[5])

#define PES_PACKET_SYNC_CODE(b)		((b[6] & 0xC0) >> 6)
#define PES_PACKET_SCRAMBLED(b)		((b[6] & 0x30) >> 4)
#define PES_PACKET_PRIORITY(b)		((b[6] & 0x08) >> 3)
#define PES_PACKET_ALIGNMENT(b)		((b[6] & 0x04) >> 2)
#define PES_PACKET_COPYRIGHT(b)		((b[6] & 0x02) >> 1)
#define PES_PACKET_ORIGINAL(b)		((b[6] & 0x01) >> 0)

#define PES_PACKET_PTS_DTS(b)		((b[7] & 0xC0) >> 6)
#define PES_PACKET_ESCR(b)			((b[7] & 0x20) >> 5)
#define PES_PACKET_ESR(b)			((b[7] & 0x10) >> 4)
#define PES_PACKET_DSM_TRICK(b)		((b[7] & 0x8) >> 3)
#define PES_PACKET_ADD_COPY(b)		((b[7] & 0x4) >> 2)
#define PES_PACKET_CRC(b)			((b[7] & 0x2) >> 1)
#define PES_PACKET_EXTEN(b)			((b[7] & 0x1) >> 0)
#define PES_PACKET_HEAD_LEN(b)		(b[8])



/* Structure containing single channel */
typedef struct shout_channel {

	int num;				// channel number
	char name[STR_BUF_SIZE];// channel name
	int fd;					// debux file descriptor
	int pid;				// Packet Identifier of audio stream
	int stream_id;			// PES stream ID
	
	shout_t *shout;			// libshout structure

	mpa_header_t mpah;		// Parsed MPEG audio header
	int synced;				// Have MPA sync?
	
	unsigned char* buf;		// MPEG Audio Buffer (ready for sending)
	int buf_size;			// Total size of MPEG Audio Buffer
	int buf_used;			// Amount of buffer used
	
	size_t pes_remaining;	// Number of bytes remaining in current PES packet

} shout_channel_t;


/* Structure server settings */
typedef struct shout_server {
	char host[STR_BUF_SIZE];
	int port;
	char user[STR_BUF_SIZE];
	char password[STR_BUF_SIZE];
	int protocol;

} shout_server_t;


/* Structure containing tuning settings */
typedef struct fe_settings {

	unsigned char card;			// Card number
	
	unsigned int freq;			// Frequency (Hz)
	unsigned char polarity;		// Polarity
	unsigned int srate;			// Symbols per Second (Hz)
	int tone;					// 22kHz tone (-1 = auto)
	
	unsigned int diseqc;
	fe_spectral_inversion_t spec_inv;
	fe_modulation_t modulation;
	fe_code_rate_t code_rate;
	fe_transmit_mode_t transmission_mode;
	fe_guard_interval_t guard_interval;
	fe_bandwidth_t bandwidth;

} fe_settings_t;




/* In dvbshout.c */
extern int channel_count;
extern fe_settings_t *fe_set;
extern shout_channel_t *channel_map[MAX_PID_COUNT];
extern shout_channel_t *channels[MAX_CHANNEL_COUNT];
extern shout_server_t shout_server;


/* In tune.c */
int tune_it(int fd_frontend, fe_settings_t *set);

/* In pes.c */
unsigned char* parse_pes( unsigned char* buf, int size, size_t *payload_size, shout_channel_t *chan);


/* In parse_config.c */
int parse_config( char *filepath );



#endif
