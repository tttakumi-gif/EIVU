#include "buffer.hpp"
#include "shm.hpp"

void rs_packet(ring&, ring&, packet[]);

int main() {
	puts("begin");

	// 初期設定
	int bfd = open_shmfile("shm_buf", SIZE_SHM, false);
	ring *csring = (ring*)mmap(NULL, SIZE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, bfd, 0);
	ring *scring = (ring*)(csring + 1);
	packet *pool = (packet*)(scring + 1);
	bool *flag = (bool*)(pool + SIZE_POOL);

	*flag = true;

	// 送受信開始
	rs_packet(*csring, *scring, pool);

	shm_unlink("shm_buf");

	return 0;
}

void rs_packet(ring &csring, ring &scring, packet pool[SIZE_POOL]) {
	bind_core(1);

	int_fast32_t j;
	int_fast32_t num_fin = SIZE_BATCH;
	packet parray[SIZE_BATCH];

	for(int_fast32_t i = NUM_PACKET; 0 < i; i -= SIZE_BATCH) {
		// 送受信パケット数の決定
		if(unlikely(i < SIZE_BATCH)) {
			num_fin = i;
		}

		// パケット受信
		csring.pull(parray, pool, num_fin);

		// verificationセット
		for(j = 0; j < num_fin; j++) {
#if INFO_CPU == PROC_SRV
			if(unlikely((parray[j].id & 8388607) == 0)) {
				std::printf("%g%%\n", getCurrentValue_p());
			}
#elif INFO_CPU == TOTAL_SRV
			if(unlikely((parray[j].id & 8388607) == 0)) {
				std::printf("%g%%\n", getCurrentValue_t());
			}
#endif
			parray[j].set_verification();
		}

		// パケット送信
		scring.ipush(parray, pool, SRV, num_fin);
	}
}

void init_resource() {
#if INFO_CPU == PROC_SRV
	init_p();
#elif INFO_CPU == TOTAL_SRV
	init_t();
#endif
}
