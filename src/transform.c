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


#include "transform.h"
#include <stdlib.h>
#include <string.h>
#include "ctools.h"

static uint8_t tspid0[TS_SIZE] = { 
	0x47, 0x40, 0x00, 0x10, 0x00, 0x00, 0xb0, 0x11, 
	0x00, 0x00, 0xcb, 0x00, 0x00, 0x00, 0x00, 0xe0, 
	0x10, 0x00, 0x01, 0xe4, 0x00, 0x2a, 0xd6, 0x1a, 
	0xfb, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff
};

static	uint8_t tspid1[TS_SIZE] = { 
	0x47, 0x44, 0x00, 0x10, 0x00, 0x02, 0xb0, 0x1c,
	0x00, 0x01, 0xcb, 0x00, 0x00, 0xe0, 0xa0, 0xf0, 
	0x05, 0x48, 0x03, 0x01, 0x00, 0x00, 0x02, 0xe0,
	0xa0, 0xf0, 0x00, 0x03, 0xe0, 0x50, 0xf0, 0x00, 
	0xae, 0xea, 0x4e, 0x48, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff
};

uint32_t trans_pts_dts(uint8_t *pts)
{
	uint32_t wts;
	
	wts = (((pts[0] & 0x06) << 4) | 
	       ((pts[1] & 0xFC) >> 2)) << 24; 
	wts |= (((pts[1] & 0x03) << 6) |
		((pts[2] & 0xFC) >> 2)) << 16; 
	wts |= (((pts[2] & 0x02) << 6) |
		((pts[3] & 0xFE) >> 1)) << 8;
	wts |= (((pts[3] & 0x01) << 7) |
		((pts[4] & 0xFE) >> 1));
	return wts;
}


void get_pespts(uint8_t *av_pts,uint8_t *pts)
{
	
	pts[0] = 0x21 | 
		((av_pts[0] & 0xC0) >>5);
	pts[1] = ((av_pts[0] & 0x3F) << 2) |
		((av_pts[1] & 0xC0) >> 6);
	pts[2] = 0x01 | ((av_pts[1] & 0x3F) << 2) |
		((av_pts[2] & 0x80) >> 6);
	pts[3] = ((av_pts[2] & 0x7F) << 1) |
		((av_pts[3] & 0x80) >> 7);
	pts[4] = 0x01 | ((av_pts[3] & 0x7F) << 1);
}

uint16_t get_pid(uint8_t *pid)
{
	uint16_t pp = 0;

	pp = (pid[0] & PID_MASK_HI)<<8;
	pp |= pid[1];

	return pp;
}

int write_ts_header(uint16_t pid, uint8_t *counter, int pes_start, 
		    uint8_t *buf, uint8_t length)
{
	int i;
	int c = 0;
	int fill;
	uint8_t tshead[4] = { 0x47, 0x00, 0x00, 0x10}; 
        

	fill = TS_SIZE-4-length;
        if (pes_start) tshead[1] = 0x40;
	if (fill) tshead[3] = 0x30;
        tshead[1] |= (uint8_t)((pid & 0x1F00) >> 8);
        tshead[2] |= (uint8_t)(pid & 0x00FF);
        tshead[3] |= ((*counter)++ & 0x0F) ;
        memcpy(buf,tshead,4);
	c+=4;


	if (fill){
		buf[4] = fill-1;
		c++;
		if (fill >1){
			buf[5] = 0x00;
			c++;
		}
		for ( i = 6; i < fill+4; i++){
			buf[i] = 0xFF;
			c++;
		}
	}

        return c;
}


int write_pes_header(uint8_t id,int length , long PTS, uint8_t *obuf, 
		     int stuffing)
{
	uint8_t le[2];
	uint8_t dummy[3];
	uint8_t *pts;
	uint8_t ppts[5];
	long lpts;
	int c;
	uint8_t headr[3] = {0x00, 0x00, 0x01};
	
	lpts = htonl(PTS);
	pts = (uint8_t *) &lpts;
	
	get_pespts(pts,ppts);

	c = 0;
	memcpy(obuf+c,headr,3);
	c += 3;
	memcpy(obuf+c,&id,1);
	c++;

	le[0] = 0;
	le[1] = 0;
	length -= 6+stuffing;

	le[0] |= ((uint8_t)(length >> 8) & 0xFF); 
	le[1] |= ((uint8_t)(length) & 0xFF); 
	memcpy(obuf+c,le,2);
	c += 2;

	if (id == PADDING_STREAM){
		memset(obuf+c,0xff,length);
		c+= length;
		return c;
	}

	dummy[0] = 0x80;
	dummy[1] = 0;
	dummy[2] = 0;
	if (PTS){
		dummy[1] |= PTS_ONLY;
		dummy[2] = 5+stuffing;
	}
	memcpy(obuf+c,dummy,3);
	c += 3;
	memset(obuf+c,0xFF,stuffing);

	if (PTS){
		memcpy(obuf+c,ppts,5);
		c += 5;
	}
	
	return c;
}


void init_p2p(p2p *p, void (*func)(uint8_t *buf, int count, p2p *p), 
	      int repack){
	p->found = 0;
	p->cid = 0;
	p->mpeg = 0;
	memset(p->buf,0,MMAX_PLENGTH);
	p->done = 0;
	p->fd1 = -1;
	p->func = func;
	p->bigend_repack = 0;
	p->repack = 0; 
	if ( repack < MAX_PLENGTH && repack > 265 ){
		p->repack = repack-6;
		p->bigend_repack = (uint16_t)htons((short)
						   ((repack-6) & 0xFFFF));
	} else {
		fprintf(stderr, "Repack size %d is out of range\n",repack);
		exit(1);
	}
}



void pes_repack(p2p *p)
{
	int count = 0;
	int repack = p->repack;
	int rest = p->plength;
	uint8_t buf[MAX_PLENGTH];
	int bfill = 0;
	int diff;
	uint16_t length;

	if (rest < 0) {
                fprintf(stderr,"Error in repack\n");
                return;
        }

        if (!repack){
                fprintf(stderr,"forgot to set repack size\n");
                return;
        }

	if (p->plength == repack){
		memcpy(p->buf+4,(char *)&p->bigend_repack,2);
		p->func(p->buf, repack+6, p);
		return;
	}

	buf[0] = 0x00;
	buf[1] = 0x00;
	buf[2] = 0x01;
	buf[3] = p->cid;
	memcpy(buf+4,(char *)&p->bigend_repack,2);
	memset(buf+6,0,MAX_PLENGTH-6);

	if (p->mpeg == 2){

		if ( rest > repack){
			memcpy(p->buf+4,(char *)&p->bigend_repack,2);
			p->func(p->buf, repack+6, p);
			count += repack+6;
			rest -= repack;
		} else {
			memcpy(buf,p->buf,9+p->hlength);
			bfill = p->hlength;
			count += 9+p->hlength;
			rest -= p->hlength+3;
		}

		while (rest >= repack-3){
			memset(buf+6,0,MAX_PLENGTH-6);
			buf[6] = 0x80;
			buf[7] = 0x00;
			buf[8] = 0x00;
			memcpy(buf+9,p->buf+count,repack-3);
			rest -= repack-3;
			count += repack-3;
			p->func(buf, repack+6, p);
		}
		
		if (rest){
			diff = repack - 3 - rest - bfill;
			if (!bfill){
				buf[6] = 0x80;
				buf[7] = 0x00;
				buf[8] = 0x00;
			}

			if ( diff < PES_MIN){
				length = rest+ diff + bfill+3; 
				buf[4] = (uint8_t)((length & 0xFF00) >> 8);
				buf[5] = (uint8_t)(length & 0x00FF);
				buf[8] = (uint8_t)(bfill+diff);
				memset(buf+9+bfill,0xFF,diff);
				memcpy(buf+9+bfill+diff,p->buf+count,rest);
			} else {
				length = rest+ bfill+3; 
				buf[4] = (uint8_t)((length & 0xFF00) >> 8);
				buf[5] = (uint8_t)(length & 0x00FF);
				memcpy(buf+9+bfill,p->buf+count,rest);
				bfill += rest+9;
				write_pes_header( PADDING_STREAM, diff, 0,
						  buf+bfill, 0);
			}
			p->func(buf, repack+6, p);
		}
	}	

	if (p->mpeg == 1){

		if ( rest > repack){
			memcpy(p->buf+4,(char *)&p->bigend_repack,2);
			p->func(p->buf, repack+6, p);
			count += repack+6;
			rest -= repack;
		} else {
			memcpy(buf,p->buf,6+p->hlength);
			bfill = p->hlength;
			count += 6;
			rest -= p->hlength;
		}

		while (rest >= repack-1){
			memset(buf+6,0,MAX_PLENGTH-6);
			buf[6] = 0x0F;
			memcpy(buf+7,p->buf+count,repack-1);
			rest -= repack-1;
			count += repack-1;
			p->func(buf, repack+6, p);
		}
		

		if (rest){
			diff = repack - 1 - rest - bfill;

			if ( diff < PES_MIN){
				length = rest+ diff + bfill+1; 
				buf[4] = (uint8_t)((length & 0xFF00) >> 8);
				buf[5] = (uint8_t)(length & 0x00FF);
				memset(buf+6,0xFF,diff);
				if (!bfill){
					buf[6+diff] = 0x0F;
				}
				memcpy(buf+7+diff,p->buf+count,rest+bfill);
			} else {
				length = rest+ bfill+1; 
				buf[4] = (uint8_t)((length & 0xFF00) >> 8);
				buf[5] = (uint8_t)(length & 0x00FF);
				if (!bfill){
					buf[6] = 0x0F;
					memcpy(buf+7,p->buf+count,rest);
					bfill = rest+7;
				} else {
					memcpy(buf+6,p->buf+count,rest+bfill);
					bfill += rest+6;
				}
				write_pes_header( PADDING_STREAM, diff, 0,
						  buf+bfill, 0);
			}
			p->func(buf, repack+6, p);
		}
	}	
}




void pes_filt(p2p *p)
{
	int factor = p->mpeg-1;

	if ( p->cid == p->filter) {
		if (p->es)
			write(p->fd1,p->buf+p->hlength+6+3*factor, 
			      p->plength-p->hlength-3*factor);
		else
			write(p->fd1,p->buf,p->plength+6);
	}
}

#define SIZE 4096
void extract_from_pes(int fdin, int fdout, uint8_t id, int es)
{
		p2p p;
		int count = 1;
		uint8_t buf[SIZE];

		init_p2p(&p, NULL, 2048);
		p.fd1 = fdout;
		p.filter = id;
		p.es = es;

		while (count > 0){
			count = read(fdin,buf,SIZE);
			get_pes(buf,count,&p,pes_filt);
		}
}


void pes_dfilt(p2p *p)
{
	int factor = p->mpeg-1;
	int fd =0;
	int type = NOPES;

	switch ( p->cid ) {
		case AUDIO_STREAM_S ... AUDIO_STREAM_E:			
			fd = p->fd1;
			type = AUDIO;
			break;
		case VIDEO_STREAM_S ... VIDEO_STREAM_E:
			fd = p->fd2;
			type = VIDEO;
			break;
	}
	
	if (p->es && !p->startv && type == VIDEO){
		int found = 0;
		int c = 6+p->hlength+3*factor;
		
		if  ( p->flag2 & PTS_DTS ) 
			p->vpts =  ntohl(trans_pts_dts(p->pts)); 
		while ( !found && c+3 < p->plength+6 ){
			if ( p->buf[c] == 0x00 && 
			     p->buf[c+1] == 0x00 && 
			     p->buf[c+2] == 0x01 &&
			     p->buf[c+3] == 0xb3) 
				found = 1;
			else c++;
		}
		if (found){
			p->startv = 1;
			write(fd, p->buf+c, p->plength+6-c);
		}
		fd = 0;
	} 

		
	if ( p->es && !p->starta && type == AUDIO){
		int found = 0;
		int c = 6+p->hlength+3*factor;
		if  ( p->flag2 & PTS_DTS ) 
			p->apts =  ntohl(trans_pts_dts(p->pts));  
		if (p->startv)
			while ( !found && c+1 < p->plength+6){
				if ( p->buf[c] == 0xFF && 
				     (p->buf[c+1] & 0xF8) == 0xF8)
					found = 1;
				else c++;
			}
		if (found){
			p->starta = 1;
			write(fd, p->buf+c, p->plength+6-c);
		}
		fd = 0;
	} 


	if (fd){
		if (p->es)
			write(fd,p->buf+p->hlength+6+3*factor, 
			      p->plength-p->hlength-3*factor);
		else
			write(fd,p->buf,p->plength+6);
	}
} 

int64_t pes_dmx( int fdin, int fdouta, int fdoutv, int es)
{
	p2p p;
	int count = 1;
	uint8_t buf[SIZE];
	uint64_t length = 0;
	uint64_t l = 0;
	int verb = 0;
	
	init_p2p(&p, NULL, 2048);
	p.fd1 = fdouta;
	p.fd2 = fdoutv;
	p.es = es;
	p.startv = 0;
	p.starta = 0;
	p.apts=-1;
	p.vpts=-1;
	
	if (fdin != STDIN_FILENO) verb = 1; 
	
	if (verb) {
		length = lseek(fdin, 0, SEEK_END);
		lseek(fdin,0,SEEK_SET);
	}
	
	while (count > 0){
		count = read(fdin,buf,SIZE);
		l += count;
		if (verb)
			fprintf(stderr,"Demuxing %2.2f %%\r",
				100.*l/length);
		
		get_pes(buf,count,&p,pes_dfilt);
	}
	
	return (int64_t)p.vpts - (int64_t)p.apts;
	
}



static void pes_in_ts(p2p *p)
{
	int l, pes_start;
	uint8_t obuf[TS_SIZE];
	long int c = 0;
	int length = p->plength+6;
	uint16_t pid;
	uint8_t *counter;
	pes_start = 1;
	switch ( p->cid ) {
	case AUDIO_STREAM_S ... AUDIO_STREAM_E:			
		pid = p->pida;
		counter = &p->acounter;
		break;
	case VIDEO_STREAM_S ... VIDEO_STREAM_E:
		pid = p->pidv;
		counter = &p->acounter;

		tspid0[3] |= (p->count0++) 
			& 0x0F ;
		tspid1[3] |= (p->count1++) 
			& 0x0F ;
	
		tspid1[24]  = p->pidv;
		tspid1[23] |= (p->pidv >> 8) & 0x3F;
		tspid1[29]  = p->pida;
		tspid1[28] |= (p->pida >> 8) & 0x3F;
		
		p->func(tspid0,188,p);
		p->func(tspid1,188,p);
		break;
	default:
		return;
	}

	while ( c < length ){
		memset(obuf,0,TS_SIZE);
		if (length - c >= TS_SIZE-4){
			l = write_ts_header(pid, counter, pes_start
					     , obuf, TS_SIZE-4);
			memcpy(obuf+l, p->buf+c, TS_SIZE-l);
			c += TS_SIZE-l;
		} else { 
			l = write_ts_header(pid, counter, pes_start
					     , obuf, length-c);
			memcpy(obuf+l, p->buf+c, TS_SIZE-l);
			c = length;
		}
		p->func(obuf,188,p);
		pes_start = 0;
	}
}



void pes_to_ts2( int fdin, int fdout, uint16_t pida, uint16_t pidv)
{
	p2p p;
	int count = 1;
	uint8_t buf[SIZE];
	uint64_t length = 0;
	uint64_t l = 0;
	int verb = 0;
	
	init_p2p(&p, NULL, 2048);
	p.fd1 = fdout;
	p.pida = pida;
	p.pidv = pidv;
	p.acounter = 0;
	p.vcounter = 0;
	p.count1 = 0;
	p.count0 = 0;
		
	if (fdin != STDIN_FILENO) verb = 1; 

	if (verb) {
		length = lseek(fdin, 0, SEEK_END);
		lseek(fdin,0,SEEK_SET);
	}

	while (count > 0){
		count = read(fdin,buf,SIZE);
		l += count;
		if (verb)
			fprintf(stderr,"Writing TS  %2.2f %%\r",
				100.*l/length);

		get_pes(buf,count,&p,pes_in_ts);
	}
		
}

void write_out(uint8_t *buf, int count,void  *p)
{
	write(STDOUT_FILENO, buf, count);
}


#define IN_SIZE TS_SIZE*10
#define IPACKS 2048
void find_avpids(int fd, uint16_t *vpid, uint16_t *apid)
{
        uint8_t buf[IN_SIZE];
        int count;
        int i;  
        int off =0;

        while ( *apid == 0 || *vpid == 0){
                count = read(fd, buf, IN_SIZE);
                for (i = 0; i < count-7; i++){
                        if (buf[i] == 0x47){
                                if (buf[i+1] & 0x40){
                                        off = 0;
                                        if ( buf[3+i] & 0x20)//adapt field?
                                                off = buf[4+i] + 1;
                                        switch(buf[i+7+off]){
                                        case VIDEO_STREAM_S ... VIDEO_STREAM_E:
                                                *vpid = get_pid(buf+i+1);
                                                break;
                                        case PRIVATE_STREAM1:
                                        case AUDIO_STREAM_S ... AUDIO_STREAM_E:
                                                *apid = get_pid(buf+i+1);
                                                break;
                                        }
                                }
                                i += 187;
                        }
                        if (*apid != 0 && *vpid != 0) break;
                }
        }
}

void find_bavpids(uint8_t *buf, int count, uint16_t *vpid, uint16_t *apid)
{
        int i;  
        int founda = 0;
        int foundb = 0;
        int off = 0;
        
        *vpid = 0;
        *apid = 0;
        for (i = 0; i < count-7; i++){
                if (buf[i] == 0x47){
                        if ((buf[i+1] & 0xF0) == 0x40){
                                off = 0;
                                if ( buf[3+i] & 0x20)  // adaptation field?
                                        off = buf[4+i] + 1;
                                
                                if (buf[off+i+4] == 0x00 && 
                                    buf[off+i+5] == 0x00 &&
                                    buf[off+i+6] == 0x01){
                                        switch(buf[off+i+7]){
                                        case VIDEO_STREAM_S ... VIDEO_STREAM_E:
                                                *vpid = get_pid(buf+i+1);
                                                foundb=1;
                                                break;
                                        case PRIVATE_STREAM1:
                                        case AUDIO_STREAM_S ... AUDIO_STREAM_E:
                                                *apid = get_pid(buf+i+1);
                                                founda=1;
                                                break;
                                        }
                                }
                        }
                        i += 187;
                }
                if (founda && foundb) break;
        }
}


void ts_to_pes( int fdin, uint16_t pida, uint16_t pidv, int ps)
{
	
	uint8_t buf[IN_SIZE];
	uint8_t mbuf[TS_SIZE];
	int i;
	int count = 1;
	uint16_t pid;
	uint16_t dummy;
	ipack pa, pv;
	ipack *p;

	if (fdin != STDIN_FILENO && (!pida || !pidv))
		find_avpids(fdin, &pidv, &pida);

	init_ipack(&pa, IPACKS,write_out, ps);
	init_ipack(&pv, IPACKS,write_out, ps);

 	if ((count = read(fdin,mbuf,TS_SIZE))<0)
	    perror("reading");

	for ( i = 0; i < 188 ; i++){
		if ( mbuf[i] == 0x47 ) break;
	}
	if ( i == 188){
		fprintf(stderr,"Not a TS\n");
		return;
	} else {
		memcpy(buf,mbuf+i,TS_SIZE-i);
		if ((count = read(fdin,mbuf,i))<0)
			perror("reading");
		memcpy(buf+TS_SIZE-i,mbuf,i);
		i = 188;
	}
	count = 1;
	while (count > 0){
		if ((count = read(fdin,buf+i,IN_SIZE-i))<0)
			perror("reading");
		

		if (!pidv){
                        find_bavpids(buf+i, IN_SIZE-i, &pidv, &dummy);
                        if (pidv) fprintf(stderr, "vpid %d (0x%02x)\n",
					  pidv,pidv);
                } 

                if (!pida){
                        find_bavpids(buf+i, IN_SIZE-i, &dummy, &pida);
                        if (pida) fprintf(stderr, "apid %d (0x%02x)\n",
					  pida,pida);
                } 


		for( i = 0; i < count; i+= TS_SIZE){
			uint8_t off = 0;

			if ( count - i < TS_SIZE) break;

			pid = get_pid(buf+i+1);
			if (!(buf[3+i]&0x10)) // no payload?
				continue;
			if (pid == pidv){
				p = &pv;
			} else {
				if (pid == pida){
					p = &pa;
				} else continue;
			}

			if ( buf[1+i]&0x40) {
				if (p->plength == MMAX_PLENGTH-6){
					p->plength = p->found-6;
					p->found = 0;
					send_ipack(p);
					reset_ipack(p);
				}
			}

			if ( buf[3+i] & 0x20) {  // adaptation field?
				off = buf[4+i] + 1;
			}
        
			instant_repack(buf+4+off+i, TS_SIZE-4-off, p);
		}
		i = 0;

	}

}


#define INN_SIZE 2*IN_SIZE
void insert_pat_pmt( int fdin, int fdout)
{
	
	uint8_t buf[INN_SIZE];
	uint8_t mbuf[TS_SIZE];
	int i;
	int count = 1;
	uint16_t pida = 0;
	uint16_t pidv = 0;
	int written,c;
	uint8_t c0 = 0;
	uint8_t c1 = 0;


	find_avpids(fdin, &pidv, &pida);
	
 	count = read(fdin,mbuf,TS_SIZE);
	for ( i = 0; i < 188 ; i++){
		if ( mbuf[i] == 0x47 ) break;
	}
	if ( i == 188){
		fprintf(stderr,"Not a TS\n");
		return;
	} else {
		memcpy(buf,mbuf+i,TS_SIZE-i);
		count = read(fdin,mbuf,i);
		memcpy(buf+TS_SIZE-i,mbuf,i);
		i = 188;
	}
	
	count = 1;
	while (count > 0){
		tspid1[24]  = pidv;
		tspid1[23] |= (pidv >> 8) & 0x3F;
		tspid1[29]  = pida;
		tspid1[28] |= (pida >> 8) & 0x3F;
		
		write(fdout,tspid0,188);
		write(fdout,tspid1,188);

		count = read(fdin,buf+i,INN_SIZE-i);
		
		written = 0;
		while (written < IN_SIZE){
			c = write(fdout,buf,INN_SIZE);
			if (c>0) written += c;
		}
		tspid0[3] |= (c0++) 
			& 0x0F ;
		tspid1[3] |= (c1++) 
			& 0x0F ;
	
		i=0;
	}

}

void get_pes (uint8_t *buf, int count, p2p *p, void (*func)(p2p *p))
{

	int l;
	unsigned short *pl;
	int c=0;

	uint8_t headr[3] = { 0x00, 0x00, 0x01} ;

	while (c < count && (p->mpeg == 0 ||
			     (p->mpeg == 1 && p->found < 7) ||
			     (p->mpeg == 2 && p->found < 9))
	       &&  (p->found < 5 || !p->done)){
		switch ( p->found ){
		case 0:
		case 1:
			if (buf[c] == 0x00) p->found++;
			else p->found = 0;
			c++;
			break;
		case 2:
			if (buf[c] == 0x01) p->found++;
			else if (buf[c] == 0){
				p->found = 2;
			} else p->found = 0;
			c++;
			break;
		case 3:
			p->cid = 0;
			switch (buf[c]){
			case PROG_STREAM_MAP:
			case PRIVATE_STREAM2:
			case PROG_STREAM_DIR:
			case ECM_STREAM     :
			case EMM_STREAM     :
			case PADDING_STREAM :
			case DSM_CC_STREAM  :
			case ISO13522_STREAM:
				p->done = 1;
			case PRIVATE_STREAM1:
			case VIDEO_STREAM_S ... VIDEO_STREAM_E:
			case AUDIO_STREAM_S ... AUDIO_STREAM_E:
				p->found++;
				p->cid = buf[c];
				c++;
				break;
			default:
				p->found = 0;
				break;
			}
			break;
			

		case 4:
			if (count-c > 1){
				pl = (unsigned short *) (buf+c);
				p->plength =  ntohs(*pl);
				p->plen[0] = buf[c];
				c++;
				p->plen[1] = buf[c];
				c++;
				p->found+=2;
			} else {
				p->plen[0] = buf[c];
				p->found++;
				return;
			}
			break;
		case 5:
			p->plen[1] = buf[c];
			c++;
			pl = (unsigned short *) p->plen;
			p->plength = ntohs(*pl);
			p->found++;
			break;


		case 6:
			if (!p->done){
				p->flag1 = buf[c];
				c++;
				p->found++;
				if ( (p->flag1 & 0xC0) == 0x80 ) p->mpeg = 2;
				else {
					p->hlength = 0;
					p->which = 0;
					p->mpeg = 1;
					p->flag2 = 0;
				}
			}
			break;

		case 7:
			if ( !p->done && p->mpeg == 2){
				p->flag2 = buf[c];
				c++;
				p->found++;
			}	
			break;

		case 8:
			if ( !p->done && p->mpeg == 2){
				p->hlength = buf[c];
				c++;
				p->found++;
			}
			break;
			
		default:

			break;
		}
	}

	if (!p->plength) p->plength = MMAX_PLENGTH-6;


	if ( p->done || ((p->mpeg == 2 && p->found >= 9)  || 
	     (p->mpeg == 1 && p->found >= 7)) ){
		switch (p->cid){
			
		case AUDIO_STREAM_S ... AUDIO_STREAM_E:			
		case VIDEO_STREAM_S ... VIDEO_STREAM_E:
		case PRIVATE_STREAM1:

			memcpy(p->buf, headr, 3);
			p->buf[3] = p->cid;
			memcpy(p->buf+4,p->plen,2);

			if (p->mpeg == 2 && p->found == 9){
				p->buf[6] = p->flag1;
				p->buf[7] = p->flag2;
				p->buf[8] = p->hlength;
			}

			if (p->mpeg == 1 && p->found == 7){
				p->buf[6] = p->flag1;
			}


			if (p->mpeg == 2 && (p->flag2 & PTS_ONLY) &&  
			    p->found < 14){
				while (c < count && p->found < 14){
					p->pts[p->found-9] = buf[c];
					p->buf[p->found] = buf[c];
					c++;
					p->found++;
				}
				if (c == count) return;
			}

			if (p->mpeg == 1 && p->which < 2000){

				if (p->found == 7) {
					p->check = p->flag1;
					p->hlength = 1;
				}

				while (!p->which && c < count && 
				       p->check == 0xFF){
					p->check = buf[c];
					p->buf[p->found] = buf[c];
					c++;
					p->found++;
					p->hlength++;
				}

				if ( c == count) return;
				
				if ( (p->check & 0xC0) == 0x40 && !p->which){
					p->check = buf[c];
					p->buf[p->found] = buf[c];
					c++;
					p->found++;
					p->hlength++;

					p->which = 1;
					if ( c == count) return;
					p->check = buf[c];
					p->buf[p->found] = buf[c];
					c++;
					p->found++;
					p->hlength++;
					p->which = 2;
					if ( c == count) return;
				}

				if (p->which == 1){
					p->check = buf[c];
					p->buf[p->found] = buf[c];
					c++;
					p->found++;
					p->hlength++;
					p->which = 2;
					if ( c == count) return;
				}
				
				if ( (p->check & 0x30) && p->check != 0xFF){
					p->flag2 = (p->check & 0xF0) << 2;
					p->pts[0] = p->check;
					p->which = 3;
				} 

				if ( c == count) return;
				if (p->which > 2){
					if ((p->flag2 & PTS_DTS_FLAGS)
					    == PTS_ONLY){
						while (c < count && 
						       p->which < 7){
							p->pts[p->which-2] =
								buf[c];
							p->buf[p->found] = 
								buf[c];
							c++;
							p->found++;
							p->which++;
							p->hlength++;
						}
						if ( c == count) return;
					} else if ((p->flag2 & PTS_DTS_FLAGS) 
						   == PTS_DTS){
						while (c < count && 
						       p->which< 12){
							if (p->which< 7)
								p->pts[p->which
								      -2] =
									buf[c];
							p->buf[p->found] = 
								buf[c];
							c++;
							p->found++;
							p->which++;
							p->hlength++;
						}
						if ( c == count) return;
					}
					p->which = 2000;
				}
							
			}

			while (c < count && p->found < p->plength+6){
				l = count -c;
				if (l+p->found > p->plength+6)
					l = p->plength+6-p->found;
				memcpy(p->buf+p->found, buf+c, l);
				p->found += l;
				c += l;
			}			
			if(p->found == p->plength+6)
				func(p);
			
			break;
		}


		if ( p->done ){
			if( p->found + count - c < p->plength+6){
				p->found += count-c;
				c = count;
			} else {
				c += p->plength+6 - p->found;
				p->found = p->plength+6;
			}
		}

		if (p->plength && p->found == p->plength+6) {
			p->found = 0;
			p->done = 0;
			p->plength = 0;
			memset(p->buf, 0, MAX_PLENGTH);
			if (c < count)
				get_pes(buf+c, count-c, p, func);
		}
	}
	return;
}




void setup_pes2ts( p2p *p, uint32_t pida, uint32_t pidv, 
		   void (*ts_write)(uint8_t *buf, int count, p2p *p))
{
	init_p2p( p, ts_write, 2048);
	p->pida = pida;
	p->pidv = pidv;
	p->acounter = 0;
	p->vcounter = 0;
	p->count1 = 0;
	p->count0 = 0;
}

void kpes_to_ts( p2p *p,uint8_t *buf ,int count )
{
	get_pes(buf,count, p,pes_in_ts);
}


void setup_ts2pes( p2p *pa, p2p *pv, uint32_t pida, uint32_t pidv, 
		   void (*pes_write)(uint8_t *buf, int count, p2p *p))
{
	init_p2p( pa, pes_write, 2048);
	init_p2p( pv, pes_write, 2048);
	pa->pid = pida;
	pv->pid = pidv;
}

void kts_to_pes( p2p *p, uint8_t *buf) // don't need count (=188)
{
	uint8_t off = 0;
	uint16_t pid = 0;

	if (!(buf[3]&PAYLOAD)) // no payload?
		return;

	pid = get_pid(buf+1);
			
	if (pid != p->pid) return;

	if ( buf[1]&PAY_START) {
		if (p->plength == MMAX_PLENGTH-6){
			p->plength = p->found-6;
			p->found = 0;
			pes_repack(p);
		}
	}

	if ( buf[3] & ADAPT_FIELD) {  // adaptation field?
		off = buf[4] + 1;
		if (off+4 > 187) return;
	}
        
	get_pes(buf+4+off, TS_SIZE-4-off, p , pes_repack);
}




// instant repack


void reset_ipack(ipack *p)
{
	p->found = 0;
	p->cid = 0;
	p->plength = 0;
	p->flag1 = 0;
	p->flag2 = 0;
	p->hlength = 0;
	p->mpeg = 0;
	p->check = 0;
	p->which = 0;
	p->done = 0;
	p->count = 0;
	p->size = p->size_orig;
}

void init_ipack(ipack *p, int size,
		void (*func)(uint8_t *buf,  int size, void *priv), int ps)
{
	if ( !(p->buf = malloc(size)) ){
		fprintf(stderr,"Couldn't allocate memory for ipack\n");
		exit(1);
	}
	p->ps = ps;
	p->size_orig = size;
	p->func = func;
	reset_ipack(p);
	p->has_ai = 0;
	p->has_vi = 0;
	p->start = 0;
}

void free_ipack(ipack * p)
{
	if (p->buf) free(p->buf);
}



int get_vinfo(uint8_t *mbuf, int count, VideoInfo *vi, int pr)
{
	uint8_t *headr;
	int found = 0;
        int sw;
	int form = -1;
	int c = 0;

	while (found < 4 && c+4 < count){
		uint8_t *b;

		b = mbuf+c;
		if ( b[0] == 0x00 && b[1] == 0x00 && b[2] == 0x01
		     && b[3] == 0xb3) found = 4;
		else {
			c++;
		}
	}

	if (! found) return -1;
	c += 4;
	if (c+12 >= count) return -1;
	headr = mbuf+c;

	vi->horizontal_size	= ((headr[1] &0xF0) >> 4) | (headr[0] << 4);
	vi->vertical_size	= ((headr[1] &0x0F) << 8) | (headr[2]);
    
        sw = (int)((headr[3]&0xF0) >> 4) ;

        switch( sw ){
	case 1:
		if (pr)
			fprintf(stderr,"Videostream: ASPECT: 1:1");
		vi->aspect_ratio = 100;        
		break;
	case 2:
		if (pr)
			fprintf(stderr,"Videostream: ASPECT: 4:3");
                vi->aspect_ratio = 133;        
		break;
	case 3:
		if (pr)
			fprintf(stderr,"Videostream: ASPECT: 16:9");
                vi->aspect_ratio = 177;        
		break;
	case 4:
		if (pr)
			fprintf(stderr,"Videostream: ASPECT: 2.21:1");
                vi->aspect_ratio = 221;        
		break;

        case 5 ... 15:
		if (pr)
			fprintf(stderr,"Videostream: ASPECT: reserved");
                vi->aspect_ratio = 0;        
		break;

        default:
                vi->aspect_ratio = 0;        
                return -1;
	}

	if (pr)
		fprintf(stderr,"  Size = %dx%d",vi->horizontal_size,
			vi->vertical_size);

        sw = (int)(headr[3]&0x0F);

        switch ( sw ) {
	case 1:
		if (pr)
			fprintf(stderr,"  FRate: 23.976 fps");
                vi->framerate = 24000/1001.;
		form = -1;
		break;
	case 2:
		if (pr)
			fprintf(stderr,"  FRate: 24 fps");
                vi->framerate = 24;
		form = -1;
		break;
	case 3:
		if (pr)
			fprintf(stderr,"  FRate: 25 fps");
                vi->framerate = 25;
		form = VIDEO_MODE_PAL;
		break;
	case 4:
		if (pr)
			fprintf(stderr,"  FRate: 29.97 fps");
                vi->framerate = 30000/1001.;
		form = VIDEO_MODE_NTSC;
		break;
	case 5:
		if (pr)
			fprintf(stderr,"  FRate: 30 fps");
                vi->framerate = 30;
		form = VIDEO_MODE_NTSC;
		break;
	case 6:
		if (pr)
			fprintf(stderr,"  FRate: 50 fps");
                vi->framerate = 50;
		form = VIDEO_MODE_PAL;
		break;
	case 7:
		if (pr)
			fprintf(stderr,"  FRate: 60 fps");
                vi->framerate = 60;
		form = VIDEO_MODE_NTSC;
		break;
	}

	vi->bit_rate = 400*(((headr[4] << 10) & 0x0003FC00UL) 
			    | ((headr[5] << 2) & 0x000003FCUL) | 
			    (((headr[6] & 0xC0) >> 6) & 0x00000003UL));
	
	if (pr){
		fprintf(stderr,"  BRate: %.2f Mbit/s",(vi->bit_rate)/1000000.);
		fprintf(stderr,"\n");
	}
        vi->video_format = form;

	vi->off = c-4;
	return c-4;
}

extern unsigned int bitrates[3][16];
extern uint32_t freq[4];

int get_ainfo(uint8_t *mbuf, int count, AudioInfo *ai, int pr)
{
	uint8_t *headr;
	int found = 0;
	int c = 0;
	int fr =0;
	
	while (!found && c < count){
		uint8_t *b = mbuf+c;

		if ( b[0] == 0xff && (b[1] & 0xf8) == 0xf8)
			found = 1;
		else {
			c++;
		}
	}	

	if (!found) return -1;

	if (c+3 >= count) return -1;
        headr = mbuf+c;

	ai->layer = (headr[1] & 0x06) >> 1;

        if (pr)
		fprintf(stderr,"Audiostream: Layer: %d", 4-ai->layer);


	ai->bit_rate = bitrates[(3-ai->layer)][(headr[2] >> 4 )]*1000;

	if (pr){
		if (ai->bit_rate == 0)
			fprintf (stderr,"  Bit rate: free");
		else if (ai->bit_rate == 0xf)
			fprintf (stderr,"  BRate: reserved");
		else
			fprintf (stderr,"  BRate: %d kb/s", ai->bit_rate/1000);
	}

	fr = (headr[2] & 0x0c ) >> 2;
	ai->frequency = freq[fr]*100;
	
	if (pr){
		if (ai->frequency == 3)
			fprintf (stderr, "  Freq: reserved\n");
		else
			fprintf (stderr,"  Freq: %2.1f kHz\n", 
				 ai->frequency/1000.);
	}
	ai->off = c;
	return c;
}

unsigned int ac3_bitrates[32] =
    {32,40,48,56,64,80,96,112,128,160,192,224,256,320,384,448,512,576,640,
     0,0,0,0,0,0,0,0,0,0,0,0,0};

uint32_t ac3_freq[4] = {480, 441, 320, 0};
uint32_t ac3_frames[3][32] =
    {{64,80,96,112,128,160,192,224,256,320,384,448,512,640,768,896,1024,
      1152,1280,0,0,0,0,0,0,0,0,0,0,0,0,0},
     {69,87,104,121,139,174,208,243,278,348,417,487,557,696,835,975,1114,
      1253,1393,0,0,0,0,0,0,0,0,0,0,0,0,0},
     {96,120,144,168,192,240,288,336,384,480,576,672,768,960,1152,1344,
      1536,1728,1920,0,0,0,0,0,0,0,0,0,0,0,0,0}}; 

int get_ac3info(uint8_t *mbuf, int count, AudioInfo *ai, int pr)
{
	uint8_t *headr;
	int found = 0;
	int c = 0;
	uint8_t frame;
	int fr = 0;

	while ( !found  && c < count){
		uint8_t *b = mbuf+c;
		if ( b[0] == 0x0b &&  b[1] == 0x77 )
			found = 1;
		else {
			c++;
		}
	}	


	if (!found){
		return -1;
	}
	ai->off = c;

	if (c+5 >= count) return -1;

	ai->layer = 0;  // 0 for AC3
        headr = mbuf+c+2;

	frame = (headr[2]&0x3f);
	ai->bit_rate = ac3_bitrates[frame>>1]*1000;

	if (pr) fprintf (stderr,"  BRate: %d kb/s", ai->bit_rate/1000);

	fr = (headr[2] & 0xc0 ) >> 6;
	ai->frequency = freq[fr]*100;
	if (pr) fprintf (stderr,"  Freq: %d Hz\n", ai->frequency);

	ai->framesize = ac3_frames[fr][frame >> 1];
	if ((frame & 1) &&  (fr == 1)) ai->framesize++;
	ai->framesize = ai->framesize << 1;
	if (pr) fprintf (stderr,"  Framesize %d\n", ai->framesize);

	return c;
}


void ps_pes(ipack *p)
{
	int check;
	uint8_t pbuf[PS_HEADER_L2];
	static int muxr = 0;
	static int ai = 0;
	static int vi = 0;
	static int start = 0;
	static uint32_t SCR = 0;

	if (p->mpeg == 2){
		switch(p->buf[3]){
		case VIDEO_STREAM_S ... VIDEO_STREAM_E:
			if (!p->has_vi){
				if(get_vinfo(p->buf, p->count, &p->vi,1) >=0) {
					p->has_vi = 1;
					vi = p->vi.bit_rate;
				}
			} 			
			break;

		case AUDIO_STREAM_S ... AUDIO_STREAM_E:
			if (!p->has_ai){
				if(get_ainfo(p->buf, p->count, &p->ai,1) >=0) {
					p->has_ai = 1;
					ai = p->ai.bit_rate;
				}
			} 
			break;
		}

		if (p->has_vi && vi && !muxr){
			muxr = (vi+ai)/400;
		}

		if ( start && muxr && (p->buf[7] & PTS_ONLY) && (p->has_ai || 
				       p->buf[9+p->buf[8]+4] == 0xb3)){  
			SCR = trans_pts_dts(p->pts)-3600;
			
			check = write_ps_header(pbuf,
						SCR,
						muxr, 1, 0, 0, 1, 1, 1, 
						0, 0, 0, 0, 0, 0);

			p->func(pbuf, check , p->data);
		}

		if (muxr && !start && vi){
			SCR = trans_pts_dts(p->pts)-3600;
			check = write_ps_header(pbuf,
						SCR, 
						muxr, 1, 0, 0, 1, 1, 1, 
						0xC0, 0, 64, 0xE0, 1, 460);
			start = 1;
			p->func(pbuf, check , p->data);
		}

		if (start)
			p->func(p->buf, p->count, p->data);
	}
}

void send_ipack(ipack *p)
{
	int streamid=0;
	int off;
	int ac3_off = 0;
	AudioInfo ai;
	int nframes= 0;
	int f=0;

	if (p->count < 10) return;
	p->buf[3] = p->cid;
	p->buf[4] = (uint8_t)(((p->count-6) & 0xFF00) >> 8);
	p->buf[5] = (uint8_t)((p->count-6) & 0x00FF);

	
	if (p->cid == PRIVATE_STREAM1){

		off = 9+p->buf[8];
		streamid = p->buf[off];
		if ((streamid & 0xF8) == 0x80){
			ai.off = 0;
			ac3_off = ((p->buf[off+2] << 8)| p->buf[off+3]);
			if (ac3_off < p->count)
				f=get_ac3info(p->buf+off+3+ac3_off, 
					      p->count-ac3_off, &ai,0);
			if ( !f ){
				nframes = (p->count-off-3-ac3_off)/ 
					ai.framesize + 1;
				p->buf[off+2] = (ac3_off >> 8)& 0xFF;
				p->buf[off+3] = (ac3_off)& 0xFF;
				p->buf[off+1] = nframes;
				
				ac3_off +=  nframes * ai.framesize - p->count;
			}
		}
	} 
	
	if (p->ps) ps_pes(p);
	else p->func(p->buf, p->count, p->data);

	switch ( p->mpeg ){
	case 2:		
		
		p->buf[6] = 0x80;
		p->buf[7] = 0x00;
		p->buf[8] = 0x00;
		p->count = 9;

		if (p->cid == PRIVATE_STREAM1 && (streamid & 0xF8)==0x80 ){
			p->count += 4;
			p->buf[9] = streamid;
			p->buf[10] = (ac3_off >> 8)& 0xFF;
			p->buf[11] = (ac3_off)& 0xFF;
			p->buf[12] = 0;
		}
		
		break;
	case 1:
		p->buf[6] = 0x0F;
		p->count = 7;
		break;
	}

}


static void write_ipack(ipack *p, uint8_t *data, int count)
{

	uint8_t headr[3] = { 0x00, 0x00, 0x01} ;
	int diff =0;

	if (p->count < 6){
		if (trans_pts_dts(p->pts) > trans_pts_dts(p->last_pts))
			memcpy(p->last_pts, p->pts, 5);
		p->count = 0;
		memcpy(p->buf+p->count, headr, 3);
		p->count += 6;
	}
	if ( p->size == p->size_orig && p->plength &&
	     (diff = 6+p->plength - p->found + p->count +count) > p->size &&
	     diff < 3*p->size/2){
		
			p->size = diff/2;
//			fprintf(stderr,"size: %d \n",p->size);
	}
	if (p->count + count < p->size){
		memcpy(p->buf+p->count, data, count); 
		p->count += count;
	} else {
		int rest = p->size - p->count;
		if (rest < 0) rest = 0;
		memcpy(p->buf+p->count, data, rest);
		p->count += rest;
//		fprintf(stderr,"count: %d \n",p->count);
		send_ipack(p);
		if (count - rest > 0)
			write_ipack(p, data+rest, count-rest);
	}
}

void instant_repack (uint8_t *buf, int count, ipack *p)
{

	int l;
	unsigned short *pl;
	int c=0;

	while (c < count && (p->mpeg == 0 ||
			     (p->mpeg == 1 && p->found < 7) ||
			     (p->mpeg == 2 && p->found < 9))
	       &&  (p->found < 5 || !p->done)){
		switch ( p->found ){
		case 0:
		case 1:
			if (buf[c] == 0x00) p->found++;
			else p->found = 0;
			c++;
			break;
		case 2:
			if (buf[c] == 0x01) p->found++;
			else if (buf[c] == 0){
				p->found = 2;
			} else p->found = 0;
			c++;
			break;
		case 3:
			p->cid = 0;
			switch (buf[c]){
			case PROG_STREAM_MAP:
			case PRIVATE_STREAM2:
			case PROG_STREAM_DIR:
			case ECM_STREAM     :
			case EMM_STREAM     :
			case PADDING_STREAM :
			case DSM_CC_STREAM  :
			case ISO13522_STREAM:
				p->done = 1;
			case PRIVATE_STREAM1:
			case VIDEO_STREAM_S ... VIDEO_STREAM_E:
			case AUDIO_STREAM_S ... AUDIO_STREAM_E:
				p->found++;
				p->cid = buf[c];
				c++;
				break;
			default:
				p->found = 0;
				break;
			}
			break;
			

		case 4:
			if (count-c > 1){
				pl = (unsigned short *) (buf+c);
				p->plength =  ntohs(*pl);
				p->plen[0] = buf[c];
				c++;
				p->plen[1] = buf[c];
				c++;
				p->found+=2;
			} else {
				p->plen[0] = buf[c];
				p->found++;
				return;
			}
			break;
		case 5:
			p->plen[1] = buf[c];
			c++;
			pl = (unsigned short *) p->plen;
			p->plength = ntohs(*pl);
			p->found++;
			break;


		case 6:
			if (!p->done){
				p->flag1 = buf[c];
				c++;
				p->found++;
				if ( (p->flag1 & 0xC0) == 0x80 ) p->mpeg = 2;
				else {
					p->hlength = 0;
					p->which = 0;
					p->mpeg = 1;
					p->flag2 = 0;
				}
			}
			break;

		case 7:
			if ( !p->done && p->mpeg == 2){
				p->flag2 = buf[c];
				c++;
				p->found++;
			}	
			break;

		case 8:
			if ( !p->done && p->mpeg == 2){
				p->hlength = buf[c];
				c++;
				p->found++;
			}
			break;
			
		default:

			break;
		}
	}


	if (c == count) return;

	if (!p->plength) p->plength = MMAX_PLENGTH-6;


	if ( p->done || ((p->mpeg == 2 && p->found >= 9)  || 
	     (p->mpeg == 1 && p->found >= 7)) ){
		switch (p->cid){
			
		case AUDIO_STREAM_S ... AUDIO_STREAM_E:			
		case VIDEO_STREAM_S ... VIDEO_STREAM_E:
		case PRIVATE_STREAM1:
			
			if (p->mpeg == 2 && p->found == 9){
				write_ipack(p, &p->flag1, 1);
				write_ipack(p, &p->flag2, 1);
				write_ipack(p, &p->hlength, 1);
			}

			if (p->mpeg == 1 && p->found == 7){
				write_ipack(p, &p->flag1, 1);
			}


			if (p->mpeg == 2 && (p->flag2 & PTS_ONLY) &&  
			    p->found < 14){
				while (c < count && p->found < 14){
					p->pts[p->found-9] = buf[c];
					write_ipack(p, buf+c, 1);
					c++;
					p->found++;
				}
				if (c == count) return;
			}

			if (p->mpeg == 1 && p->which < 2000){

				if (p->found == 7) {
					p->check = p->flag1;
					p->hlength = 1;
				}

				while (!p->which && c < count && 
				       p->check == 0xFF){
					p->check = buf[c];
					write_ipack(p, buf+c, 1);
					c++;
					p->found++;
					p->hlength++;
				}

				if ( c == count) return;
				
				if ( (p->check & 0xC0) == 0x40 && !p->which){
					p->check = buf[c];
					write_ipack(p, buf+c, 1);
					c++;
					p->found++;
					p->hlength++;

					p->which = 1;
					if ( c == count) return;
					p->check = buf[c];
					write_ipack(p, buf+c, 1);
					c++;
					p->found++;
					p->hlength++;
					p->which = 2;
					if ( c == count) return;
				}

				if (p->which == 1){
					p->check = buf[c];
					write_ipack(p, buf+c, 1);
					c++;
					p->found++;
					p->hlength++;
					p->which = 2;
					if ( c == count) return;
				}
				
				if ( (p->check & 0x30) && p->check != 0xFF){
					p->flag2 = (p->check & 0xF0) << 2;
					p->pts[0] = p->check;
					p->which = 3;
				} 

				if ( c == count) return;
				if (p->which > 2){
					if ((p->flag2 & PTS_DTS_FLAGS)
					    == PTS_ONLY){
						while (c < count && 
						       p->which < 7){
							p->pts[p->which-2] =
								buf[c];
							write_ipack(p,buf+c,1);
							c++;
							p->found++;
							p->which++;
							p->hlength++;
						}
						if ( c == count) return;
					} else if ((p->flag2 & PTS_DTS_FLAGS) 
						   == PTS_DTS){
						while (c < count && 
						       p->which< 12){
							if (p->which< 7)
								p->pts[p->which
								      -2] =
									buf[c];
							write_ipack(p,buf+c,1);
							c++;
							p->found++;
							p->which++;
							p->hlength++;
						}
						if ( c == count) return;
					}
					p->which = 2000;
				}
							
			}

			while (c < count && p->found < p->plength+6){
				l = count -c;
				if (l+p->found > p->plength+6)
					l = p->plength+6-p->found;
				write_ipack(p, buf+c, l);
				p->found += l;
				c += l;
			}	
		
			break;
		}


		if ( p->done ){
			if( p->found + count - c < p->plength+6){
				p->found += count-c;
				c = count;
			} else {
				c += p->plength+6 - p->found;
				p->found = p->plength+6;
			}
		}

		if (p->plength && p->found == p->plength+6) {
			send_ipack(p);
			reset_ipack(p);
			if (c < count)
				instant_repack(buf+c, count-c, p);
		}
	}
	return;
}

void write_out_es(uint8_t *buf, int count,void  *priv)
{
	ipack *p = (ipack *) priv;
	u8 payl = buf[8]+9+p->start-1;

	write(p->fd, buf+payl, count-payl);
	p->start = 1;
}

void write_out_pes(uint8_t *buf, int count,void  *priv)
{
	ipack *p = (ipack *) priv;
	write(p->fd, buf, count);
}



int64_t ts_demux(int fdin, int fdv_out,int fda_out,uint16_t pida,
		  uint16_t pidv, int es)
{
	uint8_t buf[IN_SIZE];
	uint8_t mbuf[TS_SIZE];
	int i;
	int count = 1;
	uint16_t pid;
	ipack pa, pv;
	ipack *p;
	u8 *sb;
	int64_t apts=0;
	int64_t vpts=0;
	int verb = 0;
	uint64_t length =0;
	uint64_t l=0;
	int perc =0;
	int last_perc =0;

	if (fdin != STDIN_FILENO) verb = 1; 

	if (verb) {
		length = lseek(fdin, 0, SEEK_END);
		lseek(fdin,0,SEEK_SET);
	}

	if (!pida || !pidv)
		find_avpids(fdin, &pidv, &pida);

	if (es){
		init_ipack(&pa, IPACKS,write_out_es, 0);
		init_ipack(&pv, IPACKS,write_out_es, 0);
	} else {
		init_ipack(&pa, IPACKS,write_out_pes, 0);
		init_ipack(&pv, IPACKS,write_out_pes, 0);
	} 
	pa.fd = fda_out;
	pv.fd = fdv_out;
	pa.data = (void *)&pa;
	pv.data = (void *)&pv;

 	count = read(fdin,mbuf,TS_SIZE);
	if (count) l+=count;
	for ( i = 0; i < 188 ; i++){
		if ( mbuf[i] == 0x47 ) break;
	}
	if ( i == 188){
		fprintf(stderr,"Not a TS\n");
		return 0;
	} else {
		memcpy(buf,mbuf+i,TS_SIZE-i);
		count = read(fdin,mbuf,i);
		if (count) l+=count;
		memcpy(buf+TS_SIZE-i,mbuf,i);
		i = 188;
	}
	
	count = 1;
	while (count > 0){
		count = read(fdin,buf+i,IN_SIZE-i);
		if (count) l+=count;
		if (verb && perc >last_perc){
			perc = (100*l)/length;
			fprintf(stderr,"Reading TS  %d %%\r",perc);
			last_perc = perc;
		}
		
		for( i = 0; i < count; i+= TS_SIZE){
			uint8_t off = 0;

			if ( count - i < TS_SIZE) break;

			pid = get_pid(buf+i+1);
			if (!(buf[3+i]&0x10)) // no payload?
				continue;
			if (pid == pidv){
				p = &pv;
			} else {
				if (pid == pida){
					p = &pa;
				} else continue;
			}

			if ( buf[3+i] & 0x20) {  // adaptation field?
				off = buf[4+i] + 1;
			}

			if ( buf[1+i]&0x40) {
				if (p->plength == MMAX_PLENGTH-6){
					p->plength = p->found-6;
					p->found = 0;
					send_ipack(p);
					reset_ipack(p);
				}
				sb = buf+4+off+i;
				if( es && 
				    !p->start && (sb[7] & PTS_DTS_FLAGS)){
					uint8_t *pay = sb+sb[8]+9; 
					int l = TS_SIZE - 13 - off - sb[8];
					if ( pid == pidv &&   
					     (p->start = 
					      get_vinfo( pay, l,&p->vi,1)+1) >0
						){
						vpts = trans_pts_dts(sb+9);
						printf("vpts : %fs\n",
						       vpts/90000.);
					}
					if ( pid == pida && 
					     (p->start = 
					      get_ainfo( pay, l,&p->ai,1)+1) >0
						){
						apts = trans_pts_dts(sb+9);
						printf("apts : %fs\n",
						       apts/90000.);
					}
				}
			}

			if (p->start)
				instant_repack(buf+4+off+i, TS_SIZE-4-off, p);
		}
		i = 0;

	}

	return (vpts-apts);
}

void ts2es(int fdin,  uint16_t pidv)
{
	uint8_t buf[IN_SIZE];
	uint8_t mbuf[TS_SIZE];
	int i;
	int count = 1;
	ipack p;
	int verb = 0;
	uint64_t length =0;
	uint64_t l=0;
	int perc =0;
	int last_perc =0;
	uint16_t pid;

	if (fdin != STDIN_FILENO) verb = 1; 

	if (verb) {
		length = lseek(fdin, 0, SEEK_END);
		lseek(fdin,0,SEEK_SET);
	}

	init_ipack(&p, IPACKS,write_out_es, 0);

	p.fd = STDOUT_FILENO;
	p.data = (void *)&p;

 	count = read(fdin,mbuf,TS_SIZE);
	if (count) l+=count;
	for ( i = 0; i < 188 ; i++){
		if ( mbuf[i] == 0x47 ) break;
	}
	if ( i == 188){
		fprintf(stderr,"Not a TS\n");
		return;
	} else {
		memcpy(buf,mbuf+i,TS_SIZE-i);
		count = read(fdin,mbuf,i);
		if (count) l+=count;
		memcpy(buf+TS_SIZE-i,mbuf,i);
		i = 188;
	}
	
	count = 1;
	while (count > 0){
		count = read(fdin,buf+i,IN_SIZE-i);
		if (count) l+=count;
		if (verb && perc >last_perc){
			perc = (100*l)/length;
			fprintf(stderr,"Reading TS  %d %%\r",perc);
			last_perc = perc;
		}

		for( i = 0; i < count; i+= TS_SIZE){
			uint8_t off = 0;

			if ( count - i < TS_SIZE) break;

			pid = get_pid(buf+i+1);
			if (!(buf[3+i]&0x10)) // no payload?
				continue;
			if (pid != pidv){
				continue;
			}

			if ( buf[3+i] & 0x20) {  // adaptation field?
				off = buf[4+i] + 1;
			}

			if ( buf[1+i]&0x40) {
				if (p.plength == MMAX_PLENGTH-6){
					p.plength = p.found-6;
					p.found = 0;
					send_ipack(&p);
					reset_ipack(&p);
				}
			}

			instant_repack(buf+4+off+i, TS_SIZE-4-off, &p);
		}
		i = 0;

	}
}


void change_aspect(int fdin, int fdout, int aspect)
{
	ps_packet ps;
	pes_packet pes;
	int neof,i;

	do {
		init_ps(&ps);
		neof = read_ps(fdin,&ps);
		write_ps(fdout,&ps);
		for (i = 0; i < ps.npes; i++){
			u8 *buf;
			int c = 0;
			int l;

			init_pes(&pes);
			read_pes(fdin, &pes);

			buf = pes.pes_pckt_data;

			switch (pes.stream_id){
			case VIDEO_STREAM_S ... VIDEO_STREAM_E:
				l=pes.length;
				break;
			default:
				l = 0;
				break;
			}
			while ( c < l - 6){
				if (buf[c] == 0x00 && 
				    buf[c+1] == 0x00 &&
				    buf[c+2] == 0x01 && 
				    buf[c+3] == 0xB3) {
					c += 4;
					buf[c+3] &= 0x0f;
					buf[c+3] |= aspect;
				}
				else c++;
			}
			write_pes(fdout,&pes);
		}
	} while( neof > 0 );
}
