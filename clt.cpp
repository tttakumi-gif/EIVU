#include "buffer.hpp"
#include "shm.hpp"

bool check_verification(packet*);

int main() {
	puts("begin");

	//int bfd = open_shmfile("shm_buf", 4096, false);
	int bfd = open_shmfile("shm_buf", 4096, true);
	ring *csring = (ring*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, bfd, 0);
	*csring = ring();
	ring *scring = (ring*)(csring + 1);
	*scring = ring();
	packet *pool = (packet*)(scring + 1);
	memset(pool, 0, sizeof(packet) * 2 * SIZE_POOL);

	std::string base_text = "take";
	std::string text;

	for(int i = 0; i < NUM_PACKET;) {
		if(csring->dinit()) {
			text = base_text + std::to_string(i);
			while(true) {
				if(csring->push(packet(i, text.c_str()), pool, 0)) {
					break;
				}
			}

			while(true) {
				packet p = scring->pull(pool);
				if(0 < p.len) {
					if(i % 500000 == 0) {
						if(check_verification(&p)) {
							p.print();
						}
						else {
							puts("asdfa");
						}
					}
					break;
				}
			}

			i++;
		}
	}

	/*int fd = open_shmfile("shared_memory", sizeof(int), false);
	int *num = (int*)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	*num = 811;
	shm_unlink("shared_memory");*/

	std::cout << sizeof(ring) << std::endl;
	std::cout << sizeof(packet) << std::endl;

	std::cout << pool << std::endl;
	std::cout << pool + 1 << std::endl;

	shm_unlink("shm_buf");

	return 0;
}

bool check_verification(packet *p) {
	return p->verification ^ 0xffffffff == p->id;
}
