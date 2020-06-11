CXXFLAGS = -std=c++11 -pthread -mfpmath=both -m32 -march=native -mtune=native -fomit-frame-pointer -O3
RING_OBJS = obj/shm.o obj/buffer.o obj/packet.o

obj/%.o: %.cpp
	g++ -o $@ -c $(CXXFLAGS) $<
clt: clt.cpp $(RING_OBJS)
	g++ -o clt.out clt.cpp obj/packet.o obj/buffer.o obj/shm.o -lrt $(CXXFLAGS)
srv: srv.cpp $(RING_OBJS)
	g++ -o srv.out srv.cpp obj/packet.o obj/buffer.o obj/shm.o -lrt $(CXXFLAGS)
all: clt srv
