#pragma once

#include "packet.hpp"

#if 0
constexpr int POOL_ENTRY_NUM = 512;
//constexpr int POOL_ENTRY_NUM = 8192;
#else
constexpr int POOL_ENTRY_NUM = 163456;
#endif

constexpr int POOL_CACHE_NUM = 512;

struct cache_stack {
    buf *cache[POOL_CACHE_NUM]{};
    int top = -1;
};

struct buffer_pool {
    buf __attribute__((__aligned__(64))) buffers[POOL_ENTRY_NUM]{};
    int last_pool_idx = 0;
    cache_stack cache;
};

#include "buffer_pool_impl.hpp"
