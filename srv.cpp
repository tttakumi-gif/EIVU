#include "buffer.hpp"
#include "shm.hpp"

int main() {
	puts("begin");

	int bfd = open_shmfile("shm_buf", 4096, false);
	ring *csring = (ring*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, bfd, 0);
	ring *scring = (ring*)(csring + 1);
	packet *pool = (packet*)(scring + 1);
	bool *flag = (bool*)(pool + 2 * SIZE_POOL);

	*flag = true;

	for(int i = 0; i < NUM_PACKET;) {
		packet p = csring->pull(pool);
		if(0 < p.len) {
			p.set_verification();
			if(p.id % 500000 == 0)
				p.print();
			while(!scring->dinit()) {
				;
			}
			while(true) {
				if(scring->push(p, pool, 1)) {
					break;
				}
			}
			i++;
		}
#if 0
		//else {
			//printf("1");
			std::cout << "p: " << csring->proc_idx << ", recv: " << csring->recv_idx << ", rsrv: " << csring->rsrv_idx << std::endl;
			for(desc d : csring->descs) {
				printf("%2d, ", d.len);
				//std::cout << d.len << ", ";
			}
			std::cout << std::endl;
		//}
#endif
	}

	shm_unlink("shm_buf");

	return 0;
}
