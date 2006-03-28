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

// oRTP include
#include <ortp/ortp.h>

#include "mpa_header.h"



/* DVB-S */

// With a diseqc system you may need different values per LNB.  I hope
// no-one ever asks for that :-)

#define SLOF (11700*1000UL)
#define LOF1 (9750*1000UL)
#define LOF2 (10600*1000UL)

#define SYMBOLRATE_DEFAULT 			(27500)
#define DISEQC_DEFAULT 				(0)
#define SEC_TONE_DEFAULT 			(-1)


/* DVB-T */

/* Defaults are for the United Kingdom */
#define INVERSION_DEFAULT			INVERSION_AUTO
#define BANDWIDTH_DEFAULT           BANDWIDTH_8_MHZ
#define CODERATE_HP_DEFAULT        	FEC_3_4
#define CODERATE_LP_DEFAULT        	FEC_3_4
#define FEC_INNER_DEFAULT	        FEC_AUTO
#define MODULATON_DEFAULT     		QAM_16
#define HIERARCHY_DEFAULT           HIERARCHY_NONE
#define TRANSMISSION_MODE_DEFAULT   TRANSMISSION_MODE_2K
#define GUARD_INTERVAL_DEFAULT      GUARD_INTERVAL_1_32


/* Defauls for shout server */
#define SERVER_PORT_DEFAULT				(8000)
#define SERVER_USER_DEFAULT				"source"
#define SERVER_PASSWORD_DEFAULT			"hackme"
#define SERVER_PROTOCOL_DEFAULT			SHOUT_PROTOCOL_HTTP

/* Defaults for Multicast */
#define MULTICAST_TTL_DEFAULT			(5)
#define MULTICAST_PORT_DEFAULT			(5004)
#define MULTICAST_INTERFACE_DEFAULT		"eth0"
#define MULTICAST_MTU_DEFAULT			(1450)
 


// The size of MPEG2 TS packets
#define TS_PACKET_SIZE			188

// There seems to be a limit of 32 simultaneous filters in the driver
#define MAX_CHANNEL_COUNT		32

// Maximum allowed PID value
#define MAX_PID_COUNT			8192

// Maximum allowed PID value
#define STR_BUF_SIZE			1025

// Maximum allowed PID value
#define RTP_MPEG_AUDIO_PT		14



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
#define TS_PACKET_ADAPT_LEN(b)		(b[4])



/*
	Macros for accessing MPEG-2 PES packet headers
*/
#define PES_PACKET_SYNC_BYTE1(b)	(b[0])
#define PES_PACKET_SYNC_BYTE2(b)	(b[1])
#define PES_PACKET_SYNC_BYTE3(b)	(b[2])
#define PES_PACKET_STREAM_ID(b)		(b[3])
#define PES_PACKET_LEN(b)		((b[4]) << 8) | (b[5])

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

#define PES_PACKET_PTS(b)		((uint32_t)((b[9] & 0x0E) << 29) | \
					 (uint32_t)(b[10] << 22) | \
					 (uint32_t)((b[11] & 0xFE) << 14) | \
					 (uint32_t)(b[12] << 7) | \
					 (uint32_t)(b[13] >> 1))

#define PES_PACKET_DTS(b)		((uint32_t)((b[14] & 0x0E) << 29) | \
					 (uint32_t)(b[15] << 22) | \
					 (uint32_t)((b[16] & 0xFE) << 14) | \
					 (uint32_t)(b[17] << 7) | \
					 (uint32_t)(b[18] >> 1))



/* Structure containing single channel */
typedef struct dvbshout_channel_s {

	int num;				// channel number
	int fd;					// demux file descriptor
	int pid;				// Packet Identifier of audio stream

	
	// Metadata about the channel
	char name[STR_BUF_SIZE];	// channel name
	char genre[STR_BUF_SIZE];	// genre
	char description[STR_BUF_SIZE];	// description
	char url[STR_BUF_SIZE];		// Informational URL


	int continuity_count;	// TS packet continuity counter
	int pes_stream_id;		// PES stream ID
	size_t pes_remaining;	// Number of bytes remaining in current PES packet
	uint32_t pes_pts;		// Presentation Timestamp for current PES packet
	uint32_t pes_dts;		// Decode Timestamp for current PES packet
	
	shout_t *shout;				// libshout structure
	char is_public;				// announce existance?
	char mount[STR_BUF_SIZE];	// mount point

	mpa_header_t mpah;		// Parsed MPEG audio header
	int synced;				// Have MPA sync?
	

	uint8_t * buf;			// MPEG Audio Buffer (with 4 nulls bytes)
	uint8_t * buf_ptr;		// Pointer to start of audio data
	uint32_t buf_size;		// Usable size of MPEG Audio Buffer
	uint32_t buf_used;		// Amount of buffer used
	
	RtpSession * sess;		// Multicast RTP session
	char multicast_ip[STR_BUF_SIZE];	// Multicast IP
	char multicast_local[STR_BUF_SIZE];	// Local IP
	int multicast_port;					// Multicast Port
	int multicast_ttl;					// Multicast TTL
	int multicast_mtu;					// Maxium Transmission Unit (of payload)
	int multicast_ts;					// Session Timestamp
	
	int frames_per_packet;				// Number of MPEG audio frames per packet
	int payload_size;					// Size of the payload
	

} dvbshout_channel_t;


/* Structure server settings */
typedef struct dvbshout_multicast_s {
	int ttl;
	int port;
	int mtu;
	char interface[STR_BUF_SIZE];
} dvbshout_multicast_t;


/* Structure server settings */
typedef struct dvbshout_server_s {
	char host[STR_BUF_SIZE];
	int port;
	char user[STR_BUF_SIZE];
	char password[STR_BUF_SIZE];
	int protocol;

} dvbshout_server_t;


/* Structure containing tuning settings 
   - not all fields are used for every interface
*/
typedef struct dvbshout_tuning_s {

	unsigned char card;			// Card number
	unsigned char type;			// Card type (s/c/t)
	
	unsigned int frequency;		// Frequency (Hz) (kHz for QPSK)
	unsigned char polarity;		// Polarity
	unsigned int symbol_rate;	// Symbols per Second (Hz)
	unsigned int diseqc;
	int tone;					// 22kHz tone (-1 = auto)
	
	fe_spectral_inversion_t  inversion;
	fe_bandwidth_t  bandwidth;
	fe_code_rate_t  code_rate_hp;
	fe_code_rate_t  code_rate_lp;
	fe_code_rate_t  fec_inner;
	fe_modulation_t  modulation;
	fe_hierarchy_t  hierarchy;
	fe_transmit_mode_t  transmission_mode;
	fe_guard_interval_t  guard_interval;
	

} dvbshout_tuning_t;




/* In dvbshout.c */
extern int channel_count;
extern dvbshout_tuning_t *dvbshout_tuning;
extern dvbshout_channel_t *channel_map[MAX_PID_COUNT];
extern dvbshout_channel_t *channels[MAX_CHANNEL_COUNT];
extern dvbshout_server_t dvbshout_server;
extern dvbshout_multicast_t dvbshout_multicast;


/* In tune.c */
int tune_it(int fd_frontend, dvbshout_tuning_t *set);
dvbshout_tuning_t * init_tuning_defaults();

/* In pes.c */
unsigned char* parse_pes( unsigned char* buf, int size, size_t *payload_size, dvbshout_channel_t *chan);


/* In parse_config.c */
int parse_config( char *filepath );



#endif
