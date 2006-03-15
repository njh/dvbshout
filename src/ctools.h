/*
 *  mpegtools for the Siemens Fujitsu DVB PCI card
 *
 * Copyright (C) 2000, 2001 Marcus Metzler 
 *            for convergence integrated media GmbH
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * Or, point your browser to http://www.gnu.org/copyleft/gpl.html
 * 

 * The author can be reached at marcus@convergence.de, 

 * the project's page is at http://linuxtv.org/dvb/
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <libgen.h>
#include <stdint.h>
#include <netdb.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "ringbuffy.h"
#include "transform.h"

#ifndef _CTOOLS_H_
#define _CTOOLS_H_

#define VIDEO_MODE_PAL		0
#define VIDEO_MODE_NTSC		1

#ifdef __cplusplus
extern "C" {
#endif				/* __cplusplus */
	
	typedef uint8_t u8;
	typedef uint16_t u16;
	typedef uint32_t u32;
	typedef uint64_t u64;
	
	
	enum {PS_STREAM, TS_STREAM, PES_STREAM};
	enum {pDUNNO, pPAL, pNTSC};

	u32 trans_pts_dts(u8 *pts);

/*
  PES
*/

#define PROG_STREAM_MAP  0xBC
#ifndef PRIVATE_STREAM1
#define PRIVATE_STREAM1  0xBD
#endif
#define PADDING_STREAM   0xBE
#ifndef PRIVATE_STREAM2
#define PRIVATE_STREAM2  0xBF
#endif
#define AUDIO_STREAM_S   0xC0
#define AUDIO_STREAM_E   0xDF
#define VIDEO_STREAM_S   0xE0
#define VIDEO_STREAM_E   0xEF
#define ECM_STREAM       0xF0
#define EMM_STREAM       0xF1
#define DSM_CC_STREAM    0xF2
#define ISO13522_STREAM  0xF3
#define PROG_STREAM_DIR  0xFF

#define BUFFYSIZE    10*MAX_PLENGTH
#define MAX_PTS      8192
#define MAX_FRAME    8192
#define MAX_PACK_L   4096
#define PS_HEADER_L1    14
#define PS_HEADER_L2    (PS_HEADER_L1+18)
#define MAX_H_SIZE   (PES_H_MIN + PS_HEADER_L1 + 5)
#define PES_MIN         7
#define PES_H_MIN       9

//flags1
#define FLAGS            0x40
#define SCRAMBLE_FLAGS   0x30
#define PRIORITY_FLAG    0x08
#define DATA_ALIGN_FLAG  0x04
#define COPYRIGHT_FLAG   0x02
#define ORIGINAL_FLAG    0x01

//flags2
#define PTS_DTS_FLAGS    0xC0
#define ESCR_FLAG        0x20
#define ES_RATE_FLAG     0x10
#define DSM_TRICK_FLAG   0x08
#define ADD_CPY_FLAG     0x04
#define PES_CRC_FLAG     0x02
#define PES_EXT_FLAG     0x01

//pts_dts flags 
#define PTS_ONLY         0x80
#define PTS_DTS          0xC0

//private flags
#define PRIVATE_DATA     0x80
#define HEADER_FIELD     0x40
#define PACK_SEQ_CTR     0x20
#define P_STD_BUFFER     0x10
#define PES_EXT_FLAG2    0x01

#define MPEG1_2_ID       0x40
#define STFF_LNGTH_MASK  0x3F


	typedef struct pes_packet_{
		u8 stream_id;
		u8 llength[2];
		uint32_t length;
		u8 flags1;
		u8 flags2;
		u8 pes_hlength;
		u8 pts[5];
		u8 dts[5];
		u8 escr[6];
		u8 es_rate[3];
		u8 trick;
		u8 add_cpy;
		u8 prev_pes_crc[2];
		u8 priv_flags;
		u8 pes_priv_data[16];
		u8 pack_field_length;
		u8 *pack_header;
		u8 pck_sqnc_cntr;
		u8 org_stuff_length;
		u8 p_std[2];
		u8 pes_ext_lngth;
		u8 *pes_ext;
		u8 *pes_pckt_data;
		int padding;
		int mpeg;
		int mpeg1_pad;
		u8 *mpeg1_headr;
		u8 stuffing;
	} pes_packet;

	void init_pes(pes_packet *p);
	void kill_pes(pes_packet *p);
	void setlength_pes(pes_packet *p);
	void nlength_pes(pes_packet *p);
	int cwrite_pes(u8 *buf, pes_packet *p, long length);
	void write_pes(int fd, pes_packet *p);
	int read_pes(int f, pes_packet *p);
	void cread_pes(char *buf, pes_packet *p);

/*
   Transport Stream
*/

#define TS_SIZE        188
#define TRANS_ERROR    0x80
#define PAY_START      0x40
#define TRANS_PRIO     0x20
#define PID_MASK_HI    0x1F
//flags
#define TRANS_SCRMBL1  0x80
#define TRANS_SCRMBL2  0x40
#define ADAPT_FIELD    0x20
#define PAYLOAD        0x10
#define COUNT_MASK     0x0F

// adaptation flags
#define DISCON_IND     0x80
#define RAND_ACC_IND   0x40
#define ES_PRI_IND     0x20
#define PCR_FLAG       0x10
#define OPCR_FLAG      0x08
#define SPLICE_FLAG    0x04
#define TRANS_PRIV     0x02
#define ADAP_EXT_FLAG  0x01

// adaptation extension flags
#define LTW_FLAG       0x80
#define PIECE_RATE     0x40
#define SEAM_SPLICE    0x20

	typedef struct  ts_packet_{
		u8 pid[2];
		u8 flags;
		u8 count;
		u8 data[184];
		u8 adapt_length;
		u8 adapt_flags;
		u8 pcr[6];
		u8 opcr[6];
		u8 splice_count;
		u8 priv_dat_len;
		u8 *priv_dat;
		u8 adapt_ext_len;
		u8 adapt_eflags;
		u8 ltw[2];
		u8 piece_rate[3];
		u8 dts[5];
		int rest;
		u8 stuffing;
	} ts_packet;

	void init_ts(ts_packet *p);
	void kill_ts(ts_packet *p);
	unsigned short pid_ts(ts_packet *p);
	int cwrite_ts(u8 *buf, ts_packet *p, long length);
	void write_ts(int fd, ts_packet *p);
	int read_ts(int f, ts_packet *p);
	void cread_ts (char *buf, ts_packet *p, long length);


/*
   Program Stream
*/

#define PACK_STUFF_MASK  0x07

#define FIXED_FLAG       0x02
#define CSPS_FLAG        0x01
#define SAUDIO_LOCK_FLAG 0x80
#define SVIDEO_LOCK_FLAG 0x40

#define PS_MAX 200

	typedef struct ps_packet_{
		u8 scr[6];
		u8 mux_rate[3];
		u8 stuff_length;
		u8 *data;
		u8 sheader_llength[2];
		int sheader_length;
		u8 rate_bound[3];
		u8 audio_bound;
		u8 video_bound;
		u8 reserved;
		int npes;
		int mpeg;
	} ps_packet;

	void init_ps(ps_packet *p);
	void kill_ps(ps_packet *p);
	void setlength_ps(ps_packet *p);
	u32 scr_base_ps(ps_packet *p);
	u16 scr_ext_ps(ps_packet *p);
	int mux_ps(ps_packet *p);
	int rate_ps(ps_packet *p);
	int cwrite_ps(u8 *buf, ps_packet *p, long length);
	void write_ps(int fd, ps_packet *p);
	int read_ps (int f, ps_packet *p);
	void cread_ps (char *buf, ps_packet *p, long length);



#define MAX_PLENGTH 0xFFFF

	typedef struct sectionstruct {
		int  id;
		int length;
		int found;
		u8 payload[4096+3];
	} section;


	typedef u32 tflags;
#define MAXFILT 32
#define MASKL 16
	typedef struct trans_struct {
		int found;
		u8 packet[188];
		u16 pid[MAXFILT];
		u8 mask[MAXFILT*MASKL];
		u8 filt[MAXFILT*MASKL];
		u8 transbuf[MAXFILT*188];
		int transcount[MAXFILT];
		section sec[MAXFILT];
	        tflags is_full;
	        tflags pes_start;
	        tflags pes_started;
	        tflags pes;
	        tflags set;
	} trans;


	void init_trans(trans *p);
	int set_trans_filt(trans *p, int filtn, u16 pid, u8 *mask, 
			   u8 *filt, int pes);

	void clear_trans_filt(trans *p,int filtn);
	int filt_is_set(trans *p, int filtn);
	int pes_is_set(trans *p, int filtn);
	int pes_is_started(trans *p, int filtn);
	int pes_is_start(trans *p, int filtn);
	int filt_is_ready(trans *p,int filtn);

	void trans_filt(u8 *buf, int count, trans *p);
	void filter(trans *p);
	void pes_filter(trans *p, int filtn, int off);
	void sec_filter(trans *p, int filtn, int off);
	int get_filt_buf(trans *p, int filtn,u8 **buf); 
	section *get_filt_sec(trans *p, int filtn); 


	typedef struct a2pstruct{
		int type;
		int fd;
		int found;
		int length;
		int headr;
		int plength;
		u8 cid;
		u8 flags;
		u8 abuf[MAX_PLENGTH];
		int alength;
		u8 vbuf[MAX_PLENGTH];
		int vlength;
	        u8 last_av_pts[4];
		u8 av_pts[4];
		u8 scr[4];
		u8 pid0;
		u8 pid1;
		u8 pidv;
		u8 pida;
	} a2p;



	void get_pespts(u8 *av_pts,u8 *pts);
	void init_a2p(a2p *p);
	void av_pes_to_pes(u8 *buf,int count, a2p *p);
	int w_pesh(u8 id,int length ,u8 *pts, u8 *obuf);
	int w_tsh(u8 id,int length ,u8 *pts, u8 *obuf,a2p *p,int startpes);
	void pts2pts(u8 *av_pts, u8 *pts);
	void write_ps_headr(ps_packet *p,u8 *pts,int fd);

	typedef struct p2t_s{
		u8           pes[TS_SIZE];
		u8           counter;
		long int     pos;
		int          frags;
		void         (*t_out)(u8 const *buf);
	} p2t_t;

	void twrite(u8 const *buf);
	void init_p2t(p2t_t *p, void (*fkt)(u8 const *buf));
	long int find_pes_header(u8 const *buf, long int length, int *frags);
	void pes_to_ts( u8 const *buf, long int length, u16 pid, p2t_t *p);
	void p_to_t( u8 const *buf, long int length, u16 pid, u8 *counter, 
		    void (*ts_write)(u8 const *));


	int write_pes_header(u8 id,int length , long PTS, 
			     u8 *obuf, int stuffing);

	int write_ps_header(uint8_t *buf, 
			    uint32_t   SCR, 
			    long  muxr,
			    uint8_t    audio_bound,
			    uint8_t    fixed,
			    uint8_t    CSPS,
			    uint8_t    audio_lock,
			    uint8_t    video_lock,
			    uint8_t    video_bound,
			    uint8_t    stream1,
			    uint8_t    buffer1_scale,
			    uint32_t   buffer1_size,
			    uint8_t    stream2,
			    uint8_t    buffer2_scale,
			    uint32_t   buffer2_size);                    
	



	void split_mpg(char *name, uint64_t size);
	void cut_mpg(char *name, uint64_t size);
	int http_open (char *url);
#ifdef __cplusplus
}
#endif				/* __cplusplus */

#endif /*_CTOOLS_H_*/
