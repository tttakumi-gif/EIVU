#include <iostream>
#include <vector>

#include "packet.hpp"

#define INFO_CPU 2
#define SIZE_BATCH 32
#define SIZE_RING SIZE_BATCH

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

constexpr int_fast32_t SIZE_POOL = SIZE_RING * 2;
constexpr int_fast32_t NUM_MOD = SIZE_RING - 1;

void set_packet_nums(uint32_t*);

enum rsource : int_fast32_t {
	CLT,
	SRV,
};

enum dstatus : int_fast32_t {
	INIT,
	PUSH,
	PULL,
};

class desc {
public:
	dstatus status;
	uint_fast8_t id;
	uint_fast8_t len;
	//bool is_used;
	packet *entry;

	desc();
	desc(dstatus);

	//void set_param(packet, uint_fast8_t, dstatus);
	void set_param(uint_fast8_t, dstatus);
	void delete_info(dstatus);
};

class ring {
public:
	uint_fast8_t rsrv_idx;
	//uint_fast16_t recv_idx[NUM_THREAD];
	uint_fast8_t proc_idx;
	uint_fast8_t size;
	desc descs[SIZE_RING];

	ring();
	void operator=(ring&&);

	uint_fast32_t get_index(packet, rsource, uint_fast8_t);
	bool dinit(uint_fast8_t);
	bool push(packet, packet, rsource, uint_fast8_t);
	//void ipush(packet[SIZE_BATCH], packet[NUM_THREAD], rsource, int_fast32_t);
	//void pull(packet[SIZE_BATCH], packet[NUM_THREAD], int_fast32_t);
	void ipush(packet[SIZE_BATCH], packet[SIZE_BATCH], rsource, uint_fast8_t);
	void pull(packet[SIZE_BATCH], packet[SIZE_BATCH], uint_fast8_t);
	void init_descs();
};

#include "cbuffer.hpp"
