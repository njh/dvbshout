/*
 *  mpegtools for the Siemens Fujitsu DVB PCI card
 *
 * Copyright (C) 2000, 2001 Marcus Metzler 
 *            for convergence integrated media GmbH
 * 
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

#include "ringbuffy.h"
#include "ctools.h"

#ifndef _REMUX_H_
#define _REMUX_H_

#ifdef __cplusplus
extern "C" {
#endif				/* __cplusplus */

	typedef struct video_i{
		u32 horizontal_size;
		u32 vertical_size 	;
		u32 aspect_ratio	;
		double framerate	;
		u32 video_format;
		u32 bit_rate 	;
		u32 comp_bit_rate	;
		u32 vbv_buffer_size;
		u32 CSPF 		;
		u32 off;
	} VideoInfo; 		

	typedef struct audio_i{
		int layer;
		u32 bit_rate;
		u32 frequency;
		u32 mode;
		u32 mode_extension;
		u32 emphasis;
		u32 framesize;
		u32 off;
	} AudioInfo;



	typedef
	struct PTS_list_struct{
		u32 PTS;
		int pos;
		u32 dts;
		int spos;
	} PTS_List;

	typedef
	struct frame_list_struct{
		int type;
		int pos;
		u32 FRAME;
		u32 time;
		u32 pts;
		u32 dts;
	} FRAME_List;

	typedef
	struct remux_struct{
		ringbuffy vid_buffy;
		ringbuffy aud_buffy;
		PTS_List vpts_list[MAX_PTS];
		PTS_List apts_list[MAX_PTS];
		FRAME_List vframe_list[MAX_FRAME];
		FRAME_List aframe_list[MAX_FRAME];
		int vptsn;
		int aptsn;
		int vframen;
		int aframen;
		long apes;
		long vpes;
		u32 vframe;
		u32 aframe;
		u32 vcframe;
		u32 acframe;
		u32 vpts;
		u32 vdts;
		u32 apts;
		u32 vpts_old;
		u32 apts_old;
		u32 SCR;
		u32 apts_off;
		u32 vpts_off;
		u32 apts_delay;
		u32 vpts_delay;
		u32 dts_delay;
		AudioInfo audio_info;
		VideoInfo video_info;
		int fin;
		int fout;
		long int awrite;
		long int vwrite;
		long int aread;
		long int vread;
		u32 group;
		u32 groupframe;
		u32 muxr;
		int pack_size;
		u32 time_off;
	} Remux;

	enum { NONE, I_FRAME, P_FRAME, B_FRAME, D_FRAME };

	void remux(int fin, int fout, int pack_size, int mult);
	void remux2(int fdin, int fdout);
#ifdef __cplusplus
}
#endif				/* __cplusplus */

#endif /*_REMUX_H_*/
