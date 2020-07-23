#!/bin/bash

FILE="flag.fg"

echo run...
for num in {0..10}; do
	while [ ! -e $FILE ]; do
		:
	done

	./srv.out $num

	rm $FILE
done
