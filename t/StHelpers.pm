package StHelpers;

use strict;
use warnings;
require Test::Simple;
require Exporter;

our @ISA = qw(Exporter);
our @EXPORT = qw(octave_ok);

sub octave_ok {
	my ($message, $code) = @_;
	# Start Octave process
	open my $proc, '|octave-cli';
	# Forward code
	print $proc $code;
	# Wait for exit and build test
	ok(close($proc), $message);
}

1;
