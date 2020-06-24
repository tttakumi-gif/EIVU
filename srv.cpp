#include "buffer.hpp"
#include "shm.hpp"

void rs_packet(ring&, ring&, packet[NUM_THREAD], uint_fast8_t);
void init_d(ring&, uint_fast8_t);

int main() {
	puts("begin");

	int bfd = open_shmfile("shm_buf", SIZE_SHM, false);
	ring *csring = (ring*)mmap(NULL, SIZE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, bfd, 0);
	ring *scring = (ring*)(csring + 1);
	packet *pool = (packet*)(scring + 1);
	bool *flag = (bool*)(pool + SIZE_POOL);

	set_packet_nums(nums);

#if 1
	std::thread threads[NUM_THREAD - 1];
	for(int i = 0; i < NUM_THREAD - 1; i++) {
		threads[i] = std::thread(rs_packet, std::ref(*csring), std::ref(*scring), std::ref(pool), i);
	}

#if INFO_CPU == 1
	init_p();
#endif

	*flag = true;
	rs_packet(*csring, *scring, pool, NUM_THREAD - 1);
	for(int i = 0; i < NUM_THREAD - 1; i++) {
		threads[i].join();
	}
#else
	int nthread = NUM_THREAD * 2 - 1;
	std::thread threads[nthread];
	for(int i = 0; i < NUM_THREAD; i++) {
		threads[i] = std::thread(rs_packet, std::ref(*csring), std::ref(*scring), std::ref(pool), i);
	}
	for(int i = NUM_THREAD; i < nthread; i++) {
		threads[i] = std::thread(init_d, std::ref(*scring), i - NUM_THREAD);
	}
	*flag = true;
	init_d(*scring, NUM_THREAD - 1);
	for(int i = 0; i < nthread; i++) {
		threads[i].join();
	}
#endif

	shm_unlink("shm_buf");

	return 0;
}

void rs_packet(ring &csring, ring &scring, packet pool[NUM_THREAD], uint_fast8_t id) {
	int8_t j;
	int8_t idx = SIZE_BATCH;
	int index_end = nums[id + 1];
	int num_fin = index_end - idx;
	packet p;
	packet parray[SIZE_BATCH];

	for(int i = nums[id]; i < index_end; i += SIZE_BATCH) {
		if(num_fin < i) {
			idx = index_end - i;
		}

		csring.pull(parray, pool, id);
		for(j = 0; j < idx; j++) {
#if INFO_CPU == 1
			if(unlikely((parray[j].id & 8388607) == 0)) {
				printf("%g%\n", getCurrentValue_p());
			}
#endif
			parray[j].set_verification();
		}

		scring.ipush(parray, pool, SRV, id);
	}
}
