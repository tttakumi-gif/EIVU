#include <iostream>
#include <vector>

#include "packet.hpp"

#define INFO_CPU 2

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

constexpr int_fast32_t SIZE_BATCH = 32;
constexpr int_fast32_t SIZE_RING = SIZE_BATCH;
constexpr int_fast32_t SIZE_POOL = SIZE_RING * 2;
constexpr int_fast32_t NUM_MOD = SIZE_RING - 1;

enum rsource {
	CLT = 0,
	SRV = SIZE_RING,
};

class desc {
public:
	int_fast32_t id;
	packet *entry;

	void set_param(int_fast32_t, packet*);
	void delete_info();
};

class ring {
public:
	uint_fast8_t proc_idx;
	uint_fast8_t rsrv_idx;
	uint_fast8_t recv_idx;
	uint_fast8_t size;
//	uint_fast32_t rsrv_idx;
//	uint_fast32_t recv_idx;
//	uint_fast32_t proc_idx;
	desc descs[SIZE_RING];

	ring();
	void operator=(ring&&);

	void ipush(packet[], packet[], rsource, uint_fast8_t);
	void pull(packet[], packet[], uint_fast8_t);
	void init_descs();
};

#include "cbuffer.hpp"
