/* dvbtune - tune.c

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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <unistd.h>
#include <error.h>
#include <errno.h>

#include <linux/dvb/dmx.h>
#include <linux/dvb/frontend.h>

#include "dvbshout.h"



static void
print_status (FILE * fd, fe_status_t festatus)
{
  fprintf (fd, "Frontend Status:");
  if (festatus & FE_HAS_SIGNAL)
    fprintf (fd, " FE_HAS_SIGNAL");
  if (festatus & FE_TIMEDOUT)
    fprintf (fd, " FE_TIMEDOUT");
  if (festatus & FE_HAS_LOCK)
    fprintf (fd, " FE_HAS_LOCK");
  if (festatus & FE_HAS_CARRIER)
    fprintf (fd, " FE_HAS_CARRIER");
  if (festatus & FE_HAS_VITERBI)
    fprintf (fd, " FE_HAS_VITERBI");
  if (festatus & FE_HAS_SYNC)
    fprintf (fd, " FE_HAS_SYNC");
  fprintf (fd, "\n");
}

struct diseqc_cmd
{
  struct dvb_diseqc_master_cmd cmd;
  uint32_t wait;
};

static void
diseqc_send_msg (int fd, fe_sec_voltage_t v, struct diseqc_cmd *cmd,
		 fe_sec_tone_mode_t t, fe_sec_mini_cmd_t b)
{
  ioctl (fd, FE_SET_TONE, SEC_TONE_OFF);
  ioctl (fd, FE_SET_VOLTAGE, v);
  usleep (15 * 1000);
  ioctl (fd, FE_DISEQC_SEND_MASTER_CMD, &cmd->cmd);
  usleep (cmd->wait * 1000);
  usleep (15 * 1000);
  ioctl (fd, FE_DISEQC_SEND_BURST, b);
  usleep (15 * 1000);
  ioctl (fd, FE_SET_TONE, t);
}




/* digital satellite equipment control,
 * specification is available from http://www.eutelsat.com/ 
 */
static int
do_diseqc (int secfd, int sat_no, int pol, int hi_lo)
{
  struct diseqc_cmd cmd = { {{0xe0, 0x10, 0x38, 0xf0, 0x00, 0x00}, 4}, 0 };

  /* param: high nibble: reset bits, low nibble set bits,
   * bits are: option, position, polarizaion, band
   */
  cmd.cmd.msg[3] =
    0xf0 | (((sat_no * 4) & 0x0f) | (hi_lo ? 1 : 0) | (pol ? 0 : 2));

  diseqc_send_msg (secfd, pol,
		   &cmd, hi_lo, (sat_no / 4) % 2 ? SEC_MINI_B : SEC_MINI_A);

  return 1;
}


static int
check_status (int fd_frontend, struct dvb_frontend_parameters *feparams,
	      int tone)
{
  int32_t strength;
  fe_status_t festatus;
  struct dvb_frontend_info fe_info;
  struct dvb_frontend_event event;
  struct pollfd pfd[1];
  int status;

  if (ioctl (fd_frontend, FE_SET_FRONTEND, feparams) < 0)
    {
      perror ("ERROR tuning channel\n");
      return -1;
    }

  pfd[0].fd = fd_frontend;
  pfd[0].events = POLLIN;

  if ((status = ioctl (fd_frontend, FE_GET_INFO, &fe_info) < 0))
    {
      perror ("FE_GET_INFO: ");
      return -1;
    }

  event.status = 0;
  while (((event.status & FE_TIMEDOUT) == 0)
	 && ((event.status & FE_HAS_LOCK) == 0))
    {
    //fprintf (stderr, "Polling....\n");
    if (poll (pfd, 1, 10000))
	{
	  if (pfd[0].revents & POLLIN)
	    {
	      //fprintf (stderr, "Getting frontend event\n");
	      if ((status = ioctl (fd_frontend, FE_GET_EVENT, &event)) < 0)
		{
		  if (errno != EOVERFLOW)
		    {
		      perror ("FE_GET_EVENT");
		      fprintf (stderr, "status = %d\n", status);
		      fprintf (stderr, "errno = %d\n", errno);
		      return -1;
		    }
		  else
		    fprintf (stderr,
			     "Overflow error, trying again (status = %d, errno = %d)",
			     status, errno);
		}
	    }
	  print_status (stderr, event.status);
	}
    }


  if (event.status & FE_HAS_LOCK)
    {
	
	fprintf (stderr, "Gained lock:\n");

      switch (fe_info.type)
	{
	case FE_OFDM:
	  fprintf (stderr, "  Frontend Type: OFDM\n");
	  fprintf (stderr, "  Frequency: %d\n", event.parameters.frequency);
	  break;
	case FE_QPSK:
	  fprintf (stderr, "  Frontend Type: QPSK\n");
	  fprintf (stderr, "  Frequency: %d\n",
		   (unsigned int) ((event.parameters.frequency) +
				   (tone == SEC_TONE_OFF ? LOF1 : LOF2)));
	  fprintf (stderr, "  SymbolRate: %d\n", event.parameters.u.qpsk.symbol_rate);
	  fprintf (stderr, "  FEC Inner: %d\n", event.parameters.u.qpsk.fec_inner);
	  break;
	case FE_QAM:
	  fprintf (stderr, "  Frontend Type: QAM\n");
	  fprintf (stderr, "  Frequency: %d\n", event.parameters.frequency);
	  fprintf (stderr, "  SymbolRate: %d\n", event.parameters.u.qam.symbol_rate);
	  fprintf (stderr, "  FEC Inner: %d\n", event.parameters.u.qam.fec_inner);
	  break;
	default:
	  break;
	}

      strength = 0;
      ioctl (fd_frontend, FE_READ_BER, &strength);
      fprintf (stderr, "  Bit error rate: %d\n", strength);

      strength = 0;
      ioctl (fd_frontend, FE_READ_SIGNAL_STRENGTH, &strength);
      fprintf (stderr, "  Signal strength: %d\n", strength);

      strength = 0;
      ioctl (fd_frontend, FE_READ_SNR, &strength);
      fprintf (stderr, "  SNR: %d\n", strength);

      festatus = 0;
      ioctl (fd_frontend, FE_READ_STATUS, &festatus);
      print_status (stderr, festatus);
      
    } else {
		fprintf(stderr, "Not able to lock to the signal on the given frequency\n");
		return -1;
	}
    
    fprintf(stderr, "\n");
    
	return 0;
}



int
tune_it (int fd_frontend, fe_settings_t * set)
{
	int res;
	struct dvb_frontend_parameters feparams;
	struct dvb_frontend_info fe_info;
	fe_sec_voltage_t voltage;

  if ((res = ioctl (fd_frontend, FE_GET_INFO, &fe_info) < 0))
    {
      perror ("FE_GET_INFO: ");
      return -1;
    }


  fprintf (stderr, "DVB card name: \"%s\"\n", fe_info.name);

  switch (fe_info.type)
    {
    case FE_OFDM:
      feparams.frequency = set->freq;
      feparams.inversion = INVERSION_OFF;
      feparams.u.ofdm.bandwidth = set->bandwidth;
      feparams.u.ofdm.code_rate_HP = set->code_rate;
      feparams.u.ofdm.code_rate_LP = LP_CODERATE_DEFAULT;
      feparams.u.ofdm.constellation = set->modulation;
      feparams.u.ofdm.transmission_mode = set->transmission_mode;
      feparams.u.ofdm.guard_interval = set->guard_interval;
      feparams.u.ofdm.hierarchy_information = HIERARCHY_DEFAULT;
      fprintf (stderr, "Tuning DVB-T to %d\n", set->freq);
      break;
    case FE_QPSK:
      fprintf (stderr, "Tuning DVB-S to %d, Pol:%c Srate=%d, 22kHz=%s\n",
	       set->freq, set->polarity, set->srate,
	       set->tone == SEC_TONE_ON ? "on" : "off");
      if ((set->polarity == 'h') || (set->polarity == 'H'))
	{
	  voltage = SEC_VOLTAGE_18;
	}
      else
	{
	  voltage = SEC_VOLTAGE_13;
	}
      if (set->diseqc == 0)
	if (ioctl (fd_frontend, FE_SET_VOLTAGE, voltage) < 0)
	  {
	    perror ("ERROR setting voltage\n");
	  }

      if (set->freq > 2200000)
	{
	  // this must be an absolute frequency
	  if (set->freq < SLOF) {
	      feparams.frequency = (set->freq - LOF1);
	      if (set->tone < 0) set->tone = SEC_TONE_OFF;
	    } else {
	      feparams.frequency = (set->freq - LOF2);
	      if (set->tone < 0) set->tone = SEC_TONE_ON;
	    }
	} else {
	  // this is an L-Band frequency
	  feparams.frequency = set->freq;
	}

      feparams.inversion = set->spec_inv;
      feparams.u.qpsk.symbol_rate = set->srate;
      feparams.u.qpsk.fec_inner = FEC_AUTO;

      if (set->diseqc == 0)
	{
	  if (ioctl (fd_frontend, FE_SET_TONE, set->tone) < 0)
	    {
	      perror ("ERROR setting tone\n");
	    }
	}

      if (set->diseqc > 0)
	{
	  do_diseqc (fd_frontend, set->diseqc - 1, voltage, set->tone);
	  sleep (1);
	}
      break;
    case FE_QAM:
      fprintf (stderr, "Tuning DVB-C to %d, srate=%d\n", set->freq, set->srate);
      feparams.frequency = set->freq;
      feparams.inversion = INVERSION_OFF;
      feparams.u.qam.symbol_rate = set->srate;
      feparams.u.qam.fec_inner = FEC_AUTO;
      feparams.u.qam.modulation = set->modulation;
      break;
    default:
      fprintf (stderr, "Unknown frontend type. Aborting.\n");
      exit (-1);
    }
  usleep (100000);

  return (check_status (fd_frontend, &feparams, set->tone));
}

