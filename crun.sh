#!/bin/bash

FILE="flag.fg"

echo run...
for num in {0..10}; do
	./clt.out >> result/log &
	
	sleep 0.5
	touch $FILE

	while [ -e $FILE ]; do
		:
	done
done
