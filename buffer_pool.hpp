#pragma once

#include "packet.hpp"

// guest
#if 0
constexpr int GUEST_POOL_ENTRY_NUM = 512;
//constexpr int GUEST_POOL_ENTRY_NUM = 8192;
#else
constexpr int GUEST_POOL_ENTRY_NUM = 163456;
#endif

constexpr int GUEST_POOL_CACHE_NUM = 512;

struct guest_cache_stack {
    buf *cache[GUEST_POOL_CACHE_NUM]{};
    int top = -1;
};

struct guest_buffer_pool {
    buf __attribute__((__aligned__(64))) buffers[GUEST_POOL_ENTRY_NUM]{};
    int last_pool_idx = 0;
    guest_cache_stack cache;
};

// host
#if 0
constexpr int HOST_POOL_ENTRY_NUM = 512;
//constexpr int HOST_POOL_ENTRY_NUM = 8192;
#else
constexpr int HOST_POOL_ENTRY_NUM = 163456;
#endif

constexpr int HOST_POOL_CACHE_NUM = 512;

struct host_cache_stack {
    buf *cache[GUEST_POOL_CACHE_NUM]{};
    int top = -1;
};

struct host_buffer_pool {
    buf __attribute__((__aligned__(64))) buffers[GUEST_POOL_ENTRY_NUM]{};
    int last_pool_idx = 0;
    host_cache_stack cache;
};

#include "buffer_pool_impl.hpp"
