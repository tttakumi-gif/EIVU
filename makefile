CXXFLAGS = -std=c++11 -pthread -march=native -mtune=native -O3 -funroll-loops -mfpmath=both -ffast-math -Wall -Wextra #-fopt-info-vec-optimized
#CXXFLAGS = -std=c++11 -pthread -m32 -O0 -pg
RING_OBJS = obj/shm.o #obj/packet.o obj/buffer.o

obj/%.o: %.cpp
	g++ -o $@ -c $(CXXFLAGS) $<
clt: clt.cpp $(RING_OBJS)
	g++ -o clt.out clt.cpp $(RING_OBJS) -lrt $(CXXFLAGS)
srv: srv.cpp $(RING_OBJS)
	g++ -o srv.out srv.cpp $(RING_OBJS) -lrt $(CXXFLAGS)
all: clt srv
clean:
	$(RM) obj/* *.out
