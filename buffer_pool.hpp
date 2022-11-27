#pragma once

#include "packet.hpp"

//#define QUEUE_BUFFER_POOL_CACHE_GUEST
//#define QUEUE_BUFFER_POOL_CACHE_HOST

// guest
#if 0
constexpr int GUEST_POOL_ENTRY_NUM = 512;
//constexpr int GUEST_POOL_ENTRY_NUM = 8192;
#else
constexpr int GUEST_POOL_ENTRY_NUM = 163456;
#endif

#define GUEST_POOL_CACHE_ENTRIES 512
constexpr int GUEST_POOL_CACHE_NUM = GUEST_POOL_CACHE_ENTRIES;

struct guest_cache_stack {
    buf *cache[GUEST_POOL_CACHE_NUM]{};
    int top = -1;
};

struct guest_cache_queue {
    buf *cache[GUEST_POOL_CACHE_NUM]{};
    int front = 0;
    int rear = -1;
    int itemCount = 0;
};

struct guest_buffer_pool {
    buf __attribute__((__aligned__(64))) buffers[GUEST_POOL_ENTRY_NUM]{};
    int last_pool_idx = 0;
#ifdef QUEUE_BUFFER_POOL_CACHE_GUEST
    guest_cache_queue cache;
#else
    guest_cache_stack cache;
#endif
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
    buf *cache[HOST_POOL_CACHE_NUM]{};
    int top = -1;
};

struct host_cache_queue {
    buf *cache[HOST_POOL_CACHE_NUM]{};
    int front = 0;
    int rear = -1;
    int itemCount = 0;
};

struct host_buffer_pool {
    buf __attribute__((__aligned__(64))) buffers[HOST_POOL_ENTRY_NUM]{};
    int last_pool_idx = 0;
#ifdef QUEUE_BUFFER_POOL_CACHE_HOST
    host_cache_queue cache;
#else
    host_cache_stack cache;
#endif
};

#include "buffer_pool/buffer_pool_impl_queue.hpp"
#include "buffer_pool/buffer_pool_impl_stack.hpp"
