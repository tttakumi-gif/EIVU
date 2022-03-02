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

constexpr int SIZE_RING = 256;
//constexpr int_fast32_t SIZE_POOL = 256;
constexpr int SIZE_POOL = 163456;
//constexpr int_fast32_t NUM_PMOD = SIZE_POOL / 2;
constexpr int AVAIL_FLAG = 0b1 << 7;
constexpr int USED_FLAG = 0b1 << 15;

struct desc {
	int64_t entry_index;
	int32_t len;
	int16_t id;
	int16_t flags;
};

struct ring {
	int16_t size;
	int16_t last_avail_idx;
	int16_t last_used_idx;
	int32_t pool_index;
	desc descs[SIZE_RING];
}__attribute__((__aligned__(64)));

#include "cbuffer.hpp"
