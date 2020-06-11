CXXFLAGS = -std=c++11 -pthread -mfpmath=both -m32 -march=native -mtune=native -fomit-frame-pointer -O3
RING_OBJS = obj/shm.o #obj/packet.o obj/buffer.o

obj/%.o: %.cpp
	g++ -o $@ -c $(CXXFLAGS) $<
clt: clt.cpp $(RING_OBJS)
	g++ -o clt.out clt.cpp $(RING_OBJS) -lrt $(CXXFLAGS)
srv: srv.cpp $(RING_OBJS)
	g++ -o srv.out srv.cpp $(RING_OBJS) -lrt $(CXXFLAGS)
all: clt srv
