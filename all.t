#!/bin/bash
#
# Test script for minauto
# Uses known inputs & verifies vs expected outputs
#
tests=0
ok=0
fail=0
tdiff=/tmp/diff.$$

#
# -- Iterate on all inputs test cases
#
for inp in io/inp.*; do
    out="$(echo $inp | sed 's,inp,out,')"

    echo -n === comparing $out:

    ./minauto $inp | diff - $out >$tdiff

    case $? in
	0)  echo " ok"
	    ok=$(($ok+1)) ;;
	*)  echo " FAILED"; cat $tdiff
	    fail=$(($fail+1)) ;;
    esac
    tests=$(($tests+1))
done

echo $ok/$tests succeeded

# -- Cleanup
rm $tdiff

