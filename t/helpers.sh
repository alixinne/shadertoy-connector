if [ -z "$1" ]; then
	echo "This is file is not to be executed directly."
	exit 1
fi

set -u

export TEST_NAME="$(basename $1 .sh)"
export TEST_COUNTER=1

# Test functions
ok () {
    RESULT="$1"
    ID=$((TEST_COUNTER))
    MESSAGE="$2"

    if [ "$RESULT" -ne 0 ]; then
        echo "not ok $ID - $MESSAGE (result $RESULT)"
    else
        echo "ok $ID - $MESSAGE"
    fi

    export TEST_COUNTER=$((TEST_COUNTER + 1))
}

test_message () {
	echo "$((TEST_COUNTER)) - $1"
}

test_done () {
    echo "1..$((TEST_COUNTER - 1))"
}

octave_test () {
	TEST_MESSAGE="$(test_message "$1")"
	export TEST_COUNTER=$((TEST_COUNTER + 1))
	sed "s#TEST_MESSAGE#${TEST_MESSAGE}#" | octave-cli
}
