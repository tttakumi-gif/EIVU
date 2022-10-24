bool is_empty(cache_stack *stack) {
    return stack->top == -1 || POOL_CACHE_NUM == 0;
}

bool is_full(cache_stack *stack) {
    return stack->top == (POOL_CACHE_NUM - 1) || POOL_CACHE_NUM == 0;
}

buf *pop(cache_stack *stack) {
    if (is_empty(stack)) {
        return nullptr;
    }

    buf *data = stack->cache[stack->top];
    stack->top = stack->top - 1;
    return data;
}

void push(cache_stack *stack, buf *data) {
    if (is_full(stack)) {
        return;
    }

    stack->top = stack->top + 1;
    stack->cache[stack->top] = data;
}
buf* get_buffer(buffer_pool*);
void init(buffer_pool *pool) {
    pool->last_pool_idx = 0;
    memset(&pool->buffers, 0, sizeof(buf) * POOL_ENTRY_NUM);
    for (auto &buffer: pool->buffers) {
        set_len(&buffer, -1);
    }

    memset(pool->cache.cache, 0, sizeof(buf *) * POOL_CACHE_NUM);
    pool->cache.top = -1;

    for (int i = 0; i < POOL_CACHE_NUM; i++) {
        buf* buffer = get_buffer(pool);
        push(&pool->cache, buffer);
    }
}

buf *get_buffer(buffer_pool *pool) {
    buf *cache = pop(&pool->cache);
    if (cache != nullptr) {
        return cache;
    }

    int index = pool->last_pool_idx;
    buf *buffer = &pool->buffers[index];

    pool->last_pool_idx += 1;
    if (pool->last_pool_idx >= POOL_ENTRY_NUM) {
        pool->last_pool_idx = 0;
    }
    return buffer;
}

uint64_t get_buffer_index(buffer_pool *pool, buf *buffer) {
    return ((intptr_t) buffer - (intptr_t) &pool->buffers) / sizeof(buf);
}

void add_to_cache(buffer_pool *pool, buf *data) {
    push(&pool->cache, data);
}
