package StHelpers;

use strict;
use warnings;
use Test::More;
require Exporter;

our @ISA = qw(Exporter);
our @EXPORT = qw(octave_ok mathematica_ok);

sub octave_ok {
	my ($message, $code) = @_;
	diag $code;
	SKIP: {
		# Start Octave process
		open my $proc, '|octave-cli' or skip($message, 1);
		# Forward code
		print $proc $code;
		# Wait for exit and build test
		ok(close($proc), $message);
	}
}

sub mathematica_ok {
	my ($message, $code) = @_;
	diag $code;
	SKIP: {
		# Start Mathematica process
		open my $proc, '|math' or skip($message, 1);
		# Forward code
		print $proc $code;
		# Wait for exit and build test
		ok(close($proc), $message);
	}
}

1;
