#include "buffer.hpp"
#include "shm.hpp"

void rs_packet(ring*, ring*, packet*, int);

int main() {
	puts("begin");

	int bfd = open_shmfile("shm_buf", SIZE_SHM, false);
	ring *csring = (ring*)mmap(NULL, SIZE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, bfd, 0);
	ring *scring = (ring*)(csring + 1);
	packet *pool = (packet*)(scring + 1);
	bool *flag = (bool*)(pool + 2 * SIZE_POOL);

	set_packet_nums(nums);
	*flag = true;
	for(int n : nums) {
		std::cout<<n<<std::endl;
	}

#if 1
	std::thread threads[NUM_THREAD - 1];
	for(int i = 0; i < NUM_THREAD - 1; i++) {
		threads[i] = std::thread(rs_packet, std::ref(csring), std::ref(scring), std::ref(pool), i);
	}

	rs_packet(csring, scring, pool, NUM_THREAD - 1);
	for(int i = 0; i < NUM_THREAD - 1; i++) {
		threads[i].join();
	}
#else
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
		if(NUM_PACKET - 2 < i) {
			std::cout << i << std::endl;
		}
			i++;
		}
	}
	puts("fin");
#endif

	shm_unlink("shm_buf");

	return 0;
}

void rs_packet(ring *csring, ring *scring, packet *pool, int id) {
	int index_begin = nums[id];
	int index_end = nums[id + 1];

	for(int i = index_begin; i < index_end;) {
		packet p = csring->pull(pool, id);
		if(0 < p.len) {
			p.set_verification();
			if(p.id % 500000 == 0)
				p.print();
			while(!scring->dinit(id)) {
				;
			}
			while(true) {
				if(scring->push(p, pool, 1, id)) {
					break;
				}
			}
			if(4999000 < p.id) {
				p.print();
			}
			i++;
		}
	}
}
