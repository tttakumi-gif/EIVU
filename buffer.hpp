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

#define CPU_BIND

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

//constexpr int VQ_ENYRY_NUM = 64;
constexpr int VQ_ENYRY_NUM = 256;
//constexpr int VQ_ENYRY_NUM = 32768;
#if 1
constexpr int POOL_ENTRY_NUM = 1024;
#else
constexpr int POOL_ENTRY_NUM = 163456;
#endif

constexpr int AVAIL_FLAG = 0b1 << 7;
constexpr int USED_FLAG = 0b1 << 15;

struct desc {
#if VQ_ENTRY_SIZE == 128
    char padding[56];
#endif
#if VQ_ENTRY_SIZE == 8
    int32_t entry_index;
#else
    int64_t entry_index;
    int32_t len;
#endif
    int16_t id;
    int16_t flags;
#if VQ_ENTRY_SIZE == 64
    char padding[48];
#endif
#if VQ_ENTRY_SIZE == 128
    char padding2[56];
#endif
};

struct vq {
    int16_t size;
#if 1
    uint16_t last_avail_idx;
    uint16_t last_used_idx;
    int32_t last_pool_idx;
#else
    uint16_t __attribute__((__aligned__(64))) last_avail_idx;
    uint16_t __attribute__((__aligned__(64))) last_used_idx;
    int32_t __attribute__((__aligned__(64))) last_pool_idx;
#endif
    desc descs[VQ_ENYRY_NUM];
};

#include "cbuffer.hpp"
