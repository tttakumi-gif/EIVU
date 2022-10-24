#pragma once

#if 1
#define PROC_MBUF_HEADER(addr) \
            set_id(addr, (get_id(addr) + 1) & 2047); \
            set_len(addr, (get_len(addr) + 1) & 2047);
#else
#define PROC_MBUF_HEADER(addr)
#endif

#if MBUF_HEADER_SIZE > 0
#define PREFETCH_MBUF(addr1, addr2) \
        prefetch0(addr1); \
        prefetch0(addr2);
#else
#define PREFETCH_MBUF(addr1, addr2);
#endif

#define PREFETCH_POOL(addr) \
        prefetch0(addr);

static inline void prefetch0(const volatile void *p) {
    asm volatile ("prefetcht0 %[p]" : : [p] "m"(*(const volatile char *) p));
}

static inline void cldemote(const volatile void *p) {
    asm volatile(".byte 0x0f, 0x1c, 0x06"::"S" (p));
}

void set_used_flag(desc *d) {
    volatile int16_t* flags = &d->flags;
    int16_t f;
   __atomic_load(flags, &f, __ATOMIC_RELEASE);
    f |= USED_FLAG;
    f &= ~AVAIL_FLAG;
    __atomic_store(flags, &f, __ATOMIC_RELEASE);
}

void set_avail_flag(desc *d) {
    volatile int16_t* flags = &d->flags;
    int16_t f;
    __atomic_load(flags, &f, __ATOMIC_RELEASE);
    f &= ~USED_FLAG;
    f |= AVAIL_FLAG;
    __atomic_store(flags, &f, __ATOMIC_RELEASE);
}

static constexpr uint64_t WAIT_COUNT_THRESH = 1000000000;

void wait_used(vq *v, int desc_idx) {
    static uint64_t wait_counter = 0;
    // ディスクリプタが空くまでwait
    while ((*((volatile int16_t*)&v->descs[desc_idx].flags) & USED_FLAG) == 0) {
        if (wait_counter++ >= WAIT_COUNT_THRESH) {
            throw std::runtime_error("wait_used");
	}
    }
    wait_counter = 0;
}

void wait_avail(vq *v, int desc_idx) {
    static uint64_t wait_counter = 0;
    // ディスクリプタにバッファが割り当てられるまでwait
    while ((*((volatile int16_t*)&v->descs[desc_idx].flags) & AVAIL_FLAG) == 0) {
        if (wait_counter++ >= WAIT_COUNT_THRESH) {
            throw std::runtime_error("wait_avail");
	}
    }
    wait_counter = 0;
}

void init_ring(vq *v, desc *d) {
    v->size = VQ_ENTRY_NUM;
    v->last_used_idx = 0;
    v->last_avail_idx = 0;
    v->last_inflight_idx = 0;
    v->descs = d;
}

char ids[32] = {21, 10, 24, 22, 15, 31, 0, 30, 14, 1, 11, 2, 13, 23, 12, 3, 25, 17, 4, 16, 26, 19, 5, 28, 20, 6, 27, 7,
                8, 18, 29, 9};

void send_rx_to_guest(vq *vq_rx, buf **buf_src, buffer_pool *pool_guest, int num_fin, bool is_stream) {
    const uint16_t RING_SIZE_MASK = vq_rx->size - 1;

    for (int i = 0; i < num_fin; i++) {
        PREFETCH_MBUF(buf_src[i]->header.id_addr, buf_src[i]->header.len_addr);
        //PREFETCH_MBUF(buf_src[i]->header.id_addr, buf_src[i]->header.id_addr);
        //    PREFETCH_POOL(pool_guest_addr[i]);
    }

    wait_avail(vq_rx, (vq_rx->last_used_idx + num_fin - 1) & RING_SIZE_MASK);

#if MBUF_HEADER_SIZE > 0
    for (int i = 0; i < num_fin; i++) {
        //if(*(int*)(buf_src[i]->header.id_addr) == 999999) {
        //    exit(1);
        //}
        //memset(buf_src[i]->header.id_addr, i, 4);
        PROC_MBUF_HEADER(buf_src[i]);
    }
#endif

    int last_used_idx_shadow = vq_rx->last_used_idx;
    for (int i = 0; i < num_fin; i++) {
        if (!is_stream) {
#ifndef ZERO_COPY_RX
            int64_t copy_dest_index = vq_rx->descs[vq_rx->last_used_idx].entry_index;
            auto *copy_dest_addr = get_packet_addr(&pool_guest->buffers[copy_dest_index]);
            memcpy(copy_dest_addr, get_packet_addr(buf_src[i]), SIZE_PACKET);
            //cldemote(copy_dest_addr);
            //_mm_clflushopt(copy_dest_addr);
#endif
        } else {
//            int64_t copy_dest_index = vq_rx->descs[vq_rx->last_used_idx].entry_index;
//            void *xmm01 = get_packet_addr(&pool_guest_addr[copy_dest_index]);
//            auto *xmm02 = get_packet_addr(buf_src[i]);

            for (int j = 0; j < NUM_LOOP; j++) {
                if (!IS_PSMALL) {
//                    _mm256_stream_si256((__m256i *) xmm01 + j, _mm256_stream_load_si256((__m256i *) xmm02 + j));
//                    _mm256_store_si256((__m256i *) xmm01 + j, _mm256_load_si256((__m256i *) xmm02 + j));
                } else {
//                    _mm_stream_si128((__m128i *) xmm01 + j, _mm_stream_load_si128((__m128i *) xmm02 + j));
                }
            }
        }

        // index更新
        vq_rx->last_used_idx++;
	vq_rx->last_used_idx &= RING_SIZE_MASK;
    }

#ifdef SKIP_INDEX_RX
    int skipped_index = 64 / VQ_ENTRY_SIZE;
    for (int i = skipped_index; i < num_fin; i++) {
#else
    for (int i = 0; i < num_fin; i++) {
#endif
        int desc_index = (last_used_idx_shadow + i) & RING_SIZE_MASK;
        set_used_flag(&vq_rx->descs[desc_index]);
    }

#ifdef SKIP_INDEX_RX
    for (int i = 0; i < skipped_index; i++) {
        int desc_index = (last_used_idx_shadow + i) % VQ_ENTRY_NUM;
        set_used_flag(&vq_rx->descs[desc_index]);
    }
#endif
}

void send_guest_to_tx(vq *vq_tx, buf **buf_dest, buffer_pool *pool_guest, int num_fin, bool is_stream) {
    const uint16_t RING_SIZE_MASK = vq_tx->size - 1;

    wait_used(vq_tx, (vq_tx->last_avail_idx + num_fin - 1) & RING_SIZE_MASK);

    for (int i = 0; i < num_fin; i++) {
        PREFETCH_MBUF(buf_dest[i]->header.id_addr, buf_dest[i]->header.len_addr);
        //PREFETCH_MBUF(buf_dest[i]->header.id_addr, buf_dest[i]->header.id_addr);
        //PREFETCH_POOL(pool_guest_addr[i]);
    }

    int last_avail_idx_shadow = vq_tx->last_avail_idx;
    for (int i = 0; i < num_fin; i++) {
        if (!is_stream) {
#ifndef ZERO_COPY_TX
            int entry_idx = vq_tx->descs[vq_tx->last_avail_idx].entry_index;
            memcpy(get_packet_addr(buf_dest[i]), get_packet_addr(&pool_guest->buffers[entry_idx]), SIZE_PACKET);
            //cldemote(get_packet_addr(buf_dest[i]);
            //_mm_clflushopt(get_packet_addr(buf_dest[i]);
#endif
        } else {
//            auto *xmm01 = get_packet_addr(buf_dest[i];
//            auto *xmm02 = get_packet_addr(&pool_guest_addr[i]);
            for (int j = 0; j < NUM_LOOP; j++) {
                if (!IS_PSMALL) {
//                    _mm256_stream_si256((__m256i *) xmm01 + j, _mm256_stream_load_si256((__m256i *) xmm02 + j));
//                    _mm256_store_si256((__m256i *) xmm01 + j, _mm256_load_si256((__m256i *) xmm02 + j));
                } else {
//                    _mm_store_si128((__m128i *) xmm01 + j, _mm_stream_load_si128((__m128i *) xmm02 + j));
                }
            }
        }

        // index更新
        vq_tx->last_avail_idx++;
        vq_tx->last_avail_idx &= RING_SIZE_MASK; 
    }

#if MBUF_HEADER_SIZE > 0
    for (int i = 0; i < num_fin; i++) {
        //if(*(int*)(buf_dest[i]->header.id_addr) == 999999) {
        //    exit(1);
        //}
        //memset(buf_dest[i]->header.id_addr, i, 4);
        PROC_MBUF_HEADER(buf_dest[i])
    }
#endif

#ifdef SKIP_INDEX_TX
    int skipped_index = 64 / VQ_ENTRY_SIZE;
    for (int i = skipped_index; i < num_fin; i++) {
#else
    for (int i = 0; i < num_fin; i++) {
#endif
        int desc_index = (last_avail_idx_shadow + i) & RING_SIZE_MASK;
        set_avail_flag(&vq_tx->descs[desc_index]);
    }

#ifdef SKIP_INDEX_TX
    for (int i = 0; i < skipped_index; i++) {
        int desc_index = (last_avail_idx_shadow + i) % VQ_ENTRY_NUM;
        set_avail_flag(&vq_tx->descs[desc_index]);
    }
#endif
}

void guest_recv_process(vq *vq_rx, buffer_pool *pool, buf** pkts, int num_fin) {
    const uint16_t RING_SIZE_MASK = vq_rx->size - 1;

    wait_used(vq_rx, (vq_rx->last_avail_idx + num_fin - 1) & RING_SIZE_MASK);

    for (int i = 0; i < num_fin; i++) {
        int64_t index = vq_rx->descs[(vq_rx->last_avail_idx + i) & RING_SIZE_MASK].entry_index;
        PREFETCH_MBUF(pool->buffers[index].header.id_addr, pool->buffers[index].header.len_addr)
        //PREFETCH_MBUF(pool_guest_addr[index].header.id_addr, pool_guest_addr[index].header.id_addr)
    }

    int64_t id[num_fin];
    int last_avail_idx_shadow = vq_rx->last_avail_idx;

    for (int i = 0; i < num_fin; i++) {
        id[i] = vq_rx->descs[vq_rx->last_avail_idx].entry_index;
        pkts[i] = &pool->buffers[id[i]];

#if MBUF_HEADER_SIZE > 0
        PROC_MBUF_HEADER(pkts[i]);
        set_len(pkts[i], SIZE_PACKET);
#endif

        vq_rx->last_avail_idx++;
	vq_rx->last_avail_idx &= RING_SIZE_MASK;
    }

#ifdef SKIP_INDEX_NF
    int skipped_index = 64 / VQ_ENTRY_SIZE;
    for (int i = skipped_index; i < num_fin; i++) {
#else
    for (int i = 0; i < num_fin; i++) {
#endif
        int desc_idx = (last_avail_idx_shadow + i) & RING_SIZE_MASK;
#ifdef RANDOM_NF
        int next_buffer_index = ((vq_rx->last_pool_idx + i) % POOL_ENTRY_NUM) / 32 * 32 + ids[i];
#else
        auto next_buffer_index = static_cast<int64_t>(get_buffer_index(pool, get_buffer(pool)));
#endif
#if MBUF_HEADER_SIZE > 0
        assert(get_len(&pool->buffers[next_buffer_index]) == -1);
#endif
        vq_rx->descs[desc_idx].entry_index = next_buffer_index;
        set_avail_flag(&vq_rx->descs[desc_idx]);
    }
}

void guest_flush_inflight_buffers(vq* vq_tx, buffer_pool* pool) {
    const uint16_t RING_SIZE_MASK = vq_tx->size - 1;

    int n_free = vq_tx->last_used_idx - vq_tx->last_inflight_idx;
    if (n_free == 0) {
        return ;
    } else if (n_free < 0) {
        n_free += vq_tx->size;
    }

    int desc_idx = vq_tx->last_used_idx - 1;
    if (desc_idx < 0) {
        desc_idx = vq_tx->size - 1;
    }
 
    wait_avail(vq_tx, desc_idx);

    for (int i = 0; i < n_free; i++) {
        int entry_idx = vq_tx->descs[vq_tx->last_inflight_idx].entry_index;
        add_to_cache(pool, &pool->buffers[entry_idx]);
#if MBUF_HEADER_SIZE > 0
        set_len(&pool->buffers[entry_idx], -1);
#endif

        vq_tx->last_inflight_idx++;
        vq_tx->last_inflight_idx &= RING_SIZE_MASK;
    }
}

void guest_send_process(vq* vq_tx, buffer_pool* pool, buf** pkts, int num_fin) {
    const uint16_t RING_SIZE_MASK = vq_tx->size - 1;

    guest_flush_inflight_buffers(vq_tx, pool);

    wait_avail(vq_tx, (vq_tx->last_used_idx + num_fin - 1) & RING_SIZE_MASK);
    int last_used_idx_shadow = vq_tx->last_used_idx;

    for (int i = 0; i < num_fin; i++) {
        vq_tx->descs[vq_tx->last_used_idx].entry_index = get_buffer_index(pool, pkts[i]);
        set_used_flag(&vq_tx->descs[vq_tx->last_used_idx]);

        vq_tx->last_used_idx++;
        vq_tx->last_used_idx &= RING_SIZE_MASK;
    }

#ifdef SKIP_INDEX_NF
    for (int i = 0; i < skipped_index; i++) {
        int tx_desc_entry = (last_used_idx_shadow + i) % VQ_ENTRY_NUM;
        set_used_flag(&vq_tx->descs[tx_desc_entry]);
        vq_tx->descs[tx_desc_entry].entry_index = id[i];

        int rx_desc_entry = (last_avail_idx_shadow + i) % VQ_ENTRY_NUM;
        set_avail_flag(&vq_rx->descs[rx_desc_entry]);
    }
#endif
}
