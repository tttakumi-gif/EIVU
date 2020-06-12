#include <iostream>
#include <vector>

#include "packet.hpp"

#define SIZE_BATCH 16
#define NUM_THREAD 64

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

extern uint32_t nums[NUM_THREAD + 1];

constexpr uint16_t SIZE_RING = NUM_THREAD * SIZE_BATCH;
constexpr uint16_t SIZE_POOL = SIZE_RING * 2;
constexpr uint16_t NUM_MOD = SIZE_RING - 1;

constexpr uint8_t MOD_ACCESS = SIZE_BATCH - 1;

enum rsource : uint_fast8_t {
	CLT,
	SRV,
};

enum dstatus : uint_fast8_t {
	PUSH,
	PULL,
};

class desc {
public:
	dstatus status;
	uint16_t id;
	uint8_t len;
	//bool is_used;
	packet *entry;

	desc();
	desc(dstatus);

	void set_param(packet, uint16_t, dstatus);
	void delete_info(dstatus);
};

class ring {
public:
	uint16_t rsrv_idx[NUM_THREAD];
	uint16_t recv_idx[NUM_THREAD];
	uint16_t proc_idx[NUM_THREAD];
	uint16_t size;
	desc descs[SIZE_RING];

	ring();
	ring(const ring&);
	ring(ring&&);
	ring& operator=(const ring&);
	ring&& operator=(ring&&);

	uint16_t get_index(packet*, rsource, uint_fast8_t);
	bool dinit(uint_fast8_t);
	bool push(packet, packet*, rsource, uint_fast8_t);
	void ipush(packet, packet*, rsource, uint_fast8_t);
	packet pull(packet*, uint_fast8_t);
	void init_descs();
};

void set_packet_nums(uint32_t*);
#include "cbuffer.hpp"
