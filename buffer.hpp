#pragma once

#include "packet.hpp"
#include "option.hpp"
#include "random.hpp"

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

constexpr int VQ_ENYRY_NUM = 256;
constexpr int POOL_ENTRY_NUM = 163456;

constexpr int AVAIL_FLAG = 0b1 << 7;
constexpr int USED_FLAG = 0b1 << 15;

struct desc {
    int64_t entry_index;
    int32_t len;
    int16_t id;
    int16_t flags;
};

struct vq {
    int16_t size;

    uint16_t last_avail_idx;
    uint16_t last_used_idx;
    int32_t last_pool_idx;

    desc descs[VQ_ENYRY_NUM];
};

#include "cbuffer.hpp"
