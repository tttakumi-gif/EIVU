#include <iostream>
#include <vector>

#include "packet.hpp"

#define INFO_CPU 2
#define SIZE_BATCH 18
#define NUM_THREAD 64

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

extern uint32_t nums[NUM_THREAD + 1];

constexpr int_fast32_t SIZE_RING = NUM_THREAD * SIZE_BATCH;
constexpr int_fast32_t SIZE_POOL = SIZE_RING * 2;
constexpr int_fast32_t NUM_MOD = SIZE_RING - 1;

constexpr int_fast8_t MOD_ACCESS = SIZE_BATCH - 1;

enum rsource : uint_fast8_t {
	CLT,
	SRV,
};

enum dstatus : uint_fast8_t {
	INIT,
	PUSH,
	PULL,
};

class desc {
public:
	dstatus status;
	uint16_t id;
	uint_fast8_t len;
	//bool is_used;
	packet *entry;

	desc();
	desc(dstatus);

	void set_param(packet, uint_fast16_t, dstatus);
	void delete_info(dstatus);
};

class ring {
public:
	uint_fast16_t rsrv_idx[NUM_THREAD];
	//uint_fast16_t recv_idx[NUM_THREAD];
	//uint_fast16_t proc_idx[NUM_THREAD];
	uint16_t size;
	desc descs[SIZE_RING];

	ring();
	void operator=(ring&&);

	uint_fast32_t get_index(packet[NUM_THREAD], rsource, uint_fast8_t);
	bool dinit(uint_fast8_t);
	bool push(packet, packet[NUM_THREAD], rsource, uint_fast8_t);
	void ipush(packet[SIZE_BATCH], packet[NUM_THREAD], rsource, uint_fast8_t);
	void pull(packet[SIZE_BATCH], packet[NUM_THREAD], uint_fast16_t);
	void init_descs();
};

void set_packet_nums(uint32_t*);
#include "cbuffer.hpp"
