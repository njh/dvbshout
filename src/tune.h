/* dvbtune - tune.h

   Copyright (C) Dave Chapman 2001,2002
  
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

#ifndef _TUNE_H
#define _TUNE_H

#include <linux/dvb/frontend.h>
#include "dvb_defaults.h"

typedef struct fe_settings {
	unsigned int freq;			// Frequency
	unsigned int srate;			// Symbols per Second
	unsigned char polarity;		// Polarity
	int tone;					// 22kHz tone (-1 = auto)
	
	unsigned int diseqc;
	fe_spectral_inversion_t spec_inv;
	fe_modulation_t modulation;
	fe_code_rate_t code_rate;
	fe_transmit_mode_t transmission_mode;
	fe_guard_interval_t guard_interval;
	fe_bandwidth_t bandwidth;

} fe_settings_t;


int tune_it(int fd_frontend, fe_settings_t *set);


#endif
