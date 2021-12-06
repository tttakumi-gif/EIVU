#include "buffer.hpp"
#include "shm.hpp"

//#define PRINT

namespace {
	void send_packet(ring &csring, buf *pool, info_opt opt) {
#ifdef CPU_BIND
		bind_core(5);
#endif
		assert((intptr_t(pool) & 63) == 0);

		int_fast32_t i = NUM_PACKET;
		int_fast32_t num_fin = opt.size_batch;
		bool is_stream = (opt.stream == ON) ? true : false;

#ifndef ZERO_COPY
		packet *parray;
		parray = new (std::align_val_t{64}) packet[opt.size_batch];
		assert((intptr_t(parray) & 63) == 0);
#endif

		while(0 < i) {
			// 受信パケット数の決定
			if(unlikely(i < num_fin)) {
				num_fin = i;
			}

#ifndef ZERO_COPY
			for(int_fast32_t j = 0; j < num_fin; j++, i--) {
				parray[j] = packet(i);
			}
			csring.ipush(parray, pool, num_fin, is_stream);
#else
			csring.zero_push(pool, num_fin, is_stream);
			i -= num_fin;
#endif
		}

#ifndef ZERO_COPY
		delete(parray);
#endif
	}

	inline void check_verification(packet p) {
#ifdef READ_SRV
		if(unlikely(p.id == -1)) {
#else
		if(unlikely(p.id != static_cast<signed>(p.verification ^ 0xffffffff))) {
#endif
			std::printf("verification error\n");
			p.print();
			std::printf("not 0x%x\n", (p.id ^ 0xffffffff));
			exit(1);
		}
	}

	inline void judge_packet(packet parray[], int_fast32_t num_fin) {
		for(volatile int_fast32_t i = 0; i < num_fin; i++) {
			check_verification(parray[i]);

#ifndef READ_SRV
			if(unlikely((parray[i].id & 8388607) == 0)) {
#if INFO_CPU == PROC_CLT_R
				std::printf("%g%%\n", getCurrentValue_p());
#elif INFO_CPU == TOTAL_CLT
				std::printf("%g%%\n", getCurrentValue_t());
#endif
#ifdef PRINT
				parray[i].print();
#endif
			}
#endif
		}
	}

	void init_resource() {
#if INFO_CPU == PROC_CLT_S || INFO_CPU == PROC_CLT_R || INFO_CPU == PROC_SRV
		init_p();
#elif INFO_CPU == TOTAL_CLT || INFO_CPU == TOTAL_SRV
		init_t();
#endif

#ifdef DUMMY_FULL
		set_global_dummy();
#endif
	}
}

int main(int argc, char **argv) {
	puts("begin");

	{
		constexpr int size = sizeof(ring) * 2 + sizeof(buf) * SIZE_POOL + sizeof(volatile bool);
		std::cout << "size: " << size << std::endl;
		static_assert(size <= SIZE_SHM, "over packet size");
		std::cout << "packet size: " << sizeof(packet) << std::endl;
		std::cout << "desc size: " << sizeof(desc) << std::endl;
		std::cout << "ring size: " << sizeof(ring) << std::endl;
	}

	// 初期設定
	int bfd = open_shmfile("shm_buf", SIZE_SHM, false);
	buf *pool = (buf*)mmap(NULL, SIZE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, bfd, 0);
	ring *csring = (ring*)(pool + SIZE_POOL);
	ring *scring = (ring*)(csring + 1);

	info_opt opt = get_opt(argc, argv);
//	assert(opt.size_batch < SIZE_POOL);

	volatile bool *flag = (volatile bool*)(scring + 1);
	*flag = false;
	init_resource();

	while(!*flag) {
	}

	// 送受信開始
	send_packet(*csring, pool, opt);

	shm_unlink("shm_buf");

	return 0;
}

