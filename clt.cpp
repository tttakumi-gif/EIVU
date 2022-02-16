#include "buffer.hpp"
#include "shm.hpp"

namespace {
	void send_packet(ring &csring, buf *pool, info_opt opt) {
#ifdef CPU_BIND
		bind_core(0);
#endif
		assert((intptr_t(pool) & 63) == 0);

		int32_t i = NUM_PACKET;
		int32_t num_fin = opt.size_batch;
		bool is_stream = (opt.stream == ON) ? true : false;

#ifndef ZERO_COPY
		int local_pool_index = 0;
		buf* pool_local = new (std::align_val_t{64}) buf[SIZE_POOL];
		packet** parray = new packet*[opt.size_batch];
#endif

		while(0 < i) {
			// 受信パケット数の決定
			if(unlikely(i < num_fin)) {
				num_fin = i;
			}

#ifndef ZERO_COPY
			for(int j = 0; j < num_fin; j++, i--) {
#ifdef RANDOM
				parray[j] = get_packet_addr(&pool_local[local_pool_index + (int)ids[j]]);
#else
				parray[j] = get_packet_addr(&pool_local[local_pool_index]);
				if(SIZE_POOL <= ++local_pool_index) {
					local_pool_index = 0;
				}
#endif
				parray[j]->packet_id = i;
				parray[j]->packet_len = SIZE_PACKET;
			}
			csring.ipush(parray, pool, num_fin, is_stream);
#ifdef RANDOM
			local_pool_index += num_fin;
			if(SIZE_POOL <= local_pool_index) {
				local_pool_index = 0;
			}
#endif

#else
			csring.zero_push(pool, num_fin, is_stream);
			i -= num_fin;
#endif
		}

#ifndef ZERO_COPY
		delete(parray);
		delete(pool_local);
#endif
	}

	void init_resource() {
#if INFO_CPU == PROC_CLT_S || INFO_CPU == PROC_CLT_R || INFO_CPU == PROC_SRV
		init_p();
#elif INFO_CPU == TOTAL_CLT || INFO_CPU == TOTAL_SRV
		init_t();
#endif
	}
}

int main(int argc, char **argv) {
	puts("begin");

	{
		constexpr int size = sizeof(ring) * 2 + sizeof(buf) * SIZE_POOL + sizeof(volatile bool);
		std::cout << "size: " << size << std::endl;
		static_assert(size <= SIZE_SHM, "over packet size");
		std::cout << "packet size: " << sizeof(packet) << ", " << SIZE_PACKET << std::endl;
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

