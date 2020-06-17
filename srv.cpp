#include "buffer.hpp"
#include "shm.hpp"

void rs_packet(ring*, ring*, packet*, uint_fast8_t);

int main() {
	puts("begin");

	int bfd = open_shmfile("shm_buf", SIZE_SHM, false);
	ring *csring = (ring*)mmap(NULL, SIZE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, bfd, 0);
	ring *scring = (ring*)(csring + 1);
	packet *pool = (packet*)(scring + 1);
	bool *flag = (bool*)(pool + SIZE_POOL);

	set_packet_nums(nums);
	*flag = true;

	std::thread threads[NUM_THREAD - 1];
	for(int i = 0; i < NUM_THREAD - 1; i++) {
		threads[i] = std::thread(rs_packet, std::ref(csring), std::ref(scring), std::ref(pool), i);
	}

	rs_packet(csring, scring, pool, NUM_THREAD - 1);
	for(int i = 0; i < NUM_THREAD - 1; i++) {
		threads[i].join();
	}

	shm_unlink("shm_buf");

	return 0;
}

void rs_packet(ring *csring, ring *scring, packet *pool, uint_fast8_t id) {
	int j;
	int idx = SIZE_BATCH;
	int index_begin = nums[id];
	int index_end = nums[id + 1];
	int num_fin = index_end - idx;
	packet p;
	packet parray[SIZE_BATCH];

	for(int i = index_begin; i < index_end; i += SIZE_BATCH) {
		if(num_fin < i) {
			idx = index_end - i;
		}

		for(j = 0; j < idx; j++) {
			p = csring->pull(pool, id);
			p.set_verification();
			parray[j] = p;
		}

		for(j--; 0 <= j; j--) {
			scring->ipush(parray[j], pool, SRV, id);
		}
	}
}
