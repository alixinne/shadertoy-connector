package StHelpers;

use strict;
use warnings;
use Test::More;
use File::Spec;
require Exporter;

our @ISA = qw(Exporter);
our @EXPORT = qw(octave_ok mathematica_ok);

sub binary_path {
	if (@ARGV > 0) {
		# Use first argument as provided by CTest/prove
		return $ARGV[0];
	}
	# else, undefined: use installed version
}

sub octave_ok {
	my ($message, $code) = @_;
	SKIP: {
		if (defined binary_path) {
			# Append code to load the current testing version
			my $path = File::Spec->catfile(binary_path, 'shadertoy_octave.oct');
			$code = <<OCTAVE_CODE;
autoload("shadertoy_octave", "$path");
shadertoy_octave();
$code
OCTAVE_CODE
		} else {
			# Just load the library
			$code = <<OCTAVE_CODE;
shadertoy_octave();
$code
OCTAVE_CODE
		}
		# Print the code to be executed
		diag $code;
		# Start Octave process
		open my $proc, '|octave-cli' or skip("$message (skipped because octave-cli is missing)", 1);
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
