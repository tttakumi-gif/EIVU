CXXFLAGS = -std=c++11 -pthread -O3 -mtune=native -march=native -mfpmath=both -m32

packet.o: packet.cpp packet.hpp
	g++ -o obj/packet.o -c packet.cpp $(CXXFLAGS)
buffer.o: buffer.cpp buffer.hpp packet.o
	g++ -o obj/buffer.o -c buffer.cpp $(CXXFLAGS)
shm.o: shm.cpp shm.hpp
	g++ -o obj/shm.o -c shm.cpp -lrt $(CXXFLAGS)
clt: buffer.o shm.o clt.cpp
	g++ -o clt.out clt.cpp obj/packet.o obj/buffer.o obj/shm.o -lrt $(CXXFLAGS)
srv: buffer.o shm.o srv.cpp
	g++ -o srv.out srv.cpp obj/packet.o obj/buffer.o obj/shm.o -lrt $(CXXFLAGS)
all: clt srv
test: main
	./clt.out
