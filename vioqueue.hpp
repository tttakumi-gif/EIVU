#pragma once

#include "packet.hpp"
#include "buffer_pool.hpp"
#include "option.hpp"

#define PROC_CLT_S 0
#define PROC_CLT_R 1
#define PROC_SRV 2
#define TOTAL_CLT 3
#define TOTAL_SRV 4
#define NONE 5

#define INFO_CPU NONE

#define CPU_BIND

//#define likely(x) __builtin_expect(!!(x), 1)
//#define unlikely(x) __builtin_expect(!!(x), 0)

//constexpr int VQ_ENTRY_NUM = 64;
constexpr int VQ_ENTRY_NUM = 256;
//constexpr int VQ_ENTRY_NUM = 32768;

constexpr int16_t AVAIL_FLAG = static_cast<int16_t>(0b1 << 7);
constexpr int16_t USED_FLAG = static_cast<int16_t>(0b1 << 15);

struct __attribute__((__aligned__(VQ_ENTRY_SIZE))) desc {
#if VQ_ENTRY_SIZE == 4
    int16_t entry_index;
    int16_t flags;
#elif VQ_ENTRY_SIZE == 8
    int32_t entry_index;
    int16_t id;
    int16_t flags;
#elif VQ_ENTRY_SIZE == 16
    int64_t entry_index;
    int32_t len;
    int16_t id;
    int16_t flags;
#elif VQ_ENTRY_SIZE == 32
    int64_t entry_index;
    int32_t len;
    int16_t id;
    int16_t flags;
    char padding[16];
#elif VQ_ENTRY_SIZE == 64
    int64_t entry_index;
    int32_t len;
    int16_t id;
    int16_t flags;
    char padding[48];
#elif VQ_ENTRY_SIZE == 128
    char padding[56];
    int64_t entry_index;
    int32_t len;
    int16_t id;
    int16_t flags;
    char padding[56];
#endif
};

struct vq {
    int16_t size;
    uint16_t last_avail_idx;
    uint16_t last_used_idx;
    uint16_t last_inflight_idx;
    desc *descs;
};

#include "vioqueue_impl.hpp"
