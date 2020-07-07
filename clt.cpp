#include <chrono>
#include "buffer.hpp"
#include "shm.hpp"

void send_packet(ring&, packet[NUM_THREAD]);
void recv_packet(ring&, packet[NUM_THREAD]);
bool check_verification(packet);

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

	std::thread thread_send(send_packet, std::ref(*csring), std::ref(pool));

	std::chrono::system_clock::time_point start, end;
	start = std::chrono::system_clock::now();

	recv_packet(*scring, pool);
	thread_send.join();

	end = std::chrono::system_clock::now();

	shm_unlink("shm_buf");

	double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	std::cout << elapsed / 1000 << "sec" << std::endl;

	return 0;
}

void send_packet(ring &csring, packet pool[NUM_THREAD]) {
	int_fast32_t i;
	int_fast32_t j;
	int_fast32_t num_fin = SIZE_BATCH;
	packet parray[SIZE_BATCH];

	for(i = 0; i < NUM_PACKET; i += SIZE_BATCH) {
		j = 0;
		for(packet& p : parray) {
			p = packet(j + i);
			j++;
		}
		csring.ipush(parray, pool, CLT, num_fin);
	}

	if(NUM_PACKET < i) {
		j = 0;
		for(i -= SIZE_BATCH; i < NUM_PACKET; i++, j++) {
			parray[j] = packet(i);
		}
		csring.ipush(parray, pool, CLT, j);
	}
}

void recv_packet(ring &scring, packet pool[NUM_THREAD]) {
	int_fast32_t i;
	int_fast32_t num_fin = SIZE_BATCH;
	packet parray[SIZE_BATCH];

	for(i = 0; i < NUM_PACKET; i += SIZE_BATCH) {
		scring.pull(parray, pool, num_fin);
		for(packet p : parray) {
			if(unlikely(!check_verification(p))) {
				puts("verification error");
				exit(1);
			}

			if(unlikely((p.id & 8388607) == 0)) {
#if INFO_CPU == 0
				printf("%g%\n", getCurrentValue_p());
#endif
				p.print();
			}
		}
	}

	if(NUM_PACKET < i) {
		i -= SIZE_BATCH;
		num_fin = NUM_PACKET - i;
		scring.pull(parray, pool, num_fin);

		for(i = 0; i < num_fin; i++) {
			if(unlikely(!check_verification(parray[i]))) {
				puts("verification error");
				exit(1);
			}

			if(unlikely((parray[i].id & 8388607) == 0)) {
#if INFO_CPU == 0
				printf("%g%\n", getCurrentValue_p());
#endif
				parray[i].print();
			}
		}
	}
}

bool check_verification(packet p) {
	return likely((p.verification ^ 0xffffffff) == p.id);
}
