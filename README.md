dvbshout
========
Nicholas J. Humfrey <njh@aelius.com>


dvbshout takes an MPEG transport stream from a DVB card, 
extracts audio channels from stream, and sends the audio to
an Icecast/Shoutcast server and/or a RTP multicast packet stream.

It has only been tested with DVB-S and DVB-T, but should also 
work with DVB-C.

The HTTP streams can be played back in most MPEG Audio players 
(eg iTunes, WinAmp etc). The multicast streams have been tested 
with QuickTime and VLC, however notably, they don't seem to playback 
correctly in Real Player.

For the latest version of dvbshout, please see:
<http://www.aelius.com/njh/dvbshout/>

It is released under the GPL licence, see COPYING for details.



Configuration
=============

dvbshout is configured using a single configuration file. 
A script called 'scan2dvbshout.pl' is provided to convert 
a channels.conf file (as created by the 'scan' DVB utility) to a 
skeleton dvbshout.conf. An example configuration file is also included.

The [server] section of the configuration file contains the Icecast/Shoutcast 
server settings. The [tuning] section contains the settings for tuning in the
DVB device. There are then multiple [channel] sections for each of the channels 
you wish to send to the server. 

The 'dvbshout' binary takes a single parameter, the location of its 
configuration file.


TODO
====

More error checking of incoming streams. 
Quiet/verbose command line switches.
Link the multicast packet timestamps to PES packet timestamps
 - and hence don't require a reset when there is a DVB transport error

