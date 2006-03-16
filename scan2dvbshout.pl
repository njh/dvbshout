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
print OUTPUT "pass: hackme\n";
print OUTPUT "protocol: http\n\n";


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
				print OUTPUT "modulation: 64\n";
				print OUTPUT "guard_interval: 64\n";
				print OUTPUT "code_rate: 2_3\n";
				print OUTPUT "bandwidth: 8\n";
				print OUTPUT "transmission_mode: 2\n";

				print OUTPUT "\n\n";
				$wrote_tuning=1;
			}
			
			print OUTPUT "[channel]\n";
			print OUTPUT "name: $name\n";
			print OUTPUT "mount_point: $mount\n";
			print OUTPUT "audio_pid: $apid\n";
			print OUTPUT "genre: Varied\n";
			print OUTPUT "public:\n";
			print OUTPUT "url:\n";
			print OUTPUT "description:\n";
			print OUTPUT "\n";
			
			$wrote++;
		}
		
	}
	
}


close(OUTPUT);
close(CHANNELS);


print "Wrote $wrote streams to $output\n";




