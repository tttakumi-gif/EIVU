#include "buffer.hpp"
#include "shm.hpp"

void rs_packet(ring&, ring&, packet[NUM_THREAD]);
void init_d(ring&, uint_fast32_t);

int main() {
	puts("begin");

	int bfd = open_shmfile("shm_buf", SIZE_SHM, false);
	ring *csring = (ring*)mmap(NULL, SIZE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, bfd, 0);
	ring *scring = (ring*)(csring + 1);
	packet *pool = (packet*)(scring + 1);
	bool *flag = (bool*)(pool + SIZE_POOL);

	set_packet_nums(nums);

#if INFO_CPU == 1
	init_p();
#endif

	*flag = true;
	rs_packet(*csring, *scring, pool);

	shm_unlink("shm_buf");

	return 0;
}

void rs_packet(ring &csring, ring &scring, packet pool[NUM_THREAD]) {
	int_fast32_t i;
	int_fast32_t j;
	int_fast32_t num_fin = SIZE_BATCH;
	packet p;
	packet parray[SIZE_BATCH];

	for(i = 0; i < NUM_PACKET; i += SIZE_BATCH) {
		csring.pull(parray, pool, num_fin);
		for(packet& p : parray) {
#if INFO_CPU == 1
			if(unlikely((p.id & 8388607) == 0)) {
				printf("%g%\n", getCurrentValue_p());
			}
#endif
			p.set_verification();
		}

		scring.ipush(parray, pool, SRV, num_fin);
	}

	if(NUM_PACKET < i) {
		i -= SIZE_BATCH;
		num_fin = NUM_PACKET - i;
		csring.pull(parray, pool, num_fin);
		for(j = 0; j < num_fin; j++) {
			parray[j].set_verification();
		}

		scring.ipush(parray, pool, SRV, num_fin);
	}
}
