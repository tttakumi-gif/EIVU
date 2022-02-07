#pragma once

#include "cpuinfo.hpp"

#define DUMMY_FULL

//constexpr int_fast32_t SIZE_PACKET_DATA = 64;
constexpr int_fast32_t SIZE_PACKET_DATA = 1518;
constexpr int_fast32_t header_size = 128;
constexpr int_fast32_t SIZE_PACKET = SIZE_PACKET_DATA + header_size;
constexpr bool IS_PSMALL = SIZE_PACKET < 32;
constexpr int_fast32_t NUM_LOOP = SIZE_PACKET / (IS_PSMALL ? 16 : 32) + (SIZE_PACKET % 32 == 0 ? 0 : 1);
//constexpr int_fast32_t NUM_LOOP2 = SIZE_PACKET / 64;

#if 1
//constexpr int_fast32_t NUM_PACKET = 32000000;
//constexpr int_fast32_t NUM_PACKET = 50000000;
constexpr int_fast32_t NUM_PACKET = 100000000;
//constexpr int_fast32_t NUM_PACKET = 1000000000;
#else
constexpr int_fast32_t NUM_PACKET = 100000000000000000;
#endif

class packet {
private:
	char id_addr[header_size / 2];
	char len_addr[header_size / 2];
	char dummy[SIZE_PACKET_DATA];
public:
	packet();
	packet(int32_t);
	packet(int32_t, const char*);

	void print();

	int32_t get_id();
	void set_id(int32_t);

	int32_t get_len();
	int32_t* get_len_addr();
	void set_len(int32_t);

	int32_t get_verification();
	void set_verification();
};

#if 1 
constexpr int_fast32_t SIZE_BUFFER = 2048;
#else
constexpr int_fast32_t SIZE_BUFFER = sizeof(packet);
#endif

struct buf {
	char addr[SIZE_BUFFER];
};

//constexpr int_fast32_t DUMMY_SIZE = get_dummy_size(sizeof(packet::id), sizeof(packet::verification), sizeof(packet::len));
//char GLOBAL_DUMMY[DUMMY_SIZE];

void do_none() {
}

packet::packet() {
	set_len(0);
}

packet::packet(int32_t this_id) {
	set_id(this_id);
	set_len(SIZE_PACKET);
}

void packet::print() {
	std::printf("id: %d, len: %d, dummy: %s, verification: 0x%x\n", get_id(), get_len(), dummy, get_verification());
}

void packet::set_verification() {
#ifdef ZERO_COPY
	*(int32_t*)(&dummy[SIZE_PACKET_DATA - 4]) = get_verification() + get_id() + 1;
#else
	*(int32_t*)(&dummy[SIZE_PACKET_DATA - 4]) = get_id() ^ 0xffffffff;
#endif
}

int32_t packet::get_id() {
	return *(int32_t*)(id_addr);
}

void packet::set_id(int32_t id) {
	*(int32_t*)(id_addr) = id;
}

int32_t packet::get_len() {
	return *(int32_t*)(len_addr);
}

int32_t* packet::get_len_addr() {
	return (int32_t*)(len_addr);
}

void packet::set_len(int32_t len) {
	*(int32_t*)(len_addr) = len;
}

int32_t packet::get_verification() {
	return *(int32_t*)(&dummy[SIZE_PACKET_DATA - 4]);
}

