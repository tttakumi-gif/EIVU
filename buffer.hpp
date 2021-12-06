#pragma once

#include "packet.hpp"
#include "option.hpp"

#define PROC_CLT_S 0
#define PROC_CLT_R 1
#define PROC_SRV 2
#define TOTAL_CLT 3
#define TOTAL_SRV 4
#define NONE 5

#define INFO_CPU NONE

//#define CPU_BIND

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

constexpr int_fast32_t SIZE_RING = 256;
//constexpr int_fast32_t SIZE_POOL = 256;
//constexpr int_fast32_t SIZE_POOL = 163456;
constexpr int_fast32_t SIZE_POOL = 326912;
//constexpr int_fast32_t NUM_PMOD = SIZE_POOL / 2;

class desc {
public:
	int_fast32_t id;
	packet *entry;

	void set_param(int_fast32_t, buf*);
	void set_param_avoid(int_fast32_t);
	void delete_info();
	void delete_info_avoid();
};

class ring {
private:
	int_fast32_t pindex;
	int_fast32_t rsrv_idx;
	int_fast32_t recv_idx;
	int_fast32_t proc_idx;
//	int32_t rsrv_idx;
//	int32_t recv_idx;
//	int32_t proc_idx;
	int_fast32_t size;
public:
	ring *ring_pair;
	desc descs[SIZE_RING];

private:
	void init_descs();
	void wait_push(int_fast32_t);
	void wait_push(int_fast32_t, packet*);
	packet* wait_pull(int_fast32_t, buf*);
	void wait_pull_avoid(int_fast32_t);

public:
	ring();
	void operator=(ring&&);

	void zero_push(buf*, int_fast32_t, bool);

	void ipush(packet[], buf*, int_fast32_t, bool);
	void ipush_avoid(int_fast32_t, bool);
	void pull(packet[], buf*, int_fast32_t, bool);
	void pull_avoid(int_fast32_t);
#if defined(AVOID_SRV)
	void move_packet(int_fast32_t);
#else
	void move_packet(buf*, int_fast32_t);
#endif
	void move_packet_read(packet[], int_fast32_t);
	void set_ringaddr(ring*);
	void set_pooladdr(packet*);
};

#include "cbuffer.hpp"
