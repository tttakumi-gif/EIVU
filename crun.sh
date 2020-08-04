#!/bin/bash

FILE="flag.fg"
LOG="result/log"

if [ -e $FILE ]; then
	rm $FILE
	echo "removed $FILE"
fi

if [ -e $LOG ]; then
	rm $LOG
	echo "removed $FILE"
fi

echo run...
for num in `seq 1 100`; do
	./clt.out >> result/log &
	#./clt.out &
	
	sleep 0.1
	touch $FILE

	while [ -e $FILE ]; do
		:
	done
done
