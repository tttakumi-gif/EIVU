#include <string.h>
#include <iostream>
#include <bitset>
#include <thread>
#include <atomic>
#include <mutex>

#include "cpuinfo.hpp"

//#define NUM_PACKET 5000000
#define NUM_PACKET 10000000
//#define NUM_PACKET 25000000
//#define NUM_PACKET 100000000
#define SIZE_PACKET 16

void do_none() {
}

constexpr uint_fast16_t DUMMY_SIZE = SIZE_PACKET - sizeof(uint32_t) - sizeof(uint32_t) - sizeof(uint16_t);

class packet {
public:
	uint32_t id;
	uint_fast8_t len;
	char dummy[DUMMY_SIZE];
	uint32_t verification;

	packet();
	packet(uint32_t);
	packet(uint32_t, const char*);

	void print();
	void set_verification();

};

inline packet::packet() {
	len = 0;
}

inline packet::packet(uint32_t this_id) {
	id = this_id;
	len = sizeof(packet);
}

inline packet::packet(uint32_t this_id, const char* this_dummy) {
	id = this_id;
	len = sizeof(packet);
	strncpy(dummy, this_dummy, DUMMY_SIZE);
}

inline void packet::print() {
	printf("id: %d, len: %d, dummy: %s, verification: 0x%x\n", id, len, dummy, verification);
}

inline void packet::set_verification() {
	verification = id ^ 0xffffffff;
}
