#include <chrono>
#include "buffer.hpp"
#include "shm.hpp"

void send_packet(ring&, packet[NUM_THREAD], uint_fast8_t);
void recv_packet(ring&, packet[NUM_THREAD], uint_fast8_t);
bool check_verification(packet*);

int main() {
	puts("begin");
	std::cout << "size: " << sizeof(ring) * 2 + sizeof(packet) * SIZE_POOL << std::endl;

	int bfd = open_shmfile("shm_buf", SIZE_SHM, true);
	ring *csring = (ring*)mmap(NULL, SIZE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, bfd, 0);
	*csring = ring();
	ring *scring = (ring*)(csring + 1);
	*scring = ring();
	packet *pool = (packet*)(scring + 1);
	memset(pool, 0, sizeof(packet) * SIZE_POOL);
	volatile bool *flag = (volatile bool*)(pool + SIZE_POOL);
	*flag = false;

	set_packet_nums(nums);

#if INFO_CPU == 0
	init_p();
#endif

	while(!*flag) {
	}

	//packet *p = pool;
	int nthread = NUM_THREAD * 2 - 1;
	std::thread threads[nthread];
	for(int i = 0; i < NUM_THREAD; i++) {
		threads[i] = std::thread(send_packet, std::ref(*csring), std::ref(pool), i);
		//threads[i] = std::thread(send_packet, std::ref(*csring), std::ref(p), i);
	}
	for(int i = NUM_THREAD; i < nthread; i++) {
		threads[i] = std::thread(recv_packet, std::ref(*scring), std::ref(pool), i - NUM_THREAD);
		//threads[i] = std::thread(recv_packet, std::ref(*scring), std::ref(p), i - NUM_THREAD);
	}

	std::chrono::system_clock::time_point start, end;
	start = std::chrono::system_clock::now();
	recv_packet(*scring, pool, NUM_THREAD - 1);
	for(int i = 0; i < nthread; i++) {
		threads[i].join();
	}
	end = std::chrono::system_clock::now();

	shm_unlink("shm_buf");

	double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	std::cout << elapsed / 1000 << "sec" << std::endl;

	return 0;
}

void send_packet(ring &csring, packet pool[NUM_THREAD], uint_fast8_t id) {
	int8_t j;
	int8_t idx = SIZE_BATCH;
	int index_end = nums[id + 1];
	int num_fin = index_end - idx;
	packet parray[SIZE_BATCH];

	for(int i = nums[id]; i < index_end; i += SIZE_BATCH) {
		if(unlikely(num_fin < i)) {
			idx = index_end - i;
		}
		for(j = 0; j < idx; j++) {
			parray[j] = packet(j + i);
		}

		csring.ipush(parray, pool, CLT, id);
	}
}

void recv_packet(ring &scring, packet pool[NUM_THREAD], uint_fast8_t id) {
	int8_t j;
	int8_t idx = SIZE_BATCH;
	int index_end = nums[id + 1];
	int num_fin = index_end - idx;
	packet p;
	packet parray[SIZE_BATCH];

	for(int i = nums[id]; i < index_end; i += SIZE_BATCH) {
		if(unlikely(num_fin < i)) {
			idx = index_end - i;
		}

		scring.pull(parray, pool, id);
		for(j = 0; j < idx; j++) {
			if(unlikely(!check_verification(&(parray[j])))) {
				puts("verification error");
				exit(1);
			}

			if(unlikely((parray[j].id & 8388607) == 0)) {
#if INFO_CPU == 0
				printf("%g%\n", getCurrentValue_p());
#endif
				parray[j].print();
			}
		}
	}
}

bool check_verification(packet *p) {
	return likely((p->verification ^ 0xffffffff) == p->id);
}
