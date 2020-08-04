#include "buffer.hpp"
#include "shm.hpp"

void rs_packet(ring&, ring&, packet[], info_opt);

int main(int argc, char* argv[]) {
	// 初期設定
	int bfd = open_shmfile("shm_buf", SIZE_SHM, false);
	ring *csring = (ring*)mmap(NULL, SIZE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, bfd, 0);
	ring *scring = (ring*)(csring + 1);
	packet *pool = (packet*)(scring + 1);

	info_opt opt = get_opt(argc, argv);
	if(-1 < opt.num) {
		std::printf(" %d\n", (int)opt.num);
	}
	else {
		std::puts("");
	}
	
	bool *flag = (bool*)(pool + SIZE_POOL);
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
		int_fast32_t *id;
		id = new int_fast32_t[opt.size_batch];

		csring.set_ringaddr(&scring);
		assert(csring.ring_pair != nullptr);

		int_fast32_t num_fin = opt.size_batch;

		for(int_fast32_t i = NUM_PACKET; 0 < i; i -= num_fin) {
			if(unlikely(i < num_fin)) {
				num_fin = i;
			}

			csring.move_packet(pool, num_fin, id);
		}
	}
	else if(opt.process == COPY) {
		packet *parray;
		parray = new packet[opt.size_batch];

		int_fast32_t num_fin = opt.size_batch;

		for(int_fast32_t i = NUM_PACKET; 0 < i; i -= num_fin) {
			if(unlikely(i < num_fin)) {
				num_fin = i;
			}

			csring.pull(parray, pool, num_fin);
			for(int j = 0; j < num_fin; j++) {
				parray[j].set_verification();
			}
			scring.ipush(parray, pool, SRV, num_fin);
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
