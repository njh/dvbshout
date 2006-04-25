#!/usr/bin/perl
#
# Takes a dvbshout configuration file and creates 
# SDP files for the multicast channels/streams listed in it
#

use Data::Dumper;
use Net::SDP;
use strict;
use warnings;

if ($#ARGV != 0) {
	die "Usage: $0 <dvbshout.conf>\n";
}

print "Warning: serveral files will be created in the current directory. ok? ";
my $ok = <STDIN>;
die "Aborted.\n" if ($ok !~ /^y/i);


# Parse the config file
my $configpath = $ARGV[0];
open( CONFIG, $configpath ) or 
die "Failed to open config file ($configpath): $!";

my $section = undef;
my $channels = [];
my %config = ();
my $linenum = 0;
my $channel_count = -1;
while( <CONFIG> ) {
	$linenum++;
	chomp;
	
	next if (/^#/);
	next if (/^(\s*)$/);
	
	# Start of new section
	if (/^\[(\w+)\]/) {
		$section = $1;
		if ($section eq 'channel') {
			$channel_count++;
			foreach( keys %config ) {
				$channels->[ $channel_count ]->{$_} = $config{$_};
			}
		}
		next;
	}
	
	# Seperate the name and value
	if (/^(\w+):(\s?)(.*)$/) {
		my ($name, $value) = ($1, $3);
		if ($section eq 'channel') {
			$channels->[ $channel_count ]->{$name} = $value;
		} else {
			$config{ "${section}_$name" } = $value;
		}
	} else {
		die "Failed to parse line $linenum\n";
	}
	
}

close( CONFIG );



# Output each of the channels
foreach my $channel ( @$channels ) {
	my $sdp = Net::SDP->new();
	$sdp->session_name( $channel->{'name'} );
	$sdp->session_attribute( 'x-qt-text-nam', $channel->{'name'} );
	$sdp->session_attribute( 'x-qt-text-url', $channel->{'url'} );
	$sdp->session_attribute( 'x-qt-text-inf', $channel->{'description'} );
	$sdp->session_attribute( 'x-qt-text-gen', $channel->{'genre'} );
	$sdp->session_info( $channel->{'description'} );
	$sdp->session_uri( $channel->{'url'} );
	my $time = $sdp->new_time_desc();
	my $audio = $sdp->new_media_desc('audio');
	$audio->address( $channel->{'multicast_ip'} );
	$audio->port( $channel->{'multicast_port'} );
	$audio->ttl( $channel->{'multicast_ttl'} );
	
	# 14=MPEG Audio
	$audio->default_format_num( 14 );
	$audio->add_attribute( 'fmtp', '14 layer=2');

	# Write it to disc
	my $filename = clean_filename($channel->{'name'}).'.sdp';
	print "Writing to $filename\n";
	open(SDP, ">$filename") or die "Failed to open $filename: $!";
	print SDP $sdp->generate();
	close(SDP);
	

}



# Make HTML/m3u files
print "Writing to dvb-channels.html\n";
open(HTML, '>dvb-channels.html') or die "Failed to open HTML file: $!";
print "Writing to dvb-channels.m3u\n";
open(M3U, '>dvb-channels.m3u') or die "Failed to open M3U file: $!";
print M3U "#EXTM3U\n";
foreach my $channel ( @$channels ) {
	my $url = 'http://'.$channel->{'server_host'}.':'.$channel->{'server_port'}.$channel->{'mount'};
	my $sdpfile = clean_filename($channel->{'name'}).'.sdp';
	
	print HTML "<tr>\n";
	print HTML "\t<td><a href='$url.m3u'><img src='speaker.png' width='22' height='20' alt='Listen!' /></a></td>\n";
	print HTML "\t<td>".$channel->{'name'}."</td>\n";
	print HTML "\t<td>".$channel->{'pid'}."</td>\n";
	print HTML "\t<td><a href='$sdpfile'>$sdpfile</a></td>\n";
	print HTML "\t<td><a href='".$channel->{'url'}."'>".$channel->{'description'}."</a></td>\n";
	print HTML "</tr>\n\n";
	
	print M3U "$url\n";
}
close(M3U);
close(HTML);


sub clean_filename {
	my ($filename) = @_;
	
	$filename =~ tr/A-Z/a-z/;
	$filename =~ s/\W/_/g;
	
	# Remove underscores from end
	while( $filename =~ /_$/ ) {
		$filename = substr( $filename, 0, -1 );
	}
	
	return $filename;
}


