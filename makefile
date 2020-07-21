CXXFLAGS = -std=c++11 -pthread -march=native -mtune=native -mfpmath=both -ffast-math -fomit-frame-pointer -O3 -Wall -Wextra -g#-fopt-info-vec-optimized -fopt-info-vec-missed 
#CXXFLAGS = -std=c++11 -pthread -march=native -mtune=native -mfpmath=both -ffast-math -fomit-frame-pointer -Wall -Wextra -g#-fopt-info-vec-optimized -fopt-info-vec-missed 
sCXXFLAGS = -std=c++11 -pthread -O0 -pg
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
