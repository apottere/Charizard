#!/bin/bash
TESTS="test test2 test3"
OUT="+ "
make clean
make
if [ $? -eq 0 ]
then
	for i in $TESTS
	do
		./Charizard < $i".ps" > out_$i".txt" 2>&1
		OUT=$OUT"out_"$i".txt "
	done
	vim + $OUT
fi
