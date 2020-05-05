#include <iostream>

#include "buffer.hpp"
#include "shm.hpp"

int main() {
	puts("begin");

	int bfd = open_shmfile("shm_buf", 4096, false);
	ring *mring = (ring*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, bfd, 0);
	*mring = ring();
	packet *buf = (packet*)(mring + 1);

	mring->dinit();
	char text[] = "take01";
	mring->push(packet(1, 2, text));
	buf[0] = packet(1, 2, text);
	std::cout << mring->descs[0].buf->dummy << std::endl;
	mring->descs[0].buf = buf;

	int fd = open_shmfile("shared_memory", sizeof(int), false);
	int *num = (int*)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	*num = 811;

	std::cout << *num << std::endl;


/*	ring mring(SIZE_RING);
	mring.dinit();
	char dummy[DUMMY_SIZE] = "qwert";
	packet p(1, 2, dummy);
	p.set_verification();

	mring.push(p);
	mring.descs[0].buf->print();
	mring.del();*/

	return 0;
}
