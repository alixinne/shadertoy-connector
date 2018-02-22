package StHelpers;

use strict;
use warnings;
use Test::More;
use File::Spec;
use IPC::Open3;
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

sub command_run {
	my ($cmd, $code, $message) = @_;
	# Print the code to be executed
	SKIP: {
		eval {
			# Start process
			my $pid = open3(\*CHLD_IN, \*CHLD_OUT, \*CHLD_OUT,
				$cmd);

			# Send code
			print CHLD_IN $code;

			# Get output
			my $output = do { local $/; <CHLD_OUT> };

			# Wait for exit
			waitpid($pid, 0);
			my $exit_status = $? >> 8;

			# Print code details
			diag "Running the following code in $cmd: ";
			$code =~ s/^/  /gm;
			diag $code;
			diag "Command output: ";
			$output =~ s/^/  /gm;
			diag $output;

			# Print output
			is($exit_status, 0, $message);
		};

		skip("$message (skipped because $@)", 1) if $@;
	}
}

sub octave_ok {
	my ($message, $code) = @_;
	$message = "Octave: $message";

	# Append code to load the current testing version
	if (defined binary_path) {
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

	command_run 'octave-cli', $code, $message;
}

sub mathematica_ok {
	my ($message, $code) = @_;
	$message = "Mathematica: $message";

	# Append exit wrapper
	$code = <<MATHEMATICA_CODE;
On[Assert];
Exit[If[FailureQ[Check[Module[{},
<<Shadertoy`;
$code],\$Failed]],1,0]]
MATHEMATICA_CODE

	# Append code to load the current testing version
	if (defined binary_path) {
		my $path = binary_path;
		$code = <<MATHEMATICA_CODE;
PrependTo[\$Path, "$path"];
$code
MATHEMATICA_CODE
	}

	# Reformat code
	$code =~ s/;(?!\n)/;\n/g;

	command_run 'math', $code, $message;
}

1;
