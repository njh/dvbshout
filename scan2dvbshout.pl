#!/usr/bin/perl
#
# Interactive perl script to convert 'channels.conf' file 
# created by 'scan' to a dvbshout configuration file.
#
# There maybe be some details you need to edit 
# in the resulting dvbshout.conf
#
# scan -x 0 -o zap /usr/share/dvb/dvb-s/Astra-28.2E > ~/.szap/channels.conf
# 

use strict;



# Card number ?
print "Card number? [0]: ";
my $cardnum = <STDIN>+0;

# Card type ?
my $cardtype = '';
while ($cardtype !~ /^[stc]$/) {
	print "Card type? [S/c/t]: ";
	$cardtype = <STDIN>; chomp($cardtype);
	$cardtype =~ tr/A-Z/a-z/;
	$cardtype = 's' if ($cardtype eq '');
}


# Guess location of scan file
my $default = $ENV{'HOME'}.'/.'.$cardtype.'zap/channels';
$default .= "-$cardnum" if ($cardnum != 0);
$default .= ".conf";


# Ask for location of channels.conf
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

# Ask for polarity for DVB-S
my $p = undef;
if ($cardtype eq 's') {
	print "Polarity (H/V): ";
	$p = <STDIN>;
	if ($p =~ /^h/i) { $p='h'; }
	else {$p = 'v';}
}


print OUTPUT "# dvbshout configuration file\n";
print OUTPUT "# created ".localtime()."\n";
print OUTPUT "#\n\n";

print OUTPUT "[server]\n";
print OUTPUT "host: localhost\n";
print OUTPUT "port: 8000\n";
print OUTPUT "user: source\n";
print OUTPUT "password: hackme\n";
print OUTPUT "protocol: icecast2\n\n";

print OUTPUT "[rtp]\n";
print OUTPUT "port: 5004\n";
print OUTPUT "mtu: 1450\n";
print OUTPUT "multicast_ttl: 5\n";
print OUTPUT "multicast_loopback: 0\n\n";

my $wrote = 0;
if ($cardtype eq 's') {
	$wrote = process_dvb_s();
} elsif ($cardtype eq 'c') {
	$wrote = process_dvb_c();
} elsif ($cardtype eq 't') {
	$wrote = process_dvb_t();
}
	

close(OUTPUT);
close(CHANNELS);

print "Wrote $wrote channels to $output\n";






sub process_dvb_s {
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
				unless ($wrote_tuning) {
				
					print OUTPUT "[tuning]\n";
					print OUTPUT "card: $cardnum\n";
					print OUTPUT "type: DVB-S\n";
					print OUTPUT "frequency: $f\n";
					print OUTPUT "polarity: $p\n";
					print OUTPUT "symbol_rate: $srate\n\n";
					$wrote_tuning=1;
				}
			
				print_channel( $name, $apid );

				$wrote++;
			}
		
		}
	
	}

	return $wrote;
}


sub process_dvb_c {
	my $wrote_tuning = 0;
	my $num = 0;
	my $wrote = 0;
	while(<CHANNELS>) {
		my ($name, $freq, $inversion, $srate, $fec_inner, $modulation, $vpid, $apid, $sid) = split(/:/);
		$num++;
		
		if ($freq == $f and $apid) {
			print "Include '$name' (Y/n)? ";
			my $yesno = <STDIN>;
			unless ($yesno =~ /^n/i) {
				unless ($wrote_tuning) {
				
					# Clean up parameters
					$inversion =~ s/INVERSION_//g;
					$inversion =~ tr/A-Z/a-z/;
					$fec_inner =~ s/FEC_//;
					$modulation =~ s/QAM_//;
				
					print OUTPUT "[tuning]\n";
					print OUTPUT "card: $cardnum\n";
					print OUTPUT "type: DVB-C\n";
					print OUTPUT "frequency: $f\n";
					print OUTPUT "inversion: $inversion\n";
					print OUTPUT "symbol_rate: $srate\n";
					print OUTPUT "fec_inner: $fec_inner\n";
					print OUTPUT "modulation: $modulation\n\n";
					$wrote_tuning=1;
				}
			
				print_channel( $name, $apid );

				$wrote++;
			}
		
		}
	
	}

	return $wrote;
}


sub process_dvb_t {
	my $wrote_tuning = 0;
	my $num = 0;
	my $wrote = 0;
	while(<CHANNELS>) {
		my ($name, $freq, $inversion, $bandwidth, $code_rate_hp, $code_rate_lp, $modulation, 
		    $transmission_mode, $guard_interval, $hierarchy, $vpid, $apid, $sid) = split(/:/);
		$num++;
		
		if ($freq == $f and $apid) {
			print "Include '$name' (Y/n)? ";
			my $yesno = <STDIN>;
			unless ($yesno =~ /^n/i) {
				
				unless ($wrote_tuning) {
				
					# Clean up parameters
					$bandwidth =~ s/\D//g;
					$code_rate_hp =~ s/FEC_//;
					$code_rate_lp =~ s/FEC_//;
					$modulation =~ s/QAM_//;
					$inversion =~ s/INVERSION_//;
					$inversion =~ tr/A-Z/a-z/;
					$hierarchy =~ s/HIERARCHY_//;
					$hierarchy =~ tr/A-Z/a-z/;
					$guard_interval =~ s/GUARD_INTERVAL_//;
				
					print OUTPUT "[tuning]\n";
					print OUTPUT "card: $cardnum\n";
					print OUTPUT "type: DVB-T\n";
					print OUTPUT "frequency: $f\n";
					print OUTPUT "inversion: $inversion\n";
					print OUTPUT "bandwidth: $bandwidth\n";
					print OUTPUT "code_rate_hp: $code_rate_hp\n";
					print OUTPUT "code_rate_lp: $code_rate_lp\n";
					print OUTPUT "modulation: $modulation\n";
					print OUTPUT "guard_interval: $guard_interval\n";
					print OUTPUT "hierarchy: $hierarchy\n\n";
					$wrote_tuning=1;
				}
			
				print_channel( $name, $apid );
			
				$wrote++;
			}
		
		}
	
	}

	return $wrote;
}


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


sub print_channel {
	my ($name, $pid) = @_;

	my $mount = clean_filename( $name );
	
	# Remove underscores from end
	while( $mount =~ /_$/ ) {
		$mount = substr( $mount, 0, -1 );
	}
	
	print OUTPUT "[channel]\n";
	print OUTPUT "name: $name\n";
	print OUTPUT "mount: /dvb/$mount\n";
	print OUTPUT "pid: $pid\n";
	print OUTPUT "rtp_ip: ".random_multicast_ip()."\n";
	print OUTPUT "genre: Varied\n";
	print OUTPUT "public: 0\n";
	print OUTPUT "url:\n";
	print OUTPUT "description:\n";
	print OUTPUT "\n";
	
}


sub random_multicast_ip {
	# Allocate from the 'Organization-Local Scope' range
	# 239.192.000.000-239.251.255.255 
	my $a = int(rand( 60 ))+192;
	my $b = int(rand( 256 ));
	my $c = int(rand( 256 ));
	
	return "239.$a.$b.$c";
}

