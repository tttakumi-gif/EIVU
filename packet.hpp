#pragma once

#include "cpuinfo.hpp"

#define DUMMY_FULL

constexpr int_fast32_t SIZE_PACKET = 64;
//constexpr int_fast32_t SIZE_PACKET = 1024;
constexpr bool IS_PSMALL = SIZE_PACKET < 32;
constexpr int_fast32_t NUM_LOOP = SIZE_PACKET / (IS_PSMALL ? 16 : 32);
#if 1
//constexpr int_fast32_t NUM_PACKET = 75000000;
//constexpr int_fast32_t NUM_PACKET = 100000000;
//constexpr int_fast32_t NUM_PACKET = 300000000;
constexpr int_fast32_t NUM_PACKET = 1000000000;
#elif 0
constexpr int_fast32_t NUM_PACKET = 420000000;
#else
constexpr int_fast32_t NUM_PACKET = 100000000000000000;
#endif
//constexpr int_fast32_t NUM_PACKET = 500000000;
//constexpr int_fast32_t NUM_PACKET = 1000000000;

constexpr int_fast32_t get_dummy_size(int isize, int vsize, int lsize) {
	return SIZE_PACKET - isize - vsize - lsize;
}

class packet {
public:
	int32_t id;
	int32_t verification;
	int32_t len;
	char dummy[get_dummy_size(sizeof(id), sizeof(verification), sizeof(len))];

	packet();
	packet(int32_t);
	packet(int32_t, const char*);

	void print();
	void set_verification();

};

constexpr int_fast32_t DUMMY_SIZE = get_dummy_size(sizeof(packet::id), sizeof(packet::verification), sizeof(packet::len));
char GLOBAL_DUMMY[DUMMY_SIZE];

inline void do_none() {
}

inline packet::packet() {
	len = 0;
#ifdef DUMMY_FULL
	strncpy(dummy, GLOBAL_DUMMY, DUMMY_SIZE);
#endif
}

inline packet::packet(int32_t this_id) {
	id = this_id;
	len = SIZE_PACKET;
	strncpy(dummy, GLOBAL_DUMMY, DUMMY_SIZE);
}

// not use
inline packet::packet(int32_t this_id, const char* this_dummy) {
	id = this_id;
	len = sizeof(packet);
	strncpy(dummy, this_dummy, DUMMY_SIZE);
}

inline void packet::print() {
	std::printf("id: %d, len: %d, dummy: %s, verification: 0x%x\n", id, len, dummy, verification);
}

inline void packet::set_verification() {
#ifdef ZERO_COPY
	verification = verification + id + 1;
#else
	verification = id ^ 0xffffffff;
#endif
}

void set_global_dummy() {
	int i;
	for(i = 0; i < DUMMY_SIZE - 1; i++) {
		GLOBAL_DUMMY[i] = '0' + i;
	}
	GLOBAL_DUMMY[i] = '\0';
}
