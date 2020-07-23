#include "buffer.hpp"
#include "shm.hpp"

void rs_packet(ring&, ring&, packet[]);

int main(int argc, char* argv[]) {
	std::printf("begin");
	if(1 < argc) {
		std::printf(" %s\n", argv[1]);
	}
	else {
		std::puts("");
	}

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
#ifdef CPU_BIND
	bind_core(7);
#endif

	int id[SIZE_BATCH];
	packet *p[SIZE_BATCH];
	csring.set_ringaddr(&scring);
	assert(csring.ring_pair != nullptr);

	int_fast32_t num_fin = SIZE_BATCH;
	for(int_fast32_t i = NUM_PACKET; 0 < i; i -= SIZE_BATCH) {
		if(unlikely(i < SIZE_BATCH)) {
			num_fin = i;
		}
		csring.move_packet(pool, num_fin, id, p);
	}
}

void init_resource() {
#if INFO_CPU == PROC_SRV
	init_p();
#elif INFO_CPU == TOTAL_SRV
	init_t();
#endif
}
