// guest
bool is_empty(guest_cache_stack *stack) {
    return stack->top == -1 || GUEST_POOL_CACHE_NUM == 0;
}

bool is_full(guest_cache_stack *stack) {
    return stack->top == (GUEST_POOL_CACHE_NUM - 1) || GUEST_POOL_CACHE_NUM == 0;
}

buf *pop(guest_cache_stack *stack) {
    if (is_empty(stack)) {
        return nullptr;
    }

    buf *data = stack->cache[stack->top];
    stack->top = stack->top - 1;
    return data;
}

void push(guest_cache_stack *stack, buf *data) {
    if (is_full(stack)) {
        return;
    }

    stack->top = stack->top + 1;
    stack->cache[stack->top] = data;
}

buf *get_buffer(guest_buffer_pool *pool) {
    buf *cache = pop(&pool->cache);
    if (cache != nullptr) {
        return cache;
    }

    int index = pool->last_pool_idx;
    buf *buffer = &pool->buffers[index];

    pool->last_pool_idx += 1;
    if (pool->last_pool_idx >= GUEST_POOL_ENTRY_NUM) {
        pool->last_pool_idx = 0;
    }
    return buffer;
}

uint64_t get_buffer_index(guest_buffer_pool *pool, buf *buffer) {
    return ((intptr_t) buffer - (intptr_t) &pool->buffers) / sizeof(buf);
}

void add_to_cache(guest_buffer_pool *pool, buf *data) {
    push(&pool->cache, data);
}

void init(guest_buffer_pool *pool) {
    pool->last_pool_idx = 0;
    memset(&pool->buffers, 0, sizeof(buf) * GUEST_POOL_ENTRY_NUM);
    for (auto &buffer: pool->buffers) {
#if MBUF_HEADER_SIZE > 0
        set_len(&buffer, -1);
#endif
    }

    memset(pool->cache.cache, 0, sizeof(buf *) * GUEST_POOL_CACHE_NUM);

#ifdef QUEUE_BUFFER_POOL_CACHE_GUEST
    pool->cache.front = 0;
    pool->cache.rear = -1;
    pool->cache.itemCount = 0;
#else
    pool->cache.top = -1;
#endif

    for (int i = 0; i < GUEST_POOL_CACHE_NUM; i++) {
        buf* buffer = get_buffer(pool);
        push(&pool->cache, buffer);
    }
}

// host
bool is_empty(host_cache_stack *stack) {
    return stack->top == -1 || HOST_POOL_CACHE_NUM == 0;
}

bool is_full(host_cache_stack *stack) {
    return stack->top == (HOST_POOL_CACHE_NUM - 1) || HOST_POOL_CACHE_NUM == 0;
}

buf *pop(host_cache_stack *stack) {
    if (is_empty(stack)) {
        return nullptr;
    }

    buf *data = stack->cache[stack->top];
    stack->top = stack->top - 1;
    return data;
}

void push(host_cache_stack *stack, buf *data) {
    if (is_full(stack)) {
        return;
    }

    stack->top = stack->top + 1;
    stack->cache[stack->top] = data;
}

buf *get_buffer(host_buffer_pool *pool) {
    buf *cache = pop(&pool->cache);
    if (cache != nullptr) {
        return cache;
    }

    int index = pool->last_pool_idx;
    buf *buffer = &pool->buffers[index];

    pool->last_pool_idx += 1;
    if (pool->last_pool_idx >= HOST_POOL_ENTRY_NUM) {
        pool->last_pool_idx = 0;
    }
    return buffer;
}

uint64_t get_buffer_index(host_buffer_pool *pool, buf *buffer) {
    return ((intptr_t) buffer - (intptr_t) &pool->buffers) / sizeof(buf);
}

void add_to_cache(host_buffer_pool *pool, buf *data) {
    push(&pool->cache, data);
}

void init(host_buffer_pool *pool) {
    pool->last_pool_idx = 0;
    memset(&pool->buffers, 0, sizeof(buf) * HOST_POOL_ENTRY_NUM);
    for (auto &buffer: pool->buffers) {
#if MBUF_HEADER_SIZE > 0
        set_len(&buffer, -1);
#endif
    }

    memset(pool->cache.cache, 0, sizeof(buf *) * HOST_POOL_CACHE_NUM);

#ifdef QUEUE_BUFFER_POOL_CACHE_HOST
    pool->cache.front = 0;
    pool->cache.rear = -1;
    pool->cache.itemCount = 0;
#else
    pool->cache.top = -1;
#endif

    for (int i = 0; i < HOST_POOL_CACHE_NUM; i++) {
        buf* buffer = get_buffer(pool);
        push(&pool->cache, buffer);
    }
}