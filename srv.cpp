#include "buffer.hpp"
#include "shm.hpp"

void rs_packet(ring&, ring&, packet[SIZE_POOL]);

int main() {
	puts("begin");

	int bfd = open_shmfile("shm_buf", SIZE_SHM, false);
	ring *csring = (ring*)mmap(NULL, SIZE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, bfd, 0);
	ring *scring = (ring*)(csring + 1);
	packet *pool = (packet*)(scring + 1);
	bool *flag = (bool*)(pool + SIZE_POOL);

#if INFO_CPU == 1
	init_p();
#endif

	*flag = true;
	rs_packet(*csring, *scring, pool);

	shm_unlink("shm_buf");

	return 0;
}

void rs_packet(ring &csring, ring &scring, packet pool[SIZE_POOL]) {
	uint_fast32_t j;
	uint_fast8_t num_fin = SIZE_BATCH;
	packet parray[SIZE_BATCH];

	for(uint_fast32_t i = NUM_PACKET; 0 < i; i-= SIZE_BATCH) {
		// 送受信パケット数の決定
		if(unlikely(i < SIZE_BATCH)) {
			num_fin = i;
		}

		// パケット受信
		csring.pull(parray, pool, num_fin);

		// verificationセット
		for(j = 0; j < num_fin; j++) {
#if INFO_CPU == 1
			if(unlikely((parray[j].id & 8388607) == 0)) {
				printf("%g%\n", getCurrentValue_p());
			}
#endif
			parray[j].set_verification();
		}

		// パケット送信
		scring.ipush(parray, pool, SRV, num_fin);
	}
}
