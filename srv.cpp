#include "buffer.hpp"
#include "shm.hpp"

void rs_packet(ring&, ring&, packet[], info_opt);

int main(int argc, char* argv[]) {
	// 初期設定
	int bfd = open_shmfile("shm_buf", SIZE_SHM, false);
	packet *pool = (packet*)mmap(NULL, SIZE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, bfd, 0);
	ring *csring = (ring*)(pool + SIZE_POOL);
	ring *scring = (ring*)(csring + 1);

	info_opt opt = get_opt(argc, argv);
	if(-1 < opt.num) {
		std::printf(" %d\n", (int)opt.num);
	}
	else {
		std::puts("");
	}
	
	bool *flag = (bool*)(scring + 1);
	*flag = true;

	// 送受信開始
	rs_packet(*csring, *scring, pool, opt);

	shm_unlink("shm_buf");

	return 0;
}

void rs_packet(ring &csring, ring &scring, packet pool[SIZE_POOL], info_opt opt) {
#ifdef CPU_BIND
	bind_core(7);
#endif

	if(opt.process == MOVE) {
		csring.set_ringaddr(&scring);
		assert(csring.ring_pair != nullptr);

		int_fast32_t num_fin = opt.size_batch;

		for(int_fast32_t i = NUM_PACKET; 0 < i; i -= num_fin) {
			if(unlikely(i < num_fin)) {
				num_fin = i;
			}

#ifdef SKIP_CLT
			csring.pull_avoid(num_fin);
#elif defined(AVOID_SRV)
			csring.move_packet_avoid(num_fin);
#else
			csring.move_packet(pool, num_fin);
#endif
		}
	}
	else if(opt.process == COPY) {
		bool is_stream = (opt.stream == ON) ? true : false;
#ifndef AVOID_SRV
		packet *parray;
		parray = new (std::align_val_t{64}) packet[opt.size_batch];
		assert((intptr_t(pool) & 63) == 0);
		assert((intptr_t(parray) & 63) == 0);
#endif

		int_fast32_t num_fin = opt.size_batch;

		for(int_fast32_t i = NUM_PACKET; 0 < i; i -= num_fin) {
			if(unlikely(i < num_fin)) {
				num_fin = i;
			}

#ifdef AVOID_SRV
			csring.pull_avoid(num_fin);
			for(volatile int j = 0; j < num_fin; j++) {
				;
			}
			scring.ipush_avoid(SRV, num_fin, is_stream);
#elif defined(READ_SRV)
			csring.pull(parray, pool, num_fin);
			volatile packet *p;
			for(volatile int j = 0; j < num_fin; j++) {
				p = &parray[j];
			}
			scring.ipush_avoid(SRV, num_fin, is_stream);
#else
			csring.pull(parray, pool, num_fin, is_stream);
			for(volatile int j = 0; j < num_fin; j++) {
				parray[j].set_verification();
			}
			scring.ipush(parray, pool, SRV, num_fin, is_stream);
#endif
		}
	}
}

void init_resource() {
#if INFO_CPU == PROC_SRV
	init_p();
#elif INFO_CPU == TOTAL_SRV
	init_t();
#endif
}
