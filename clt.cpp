#include "buffer.hpp"
#include "shm.hpp"

#define PRINT

void send_packet(ring&, packet[], info_opt);
void recv_packet(ring&, packet[], info_opt);
void check_verification(packet);
void judge_packet(packet[], int_fast32_t);
void init_resource();

int main(int argc, char **argv) {
	puts("begin");

	{
		constexpr int size = sizeof(ring) * 2 + sizeof(packet) * SIZE_POOL + sizeof(volatile bool);
		std::cout << "size: " << size << std::endl;
		static_assert(size <= SIZE_SHM, "over packet size");
		static_assert(SIZE_BATCH < SIZE_POOL, "need to adjust pool size");
		std::cout << "psize: " << sizeof(packet) << std::endl;
		std::cout << "dsize: " << sizeof(desc) << std::endl;
		std::cout << "rsize: " << sizeof(ring) << std::endl;
	}

	// 初期設定
	int bfd = open_shmfile("shm_buf", SIZE_SHM, true);
	packet *pool = (packet*)mmap(NULL, SIZE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, bfd, 0);
	std::cout << pool << std::endl;
	ring *csring = (ring*)(pool + SIZE_POOL);
	*csring = ring();
	ring *scring = (ring*)(csring + 1);
	*scring = ring();
	for(int i = 0; i < SIZE_POOL; i++) {
		pool[i] = packet();
	}

	info_opt opt = get_opt(argc, argv);

	volatile bool *flag = (volatile bool*)(scring + 1);
	*flag = false;
	init_resource();

	while(!*flag) {
	}

	// 計測開始
	std::chrono::system_clock::time_point start, end;
	start = std::chrono::system_clock::now();

	// 送受信開始
	std::thread thread_send(send_packet, std::ref(*csring), std::ref(pool), opt);
	recv_packet(*scring, pool, opt);

	// 計測終了
	thread_send.join();
	end = std::chrono::system_clock::now();

	shm_unlink("shm_buf");

	double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	std::cout << "result: " << elapsed / 1000 << "sec" << std::endl;

	return 0;
}

void send_packet(ring &csring, packet pool[SIZE_POOL], info_opt opt) {
#ifdef CPU_BIND
	bind_core(5);
#endif

	int_fast32_t i = NUM_PACKET;
	int_fast32_t j;
	int_fast32_t num_fin = opt.size_batch;
	packet *parray;
	parray = new (std::align_val_t{64}) packet[opt.size_batch];

	while(0 < i) {
		// 受信パケット数の決定
		if(unlikely(i < num_fin)) {
			num_fin = i;
		}

		for(j = 0; j < num_fin; j++, i--) {
			parray[j] = packet(i);
		}
		
		// パケット受信
		csring.ipush(parray, pool, CLT, num_fin);
	}
	/*
	while(true) {

		// パケット生成
		for(int i = j; j < NUM_PACKET; j++) {
			int num = j - i;
			parray[num] = packet(j);

			// 指定パケット数に達したら終了
			if(unlikely(NUM_PACKET <= j)) {
				csring.ipush(parray, pool, CLT, num);
				return;
			}
		}

		// パケット送信
		csring.ipush(parray, pool, CLT, num_fin);
	}*/

	delete(parray);
}

void recv_packet(ring &scring, packet pool[SIZE_POOL], info_opt opt) {
#ifdef CPU_BIND
	bind_core(6);
#endif

	int_fast32_t num_fin = opt.size_batch;
	packet *parray;
	parray = new packet[opt.size_batch];

	for(int_fast32_t i = NUM_PACKET; 0 < i; i -= num_fin) {
		// 受信パケット数の決定
		if(unlikely(i < num_fin)) {
			num_fin = i;
		}
		
		// パケット受信
		scring.pull(parray, pool, num_fin);

		// パケット検証
		judge_packet(parray, num_fin);
	}

	delete(parray);
}

inline void check_verification(packet p) {
	if(unlikely(p.id != static_cast<signed>(p.verification ^ 0xffffffff))) {
		std::printf("verification error\n");
		p.print();
		std::printf("not 0x%x\n", (p.id ^ 0xffffffff));
		exit(1);
	}
}

inline void judge_packet(packet parray[], int_fast32_t num_fin) {
	for(int_fast32_t i = 0; i < num_fin; i++) {
		check_verification(parray[i]);

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
