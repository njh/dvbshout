/* dvb_defaults.h

   Copyright (C) Dave Chapman 2002, Nicholas Humfrey 2006

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

// The size of MPEG2 TS packets
#define IPACK_STORAGE_SIZE		2048

// There seems to be a limit of 16 simultaneous filters in the driver
#define MAX_CHANNEL_COUNT		16

// Maximum allowed PID value
#define MAX_PID_COUNT			8192

// Maximum allowed PID value
#define STR_BUF_SIZE			1025




/* Structure containing single channel */
typedef struct shout_channel {

	int num;				// channel number
	int fd;					// debux file descriptor
	int apid;				// PID of audio stream
	int stream_id;			// PES stream ID
	int64_t count;			// Number of packets received
	shout_t *shout;			// libshout structure
	
	int ts_count;
	size_t pes_remaining;

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

/* In parse_config.c */
int parse_config( char *filepath );



#endif
