#include <chrono>
#include "buffer.hpp"
#include "shm.hpp"

void send_packet(ring*, packet*, int);
void recv_packet(ring*, packet*, int);
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

	while(!*flag) {
	}

	int nthread = NUM_THREAD * 2 - 1;
	std::thread threads[nthread];
	for(int i = 0; i < NUM_THREAD; i++) {
		threads[i] = std::thread(send_packet, std::ref(csring), std::ref(pool), i);
	}
	for(int i = NUM_THREAD; i < nthread; i++) {
		threads[i] = std::thread(recv_packet, std::ref(scring), std::ref(pool), i - NUM_THREAD);
	}

	std::chrono::system_clock::time_point start, end;
	start = std::chrono::system_clock::now();
	recv_packet(scring, pool, NUM_THREAD - 1);
	for(int i = 0; i < nthread; i++) {
		threads[i].join();
	}
	end = std::chrono::system_clock::now();

	shm_unlink("shm_buf");

	double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	std::cout << elapsed / 1000 << "sec" << std::endl;

	return 0;
}

void send_packet(ring *csring, packet *pool, int id) {
	int j, idx;
	int index_begin = nums[id];
	int index_end = nums[id + 1];
	packet parray[SIZE_BATCH];

	for(int i = index_begin; i < index_end; i += SIZE_BATCH) {
		idx = i + SIZE_BATCH;
		if(unlikely(index_end < idx)) {
			idx = index_end;
		}
		for(j = i; j < idx; j++) {
			parray[j - i] = packet(j);
		}

		idx -= i;
		for(j = 0; j < idx; j++) {
			csring->ipush(parray[j], pool, CLT, id);
		}
	}
}

void recv_packet(ring *scring, packet *pool, int id) {
	int j, idx;
	int index_begin = nums[id];
	int index_end = nums[id + 1];
	packet p;
	packet parray[SIZE_BATCH];

	for(int i = index_begin; i < index_end; i += SIZE_BATCH) {
		idx = i + SIZE_BATCH;
		if(unlikely(index_end < idx)) {
			idx = index_end;
		}
		for(j = i; j < idx; j++) {
			p = scring->pull(pool, id);

			if(unlikely(!check_verification(&p))) {
				puts("verification error");
				exit(1);
			}
			parray[j - i] = p;
		}

		idx -= i;
		for(j = 0; j < idx; j++) {
			if(unlikely((parray[j].id & 8388607) == 0)) {
					parray[j].print();
			}
		}
	}
}

bool check_verification(packet *p) {
	return p->verification ^ 0xffffffff == p->id;
}
