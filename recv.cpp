#include "buffer.hpp"
#include "shm.hpp"

#define PRINT

namespace {
	inline void check_verification(packet* p) {
#ifdef READ_SRV
		if(unlikely(p->packet_id == -1)) {
#else
		if(unlikely(p->packet_id != static_cast<signed>(p->verification ^ 0xffffffff))) {
#endif
			std::printf("verification error\n");
			print(p);
			std::printf("not 0x%x\n", (p->packet_id ^ 0xffffffff));
			exit(1);
		}
	}

	inline void judge_packet(packet* parray[], int32_t num_fin) {
		for(int i = 0; i < num_fin; i++) {
			check_verification(parray[i]);

#ifndef READ_SRV
			if(unlikely((parray[i]->packet_id & 8388607) == 0)) {
#if INFO_CPU == PROC_CLT_R
				std::printf("%g%%\n", getCurrentValue_p());
#elif INFO_CPU == TOTAL_CLT
				std::printf("%g%%\n", getCurrentValue_t());
#endif
#ifdef PRINT
				print(parray[i]);
#endif
			}
#endif
		}
	}	
	
	void recv_packet(ring &scring, buf pool[SIZE_POOL], info_opt opt) {
#ifdef CPU_BIND
		bind_core(2);
#endif

		int32_t num_fin = opt.size_batch;
		bool is_stream = (opt.stream == ON) ? true : false;

		int local_pool_index = 0;
		buf* pool_local = new (std::align_val_t{64}) buf[SIZE_POOL];
		packet** parray = new packet*[opt.size_batch];

		for(int i = NUM_PACKET; 0 < i; i -= num_fin) {
			// 受信パケット数の決定
			if(unlikely(i < num_fin)) {
				num_fin = i;
			}
			
			// パケット受信
#ifdef AVOID_CLT
			scring.pull_avoid(num_fin);
			//scring.pull_avoid(parray, pool, num_fin, is_stream);
#else
			for(int j = 0; j < num_fin; j++) {
#ifdef RANDOM
				parray[j] = get_packet_addr(&pool_local[local_pool_index + (int)ids[j]]);
#else
				parray[j] = get_packet_addr(&pool_local[local_pool_index]);
				if(SIZE_POOL <= ++local_pool_index) {
					local_pool_index = 0;
				}
#endif
			}

			scring.pull(parray, pool, num_fin, is_stream);
#ifdef RANDOM
			local_pool_index += num_fin;
			if(SIZE_POOL <= local_pool_index) {
				local_pool_index = 0;
			}
#endif
			// パケット検証
			judge_packet(parray, num_fin);
#endif
		}

#ifndef AVOID_CLT
		delete(parray);
		delete(pool_local);
#endif
	}


}

int main(int argc, char **argv) {

	// 初期設定
	int bfd = open_shmfile("shm_buf", SIZE_SHM, false);
	buf *pool = (buf*)mmap(NULL, SIZE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, bfd, 0);
	ring *csring = (ring*)(pool + SIZE_POOL);
	desc *csdesc = (desc*)(csring + 1);
	ring *scring = (ring*)(csdesc + SIZE_RING);
	desc *scdesc = (desc*)(scring + 1);

	info_opt opt = get_opt(argc, argv);
//	assert(opt.size_batch < SIZE_POOL);

	bool *flag = (bool*)(scdesc + SIZE_RING);
	*flag = true;

	// 計測開始
	std::chrono::system_clock::time_point start, end;
	start = std::chrono::system_clock::now();

	recv_packet(*scring, pool, opt);

	// 計測終了
	end = std::chrono::system_clock::now();

	double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	double second = elapsed / 1000;
	std::printf("result: %.3fsec (%.3fMpps)\n", second, NUM_PACKET / second / 1000000);

	shm_unlink("shm_buf");

	return 0;
}

