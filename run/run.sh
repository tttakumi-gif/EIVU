#! /bin/bash

run() {
	sudo /home/takeya/new-b4-assignment/clt.out --process=copy --stream=off &
	sleep 0.1
	sudo /home/takeya/new-b4-assignment/srv.out --process=move
}

run
