#!/bin/bash

# Get options
BUILD_DISTROS=( )
DISTRIBUTIONS=( stretch xenial )
USE_SBUILD=1

while getopts ":ld:" opt; do
	case "$opt" in
		d)
			if [[ " ${DISTRIBUTIONS[*]} " == *" $OPTARG "* ]]; then
				BUILD_DISTROS+=( "$OPTARG" )
			else
				echo "Unknown distribution $OPTARG" >&2
			fi
		;;
		l)
			USE_SBUILD=0
		;;
	esac
done

# Get the build directory
LIBDIRECTORY="$(pwd)"
LIBNAME=shadertoy-connector

# Grab the current version number from the rules file
LIBVERSION=$(head -n 1 debian/changelog | awk '{gsub("[()]","",$2); print $2}')

echo "[==== BUILDING v$LIBVERSION ====]" >&2

# We need to be in the parent directory
cd "$(dirname "$BASH_SOURCE")/.."

if ! [ -z "${BUILD_DISTROS[@]}" ]; then
	DISTRIBUTIONS=( "${BUILD_DISTROS[@]}" )
fi

version_suffix () {
	if [ "$1" = "stretch" ]; then
		echo -n "debian9"
	elif [ "$1" = "xenial" ]; then
		echo -n "ubuntu16"
	elif [ "$1" = "trusty" ]; then
		echo -n "ubuntu14"
	else
		exit 1
	fi
}

perform_build () {
	mkdir -p "$TARGETDIR"

	echo "[==== BUILDING $DISTRIBUTION-$ARCH ====]" >&2
	if [ "$USE_SBUILD" -ne "0" ]; then
		(cd $LIBDIRECTORY && sbuild --no-apt-update --no-apt-upgrade \
									--no-apt-clean --resolve-alternatives \
									-d $DISTRIBUTION \
									--arch $ARCH \
									--append-to-version "-$(version_suffix $DISTRIBUTION)"
		)
		RESULT="$?"
	else
		(cd $LIBDIRECTORY && debuild -jauto)
		RESULT="$?"
	fi

	if [ "$RESULT" -ne "0" ]; then
		echo "[==== BUILD FAILED FOR $DISTRIBUTION-$ARCH ====]" >&2
		exit $RESULT
	fi
	echo "[==== MOVING ARTIFACTS $DISTRIBUTION-$ARCH ====]" >&2
	find . -maxdepth 1 -type f -exec mv {} $TARGETDIR/ \;
	find . -maxdepth 1 -type l -exec mv {} $TARGETDIR/ \;
}

if [ "$USE_SBUILD" -ne "0" ]; then
	for DISTRIBUTION in "${DISTRIBUTIONS[@]}"; do
		TARGETDIR=${LIBNAME}-$LIBVERSION-$(version_suffix $DISTRIBUTION)-$DISTRIBUTION

		for ARCH in amd64 i386; do
			perform_build
		done
	done
else
	DISTRIBUTION=stretch
	ARCH=amd64
	TARGETDIR=${LIBNAME}-$LIBVERSION-$DISTRIBUTION
	perform_build
fi

echo "[==== DONE ====]" >&2

exit 0
