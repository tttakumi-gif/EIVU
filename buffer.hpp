#pragma once

#include "packet.hpp"
#include "option.hpp"
#include "random.hpp"

#define PROC_CLT_S 0
#define PROC_CLT_R 1
#define PROC_SRV 2
#define TOTAL_CLT 3
#define TOTAL_SRV 4
#define NONE 5

#define INFO_CPU NONE

#define CPU_BIND

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

//constexpr int SIZE_RING = 64;
constexpr int SIZE_RING = 256;
//constexpr int SIZE_RING = 4096;
//constexpr int SIZE_RING = 32768;
#if 0 
constexpr int SIZE_POOL = 1024;
#else
constexpr int SIZE_POOL = 163456;
#endif

//constexpr int_fast32_t NUM_PMOD = SIZE_POOL / 2;
constexpr int AVAIL_FLAG = 0b1 << 7;
constexpr int USED_FLAG = 0b1 << 15;

struct desc {
#if VQENTRY_SIZE == 8
	int32_t entry_index;
#else
	int64_t entry_index;
	int32_t len;
#endif
	int16_t id;
	int16_t flags;
#if VQENTRY_SIZE == 64 
	char padding[48];
#endif
};// __attribute__((__aligned__(2)));

struct vring_desc {
	uint64_t addr;
	uint32_t len;
	uint16_t flags;
	uint16_t next;
};

struct vring_avail {
	uint16_t flags;
	uint16_t idx;
	uint16_t ring[0];
};

struct vring_used_elem {
	uint32_t id;
	uint32_t len;
};

struct vring_used {
	uint16_t flags;
	uint16_t idx;
	struct vring_used_elem ring[0];
};

struct ring {
	int16_t size;
	uint16_t last_avail_idx;
	uint16_t last_used_idx;
	int32_t pool_index;
	//int_fast32_t __attribute__((__aligned__(64))) last_avail_idx;
	//int_fast32_t __attribute__((__aligned__(64))) last_used_idx;
	//int_fast32_t __attribute__((__aligned__(64))) pool_index;
	desc descs[SIZE_RING];
//	vring_desc vdesc[SIZE_RING];
//	vring_avail vavail[SIZE_RING];
//	vring_used vused[SIZE_RING];
};// __attribute__((__aligned__(64)));

#include "cbuffer.hpp"
