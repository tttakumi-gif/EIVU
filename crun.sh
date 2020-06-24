#!/bin/bash

FILE="flag.fg"

echo run...
for num in {0..10}; do
	./clt.out >> result/log &
	
	touch $FILE

	while [ -e $FILE ]; do
		:
	done
done
