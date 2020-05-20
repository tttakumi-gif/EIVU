#include "buffer.hpp"
#include "shm.hpp"

void send_packet(ring*, packet*, int);
void recv_packet(ring*, packet*, int);
bool check_verification(packet*);

const std::string base_text = "take";

int main() {
	puts("begin");

	int bfd = open_shmfile("shm_buf", SIZE_SHM, true);
	ring *csring = (ring*)mmap(NULL, SIZE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, bfd, 0);
	*csring = ring();
	ring *scring = (ring*)(csring + 1);
	*scring = ring();
	packet *pool = (packet*)(scring + 1);
	memset(pool, 0, sizeof(packet) * 2 * SIZE_POOL);
	bool *flag = (bool*)(pool + 2 * SIZE_POOL);
	*flag = false;

	set_packet_nums(nums);
	std::cout << "size: " << sizeof(ring) << std::endl;

	while(!*flag) {
		;
	}

	int nthread = NUM_THREAD * 2 - 1;
	std::thread threads[nthread];
	for(int i = 0; i < NUM_THREAD; i++) {
		threads[i] = std::thread(send_packet, std::ref(csring), std::ref(pool), i);
	}
	for(int i = NUM_THREAD; i < nthread; i++) {
		threads[i] = std::thread(recv_packet, std::ref(scring), std::ref(pool), i - NUM_THREAD);
	}

	recv_packet(scring, pool, NUM_THREAD - 1);
	for(int i = 0; i < nthread; i++) {
		threads[i].join();
	}
	shm_unlink("shm_buf");

	return 0;
}

void send_packet(ring *csring, packet *pool, int id) {
	std::string text;
	int index_begin = nums[id];
	int index_end = nums[id + 1];

	for(int i = index_begin; i < index_end;) {
		if(csring->dinit(id)) {
			text = base_text + std::to_string(i);
			while(true) {
				if(csring->push(packet(i, text.c_str()), pool, 0, id)) {
					break;
				}
			}

			i++;
		}
	}
}

void recv_packet(ring *scring, packet *pool, int id) {
	int index_begin = nums[id];
	int index_end = nums[id + 1];

	for(int i = index_begin; i < index_end;) {
		packet p = scring->pull(pool, id);
		if(0 < p.len) {
			if(p.id % 500000 == 0) {
				if(check_verification(&p)) {
					p.print();
				}
				else {
					puts("asdfa");
				}
			}
			i++;
		}
	}
}

bool check_verification(packet *p) {
	return p->verification ^ 0xffffffff == p->id;
}
