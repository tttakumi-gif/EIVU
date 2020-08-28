CXXFLAGS = -std=c++17 -pthread -march=native -mtune=native -Wall -Wextra -ffast-math -mfpmath=both -O3 -g#-fopt-info-vec-optimized -fopt-info-vec-missed  -fomit-frame-pointer 
#SCXXFLAGS = -std=c++17 -pthread -march=native -Wall -Wextra -pg -O0#-fopt-info-vec-optimized -fopt-info-vec-missed 
RING_OBJS = obj/shm.o #obj/option.o#obj/packet.o obj/buffer.o

obj/%.o: %.cpp
	g++ -o $@ -c $(CXXFLAGS) $<
clt: clt.cpp $(RING_OBJS)
	g++ -o clt.out clt.cpp $(RING_OBJS) -lrt $(CXXFLAGS)
srv: srv.cpp $(RING_OBJS)
	g++ -o srv.out srv.cpp $(RING_OBJS) -lrt $(CXXFLAGS)
all: clt srv
clean:
	$(RM) obj/* *.out
