#pragma once

constexpr int32_t SIZE_PACKET = 64;
constexpr bool IS_PSMALL = SIZE_PACKET < 32;
constexpr int32_t NUM_LOOP = SIZE_PACKET / (IS_PSMALL ? 16 : 32) + (SIZE_PACKET % 32 == 0 ? 0 : 1);
constexpr int32_t NUM_PACKET = 100000000;

struct packet {
	int32_t packet_id;
	int32_t packet_len;
	int32_t verification;
	char dummy[SIZE_PACKET - sizeof(int32_t) * 3];
};

#define MBUF_HEADER_SIZE 128

struct mbuf_header {
	char id_addr[MBUF_HEADER_SIZE / 2];
	char len_addr[MBUF_HEADER_SIZE / 2];
};

constexpr int32_t SIZE_BUFFER = 2176;
constexpr int32_t PACKET_BUFFER_PADDING = 128;

struct buf {
	mbuf_header header;
	char padding[PACKET_BUFFER_PADDING];
	char addr[SIZE_BUFFER - PACKET_BUFFER_PADDING];
};

void do_none() {
}

packet* get_packet_addr(buf* buffer) {
	return (packet*)buffer->addr;
}

void set_id(buf* buffer, int32_t id) {
	memset(buffer->header.id_addr, id, MBUF_HEADER_SIZE / 2);
}

int32_t get_id(buf* buffer) {
	return *(int32_t*)buffer->header.id_addr;
}

void set_len(buf* buffer, int32_t len) {
	memset(buffer->header.len_addr, len, MBUF_HEADER_SIZE / 2);
}

int32_t get_len(buf* buffer) {
	return *(int32_t*)buffer->header.len_addr;
}
