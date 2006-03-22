#!/usr/bin/perl

use strict;


my @conf_paths = (
	'.szap/channels.conf',
	'.czap/channels.conf',
	'.tzap/channels.conf'
);


my $default = '';
foreach( @conf_paths ) {
	my $test = $ENV{'HOME'}."/$_";
	if (-e $test) { $default = $test; last; } 
}
	

my $channels = $ARGV[1];
while( $channels eq '') {
	print "Path of channels.conf? [$default]: ";
	$channels = <STDIN>; chomp($channels);
	if ($channels eq '') { $channels = $default; }
	if (!-e $channels) { print "Path does not exist.\n"; }
}

# Open the channels file
open(CHANNELS, $channels) or die "Failed to open channels file: $!";


print "Path of output fule? [dvbshout.conf]: ";
my $output = <STDIN>; chomp($output);
if ($output eq '') { $output = 'dvbshout.conf'; }

#if (-e $output) { die "Error: output file already exists.\n"; }

# Open the output file
open(OUTPUT, ">$output") or die "Failed to open output file: $!";

print "Frequency (MHz): ";
my $f = <STDIN>; chomp($f);

print "Polarity (H/V): ";
my $p = <STDIN>;
if ($p =~ /^h/i) { $p='h'; }
else {$p = 'v';}



print OUTPUT "# dvbshout configuration file\n";
print OUTPUT "# created ".localtime()."\n";
print OUTPUT "#\n\n";

print OUTPUT "[server]\n";
print OUTPUT "host: localhost\n";
print OUTPUT "port: 8000\n";
print OUTPUT "user: source\n";
print OUTPUT "password: hackme\n";
print OUTPUT "protocol: icecast2\n\n";

print OUTPUT "[multicast]\n";
print OUTPUT "ttl: 127\n";
print OUTPUT "port: 5004\n";
print OUTPUT "mtu: 1450\n";
print OUTPUT "interface: eth0\n\n";


my $wrote_tuning = 0;
my $num = 0;
my $wrote = 0;
while(<CHANNELS>) {
	my ($name, $freq, $polarity, $tone, $srate, $vpid, $apid, $sid) = split(/:/);
	$num++;
	
	if ($freq == $f and $polarity =~ /$p/i and $apid) {
		print "Include '$name' (Y/n)? ";
		my $yesno = <STDIN>;
		unless ($yesno =~ /^n/i) {
			my $mount = $name;
			$mount =~ tr/A-Z/a-z/;
			$mount =~ s/\W/_/g;
			
			unless ($wrote_tuning) {
			
				print OUTPUT "[tuning]\n";
				print OUTPUT "card: 0\n";
				print OUTPUT "frequency: $f\n";
				print OUTPUT "polarity: $p\n";
				print OUTPUT "symbol_rate: $srate\n";
				
				# UK DVB-T defaults
				print OUTPUT uk_dvbt();

				print OUTPUT "\n\n";
				$wrote_tuning=1;
			}
			
			print OUTPUT "[channel]\n";
			print OUTPUT "name: $name\n";
			print OUTPUT "mount: /dvb/$mount\n";
			print OUTPUT "audio_pid: $apid\n";
			print OUTPUT "multicast_ip: ".random_multicast_ip()."\n";
			print OUTPUT "genre: Varied\n";
			print OUTPUT "public: 0\n";
			print OUTPUT "url:\n";
			print OUTPUT "description:\n";
			print OUTPUT "\n";
			
			$wrote++;
		}
		
	}
	
}


close(OUTPUT);
close(CHANNELS);


print "Wrote $wrote channels to $output\n";



sub uk_dvbt {
	return	"\n# United Kingdom DVB-T settings\n".
			"modulation: 64\n".
			"guard_interval: 32\n".
			"code_rate: 2_3\n".
			"bandwidth: 8\n".
			"transmission_mode: 2\n";
}


sub random_multicast_ip {
	# Allocate from the 'SAP Dynamic Assignments' range
	# 224.2.128.0-224.2.255.255
	my $a = int(rand( 129 ))+128;
	my $b = int(rand( 256 ));
	
	return "244.2.$a.$b";
}

