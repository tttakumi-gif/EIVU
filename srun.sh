#!/bin/bash

FILE="flag.fg"
LPATH="`pwd`/result/log_perf"

echo run...
for num in `seq 1 100`; do
	echo $num
	while [ ! -e $FILE ]; do
		:
	done

	perf stat -B -e task-clock,context-switches,cpu-migrations,page-faults,cycles,stalled-cycles-frontend,stalled-cycles-backend,instructions,cache-references,cache-misses,branches,branch-misses ./srv.out --num=$num &>> $LPATH

	rm $FILE
done
