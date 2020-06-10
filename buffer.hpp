#include <iostream>
#include <vector>

#include "packet.hpp"

#define SIZE_BATCH 8
#define NUM_THREAD 64

extern uint32_t nums[NUM_THREAD + 1];

constexpr uint16_t SIZE_RING = NUM_THREAD * SIZE_BATCH;
constexpr uint16_t SIZE_POOL = SIZE_RING * 2;

constexpr uint16_t POOL_ADD = NUM_THREAD * 2;
constexpr uint16_t NUM_MOD = SIZE_RING - 1;

constexpr uint16_t NUM_ACCESS = SIZE_RING / NUM_THREAD;
constexpr uint16_t MOD_ACCESS = NUM_ACCESS - 1;

enum rsource {
	CLT = false,
	SRV = true,
};

enum dstatus {
	INIT,
	PUSH,
	PULL,
};

class desc {
public:
	dstatus status;
	uint16_t id;
	uint16_t len;
	bool is_used;
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

	uint16_t get_index(packet*, rsource, short);
	bool push(packet, packet*, rsource, short);
	bool dinit(short);
	packet pull(packet*, short);
	void init_descs();
};

void set_packet_nums(uint32_t*);
