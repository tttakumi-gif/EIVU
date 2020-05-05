#include <iostream>

#include "buffer.hpp"
#include "shm.hpp"

int main() {
	puts("begin");

	int bfd = open_shmfile("shm_buf", 4096, true);
	ring *mring = (ring*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, bfd, 0);
	packet *buf = (packet*)(mring + 1);

	int fd = open_shmfile("shared_memory", sizeof(int), true);
	int *num = (int*)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	*num = 0;

	while(*num == 0) {
		;
	}
	std::cout << mring->descs << std::endl;
	std::cout << mring->descs[0].id << std::endl;
	std::cout << mring->descs[0].len << std::endl;
	std::cout << buf[0].dummy << std::endl;
	std::cout << mring->descs[0].buf->dummy << std::endl;
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
