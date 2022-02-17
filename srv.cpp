#include "buffer.hpp"
#include "shm.hpp"

namespace {
	void rs_packet(ring *csring, ring *scring, buf *pool, info_opt opt) {
#ifdef CPU_BIND
		bind_core(1);
#endif

		if(opt.process == MOVE) {
			int32_t num_fin = opt.size_batch;

			for(int i = NUM_PACKET; 0 < i; i -= num_fin) {
				if(unlikely(i < num_fin)) {
					num_fin = i;
				}

#ifdef SKIP_CLT
				csring->pull_avoid(num_fin);
#elif defined(AVOID_SRV)
				csring->move_packet(scring, num_fin);
#else
				csring->move_packet(scring, pool, num_fin);
#endif
			}
		}
	//	else if(opt.process == COPY) {
	//		bool is_stream = (opt.stream == ON) ? true : false;
	//#ifndef AVOID_SRV
	//		packet *parray;
	//		parray = new (std::align_val_t{64}) packet[opt.size_batch];
	//		assert((intptr_t(pool) & 63) == 0);
	//		assert((intptr_t(parray) & 63) == 0);
	//#endif
	//
	//		int_fast32_t num_fin = opt.size_batch;
	//
	//		for(int_fast32_t i = NUM_PACKET; 0 < i; i -= num_fin) {
	//			if(unlikely(i < num_fin)) {
	//				num_fin = i;
	//			}
	//
	//#ifdef AVOID_SRV
	//			csring.pull_avoid(num_fin);
	//			for(volatile int j = 0; j < num_fin; j++) {
	//				;
	//			}
	//			scring.ipush_avoid(SRV, num_fin, is_stream);
	//#elif defined(READ_SRV)
	//			//csring.pull(parray, pool, num_fin);
	//			volatile packet *p;
	//			for(volatile int j = 0; j < num_fin; j++) {
	//				p = &parray[j];
	//			}
	//			scring.ipush_avoid(SRV, num_fin, is_stream);
	//#else
	//			csring.pull(parray, pool, num_fin, is_stream);
	//			for(volatile int j = 0; j < num_fin; j++) {
	//				parray[j].set_verification();
	//			}
	//			scring.ipush(parray, pool, SRV, num_fin, is_stream);
	//#endif
	//		}
	//	}
	}
}

int main(int argc, char* argv[]) {
	// 初期設定
	int bfd = open_shmfile("shm_buf", SIZE_SHM, true);
	buf *pool = (buf*)mmap(NULL, SIZE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, bfd, 0);

	ring *csring = (ring*)(pool + SIZE_POOL);
	*csring = ring();
	desc *csdesc = (desc*)(csring + 1);
	csring->descs = csdesc;
	csring->init_descs();

	ring *scring = (ring*)(csdesc + SIZE_RING);
	*scring = ring();
	desc *scdesc = (desc*)(scring + 1);
	scring->descs = scdesc;
	scring->init_descs();

	memset(pool, 0, sizeof(buf) * SIZE_POOL);

	info_opt opt = get_opt(argc, argv);
	
	volatile bool *flag = (volatile bool*)(scdesc + SIZE_RING);
	*flag = false;
	while(!*flag) {
	}

	// 送受信開始
	rs_packet(csring, scring, pool, opt);

	shm_unlink("shm_buf");

	return 0;
}

