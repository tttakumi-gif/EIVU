// guest
bool is_empty(guest_cache_queue *queue) {
    return queue->itemCount == 0 || GUEST_POOL_CACHE_NUM == 0;
}

bool is_full(guest_cache_queue *queue) {
    return queue->itemCount == GUEST_POOL_CACHE_NUM || GUEST_POOL_CACHE_NUM == 0;
}

buf *pop(guest_cache_queue *queue) {
    if (is_empty(queue)) {
        return nullptr;
    }

    buf *data = queue->cache[queue->front];
    queue->front += 1;

    if(queue->front >= GUEST_POOL_CACHE_NUM){
        queue->front = 0;
    }

    queue->itemCount -= 1;
    return data;
}

void push(guest_cache_queue *queue, buf *data) {
    if (is_full(queue)) {
        return;
    }

    if(queue->rear >= GUEST_POOL_CACHE_NUM - 1) {
        queue->rear = -1;
    }

    queue->rear += 1;
    queue->cache[queue->rear] = data;
    queue->itemCount += 1;
}

// host
bool is_empty(host_cache_queue *queue) {
    return queue->itemCount == 0 || GUEST_POOL_CACHE_NUM == 0;
}

bool is_full(host_cache_queue *queue) {
    return queue->itemCount == GUEST_POOL_CACHE_NUM || GUEST_POOL_CACHE_NUM == 0;
}

buf *pop(host_cache_queue *queue) {
    if (is_empty(queue)) {
        return nullptr;
    }

    buf *data = queue->cache[queue->front];
    queue->front += 1;

    if(queue->front >= GUEST_POOL_CACHE_NUM){
        queue->front = 0;
    }

    queue->itemCount -= 1;
    return data;
}

void push(host_cache_queue *queue, buf *data) {
    if (is_full(queue)) {
        return;
    }

    if(queue->rear >= GUEST_POOL_CACHE_NUM - 1) {
        queue->rear = -1;
    }

    queue->rear += 1;
    queue->cache[queue->rear] = data;
    queue->itemCount += 1;
}
