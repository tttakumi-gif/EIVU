#include <chrono>
#include "buffer.hpp"
#include "shm.hpp"

void send_packet(ring&, packet[]);
void recv_packet(ring&, packet[]);
void check_verification(packet);
void judge_packet(packet[], uint_fast32_t);

int main() {
	puts("begin");
	{
		int size = sizeof(ring) * 2 + sizeof(packet) * SIZE_POOL + sizeof(volatile bool);
		std::cout << "size: " << size << std::endl;
		assert(size <= SIZE_SHM);
		std::cout << "psize: " << sizeof(packet) << std::endl;
		std::cout << "dsize: " << sizeof(desc) << std::endl;
		std::cout << "rsize: " << sizeof(ring) << std::endl;
	}

	// 初期設定
	int bfd = open_shmfile("shm_buf", SIZE_SHM, true);
	ring *csring = (ring*)mmap(NULL, SIZE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, bfd, 0);
	*csring = ring();
	ring *scring = (ring*)(csring + 1);
	*scring = ring();
	packet *pool = (packet*)(scring + 1);
	for(int i = 0; i < SIZE_POOL; i++) {
		pool[i] = packet();
	}

	volatile bool *flag = (volatile bool*)(pool + SIZE_POOL);
	*flag = false;

#if INFO_CPU == 0
	init_p();
#endif

	while(!*flag) {
	}

	// 計測開始
	std::chrono::system_clock::time_point start, end;
	start = std::chrono::system_clock::now();

	// 送受信開始
	std::thread thread_send(send_packet, std::ref(*csring), std::ref(pool));
	recv_packet(*scring, pool);

	// 計測終了
	thread_send.join();
	end = std::chrono::system_clock::now();

	shm_unlink("shm_buf");

	double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	std::cout << elapsed / 1000 << "sec" << std::endl;

	return 0;
}

void send_packet(ring &csring, packet pool[SIZE_POOL]) {
	uint_fast32_t i;
	uint_fast32_t j = 0;
	uint_fast32_t num_fin = SIZE_BATCH;
	packet parray[SIZE_BATCH];

	while(true) {
		i = j;

		// パケット生成
		for(packet& p : parray) {
			p = packet(j);
			j++;

			// 指定パケット数に達したら終了
			if(unlikely(NUM_PACKET <= j)) {
				csring.ipush(parray, pool, CLT, j - i);
				return;
			}
		}

		// パケット送信
		csring.ipush(parray, pool, CLT, num_fin);
	}
}

void recv_packet(ring &scring, packet pool[SIZE_POOL]) {
	uint_fast32_t num_fin = SIZE_BATCH;
	packet parray[SIZE_BATCH];

	for(uint_fast32_t i = NUM_PACKET; 0 < i; i -= SIZE_BATCH) {
		// 受信パケット数の決定
		if(unlikely(i < SIZE_BATCH)) {
			num_fin = i;
		}
		
		// パケット受信
		scring.pull(parray, pool, num_fin);

		// パケット検証
		judge_packet(parray, num_fin);
	}
}

inline void check_verification(packet p) {
	if(unlikely(p.id != (p.verification ^ 0xffffffff))) {
		puts("verification error");
		exit(1);
	}
}

inline void judge_packet(packet parray[SIZE_BATCH], uint_fast32_t num_fin) {
	for(uint_fast32_t i = 0; i < num_fin; i++) {
		check_verification(parray[i]);

		if(unlikely((parray[i].id & 8388607) == 0)) {
#if INFO_CPU == 0
			printf("%g%\n", getCurrentValue_p());
#endif
			parray[i].print();
		}
	}
}
