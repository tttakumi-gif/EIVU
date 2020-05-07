packet.o: packet.cpp packet.hpp
	g++ -o obj/packet.o -c packet.cpp -std=c++11
buffer.o: buffer.cpp buffer.hpp packet.o
	g++ -o obj/buffer.o -c buffer.cpp -std=c++11
shm.o: shm.cpp shm.hpp
	g++ -o obj/shm.o -c shm.cpp -lrt -std=c++11
clt: buffer.o shm.o clt.cpp
	g++ -o clt.out clt.cpp obj/packet.o obj/buffer.o obj/shm.o -lrt -g -std=c++11 -pthread
srv: buffer.o shm.o srv.cpp
	g++ -o srv.out srv.cpp obj/packet.o obj/buffer.o obj/shm.o -lrt -g -std=c++11 -pthread
all: clt srv
test: main
	./clt.out
