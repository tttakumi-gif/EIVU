#pragma once

#include "cpuinfo.hpp"

#define DUMMY_FULL

constexpr int32_t HEADER_SIZE = 128;
constexpr int32_t SIZE_PADDING = 128;
constexpr int32_t SIZE_PACKET = 64;
constexpr bool IS_PSMALL = SIZE_PACKET < 32;
constexpr int32_t NUM_LOOP = SIZE_PACKET / (IS_PSMALL ? 16 : 32) + (SIZE_PACKET % 32 == 0 ? 0 : 1);
constexpr int32_t NUM_LOOP2 = SIZE_PACKET / 64;

#if 1
//constexpr int32_t NUM_PACKET = 1024;
constexpr int32_t NUM_PACKET = 100000000;
#else
constexpr int32_t NUM_PACKET = 100000000000000000;
#endif

struct packet {
	int32_t packet_id;
	int32_t packet_len;
#ifndef VERIFICATION
	char dummy[SIZE_PACKET - sizeof(int32_t) * 2];
#else
	char dummy[SIZE_PACKET - sizeof(int32_t) * 3];
	int32_t verification;
#endif
};

#if 1 
constexpr int32_t SIZE_BUFFER = 2176;
#else
constexpr int32_t SIZE_BUFFER = sizeof(packet);
#endif

struct mbuf_header {
	char id_addr[HEADER_SIZE / 2];
	char len_addr[HEADER_SIZE / 2];
};

struct buf {
	mbuf_header header;
	char padding[SIZE_PADDING];
	char addr[SIZE_BUFFER - SIZE_PADDING];
};

void do_none() {
}

void print(packet* p) {
#ifdef VERIFICATION
	std::printf("id: %d, len: %d, dummy: %s, verification: 0x%x\n", p->packet_id, p->packet_len, p->dummy, p->verification);
#else
	std::printf("id: %d, len: %d, dummy: %s\n", p->packet_id, p->packet_len, p->dummy);
#endif
}

#ifdef VERIFICATION
void set_verification(packet* p) {
#ifdef ZERO_COPY
	p->verification = p->verification + p->packet_id + 1;
#else
	p->verification = p->packet_id ^ 0xffffffff;
#endif
}
#endif

packet* get_packet_addr(buf* buffer) {
	return (packet*)buffer->addr;
}

void set_id(buf* buffer, int32_t id) {
	*(int32_t*)buffer->header.id_addr = id;
}

int32_t get_id(buf* buffer) {
	return *(int32_t*)buffer->header.id_addr;
}

void set_len(buf* buffer, int32_t len) {
	*(int32_t*)buffer->header.len_addr = len;
}

int32_t get_len(buf* buffer) {
	return *(int32_t*)buffer->header.len_addr;
}
