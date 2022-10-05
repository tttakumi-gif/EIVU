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
    d->flags = static_cast<int16_t>(d->flags | USED_FLAG);
    d->flags = static_cast<int16_t>(d->flags & ~AVAIL_FLAG);
}

void set_avail_flag(desc *d) {
    d->flags = static_cast<int16_t>(d->flags & ~USED_FLAG);
    d->flags = static_cast<int16_t>(d->flags | AVAIL_FLAG);
}

void wait_used(vq *v, int desc_idx) {
    // ディスクリプタが空くまでwait
    while ((*(volatile int16_t *) &v->descs[desc_idx].flags & USED_FLAG) == 0) {
        do_none();
    }
}

void wait_used(newvq *v, int desc_idx) {
    // ディスクリプタが空くまでwait
    while ((*(volatile int16_t *) &v->descs[desc_idx].flags & USED_FLAG) == 0) {
        do_none();
    }
}

void wait_avail(vq *v, int desc_idx) {
    // ディスクリプタにバッファが割り当てられるまでwait
    while ((*(volatile int16_t *) &v->descs[desc_idx].flags & AVAIL_FLAG) == 0) {
        do_none();
    }
}

void wait_avail(newvq *v, int desc_idx) {
    // ディスクリプタにバッファが割り当てられるまでwait
    while ((*(volatile int16_t *) &v->descs[desc_idx].flags & AVAIL_FLAG) == 0) {
        do_none();
    }
}

void init_descs(vq *v) {
    memset(v->descs, 0, sizeof(desc) * VQ_ENYRY_NUM);
    for (int i = 0; i < VQ_ENYRY_NUM; i++) {
        v->descs[i].id = static_cast<int16_t>(i);
        v->descs[i].flags = static_cast<int16_t>(v->descs[i].flags | USED_FLAG);
    }
}

void init_ring(vq *v) {
    v->size = VQ_ENYRY_NUM;
    v->last_used_idx = 0;
    v->last_avail_idx = 0;
    init_descs(v);
}

char ids[32] = {21, 10, 24, 22, 15, 31, 0, 30, 14, 1, 11, 2, 13, 23, 12, 3, 25, 17, 4, 16, 26, 19, 5, 28, 20, 6, 27, 7,
                8, 18, 29, 9};

void send_rx_to_guest(newvq *vq_rx_to_guest, buf **pool_host_addr, buffer_pool *pool_guest, int num_fin,
                      bool is_stream) {
    for (int i = 0; i < num_fin; i++) {
        PREFETCH_MBUF(pool_host_addr[i]->header.id_addr, pool_host_addr[i]->header.len_addr);
        //PREFETCH_MBUF(pool_host_addr[i]->header.id_addr, pool_host_addr[i]->header.id_addr);
        //    PREFETCH_POOL(pool_guest_addr[i]);
    }

    for (int i = 0; i < num_fin; i++) {
        wait_avail(vq_rx_to_guest, vq_rx_to_guest->last_used_idx + i);
    }

#if MBUF_HEADER_SIZE > 0
    for (int i = 0; i < num_fin; i++) {
        //if(*(int*)(pool_host_addr[i]->header.id_addr) == 999999) {
        //    exit(1);
        //}
        //memset(pool_host_addr[i]->header.id_addr, i, 4);
        PROC_MBUF_HEADER(pool_host_addr[i]);
    }
#endif

    int last_used_idx_shadow = vq_rx_to_guest->last_used_idx;
    for (int i = 0; i < num_fin; i++) {
        if (!is_stream) {
#ifndef ZERO_COPY_RX
            int64_t copy_dest_index = vq_rx_to_guest->descs[vq_rx_to_guest->last_used_idx].entry_index;
            auto *copy_dest_addr = get_packet_addr(&pool_guest->buffers[copy_dest_index]);
            memcpy(copy_dest_addr, get_packet_addr(pool_host_addr[i]), SIZE_PACKET);
            //cldemote(copy_dest_addr);
            //_mm_clflushopt(copy_dest_addr);
#endif
        } else {
//            int64_t copy_dest_index = vq_rx_to_guest->descs[vq_rx_to_guest->last_used_idx].entry_index;
//            void *xmm01 = get_packet_addr(&pool_guest_addr[copy_dest_index]);
//            auto *xmm02 = get_packet_addr(pool_host_addr[i]);

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
        vq_rx_to_guest->last_used_idx += 1;
        if (VQ_ENYRY_NUM <= vq_rx_to_guest->last_used_idx) {
            vq_rx_to_guest->last_used_idx = 0;
        }
    }

#ifdef SKIP_INDEX_RX
    int skipped_index = 64 / VQ_ENTRY_SIZE;
    for (int i = skipped_index; i < num_fin; i++) {
#else
    for (int i = 0; i < num_fin; i++) {
#endif
        int desc_index = (last_used_idx_shadow + i) % VQ_ENYRY_NUM;
        set_used_flag(&vq_rx_to_guest->descs[desc_index]);
    }

#ifdef SKIP_INDEX_RX
    for (int i = 0; i < skipped_index; i++) {
        int desc_index = (last_used_idx_shadow + i) % VQ_ENYRY_NUM;
        set_used_flag(&vq_rx_to_guest->descs[desc_index]);
    }
#endif
}

void send_guest_to_tx(newvq *vq_guest_to_tx, buf **pool_host_addr, buffer_pool *pool_guest, int num_fin,
                      bool is_stream) {
    for (int i = 0; i < num_fin; i++) {
        wait_used(vq_guest_to_tx, vq_guest_to_tx->last_avail_idx + i);
    }

    for (int i = 0; i < num_fin; i++) {
        PREFETCH_MBUF(pool_host_addr[i]->header.id_addr, pool_host_addr[i]->header.len_addr);
        //PREFETCH_MBUF(pool_host_addr[i]->header.id_addr, pool_host_addr[i]->header.id_addr);
        //PREFETCH_POOL(pool_guest_addr[i]);
    }

    int last_avail_idx_shadow = vq_guest_to_tx->last_avail_idx;
    for (int i = 0; i < num_fin; i++) {
        if (!is_stream) {
#ifndef ZERO_COPY_TX
            memcpy(get_packet_addr(pool_host_addr[i]), get_packet_addr(&pool_guest->buffers[i]), SIZE_PACKET);
            //cldemote(get_packet_addr(pool_host_addr[i]);
            //_mm_clflushopt(get_packet_addr(pool_host_addr[i]);
#endif
        } else {
//            auto *xmm01 = get_packet_addr(pool_host_addr[i];
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
        vq_guest_to_tx->last_avail_idx += 1;
        if (VQ_ENYRY_NUM <= vq_guest_to_tx->last_avail_idx) {
            vq_guest_to_tx->last_avail_idx = 0;
        }
    }

#if MBUF_HEADER_SIZE > 0
    for (int i = 0; i < num_fin; i++) {
        //if(*(int*)(pool_host_addr[i]->header.id_addr) == 999999) {
        //    exit(1);
        //}
        //memset(pool_host_addr[i]->header.id_addr, i, 4);
        PROC_MBUF_HEADER(pool_host_addr[i])
    }
#endif

#ifdef SKIP_INDEX_TX
    int skipped_index = 64 / VQ_ENTRY_SIZE;
    for (int i = skipped_index; i < num_fin; i++) {
#else
    for (int i = 0; i < num_fin; i++) {
#endif
        int desc_index = (last_avail_idx_shadow + i) % VQ_ENYRY_NUM;
        set_avail_flag(&vq_guest_to_tx->descs[desc_index]);
    }

#ifdef SKIP_INDEX_TX
    for (int i = 0; i < skipped_index; i++) {
        int desc_index = (last_avail_idx_shadow + i) % VQ_ENYRY_NUM;
        set_avail_flag(&vq_guest_to_tx->descs[desc_index]);
    }
#endif
}

void guest_recv_process(newvq *vq_rx_to_guest, newvq *vq_guest_to_tx, buffer_pool *pool, int num_fin) {
    for (int i = 0; i < num_fin; i++) {
        wait_used(vq_rx_to_guest, vq_rx_to_guest->last_avail_idx + i);
    }

    for (int i = 0; i < num_fin; i++) {
        int64_t index = vq_rx_to_guest->descs[vq_rx_to_guest->last_avail_idx + i].entry_index;
        PREFETCH_MBUF(pool->buffers[index].header.id_addr, pool->buffers[index].header.len_addr)
        //PREFETCH_MBUF(pool_guest_addr[index].header.id_addr, pool_guest_addr[index].header.id_addr)
    }

    int64_t id[num_fin];
    int last_avail_idx_shadow = vq_rx_to_guest->last_avail_idx;
    int last_used_idx_shadow = vq_guest_to_tx->last_used_idx;

    for (int i = 0; i < num_fin; i++) {
        id[i] = vq_rx_to_guest->descs[vq_rx_to_guest->last_avail_idx].entry_index;
        buf *packet_buffer = &pool->buffers[id[i]];
        //if(((packet*)(packet_buffer->addr))->packet_len == 999999) {
        //    exit(1);
        //}
        //memset(&((packet*)(packet_buffer->addr))->packet_len, i, 4);

#if MBUF_HEADER_SIZE > 0
        PROC_MBUF_HEADER(packet_buffer);
        set_len(packet_buffer, SIZE_PACKET);
        //if(*(int*)(packet_buffer->header.id_addr) == 999999) {
        //    exit(1);
        //}
        //memset(packet_buffer->header.id_addr, i, 4);
#endif

        vq_rx_to_guest->last_avail_idx += 1;
        if (VQ_ENYRY_NUM <= vq_rx_to_guest->last_avail_idx) {
            vq_rx_to_guest->last_avail_idx = 0;
        }

        wait_avail(vq_guest_to_tx, vq_guest_to_tx->last_used_idx);
        set_len(&pool->buffers[vq_guest_to_tx->descs[vq_guest_to_tx->last_used_idx].entry_index], -1);
        vq_guest_to_tx->last_used_idx += 1;
        if (VQ_ENYRY_NUM <= vq_guest_to_tx->last_used_idx) {
            vq_guest_to_tx->last_used_idx = 0;
        }
    }

#ifdef SKIP_INDEX_NF
    int skipped_index = 64 / VQ_ENTRY_SIZE;
    for (int i = skipped_index; i < num_fin; i++) {
#else
    for (int i = 0; i < num_fin; i++) {
#endif
        int tx_desc_entry = (last_used_idx_shadow + i) % VQ_ENYRY_NUM;
        add_to_cache(pool, &pool->buffers[vq_guest_to_tx->descs[tx_desc_entry].entry_index]);
        vq_guest_to_tx->descs[tx_desc_entry].entry_index = id[i];
        set_used_flag(&vq_guest_to_tx->descs[tx_desc_entry]);

        int rx_desc_entry = (last_avail_idx_shadow + i) % VQ_ENYRY_NUM;
#ifdef RANDOM_NF
        int next_buffer_index = ((vq_rx_to_guest->last_pool_idx + i) % POOL_ENTRY_NUM) / 32 * 32 + ids[i];
#else
        auto next_buffer_index = static_cast<int64_t>(get_buffer_index(pool, get_buffer(pool)));
#endif
        assert(get_len(&pool->buffers[next_buffer_index]) == -1);
        vq_rx_to_guest->descs[rx_desc_entry].entry_index = next_buffer_index;
        set_avail_flag(&vq_rx_to_guest->descs[rx_desc_entry]);
    }

#ifdef SKIP_INDEX_NF
    for (int i = 0; i < skipped_index; i++) {
        int tx_desc_entry = (last_used_idx_shadow + i) % VQ_ENYRY_NUM;
        set_used_flag(&vq_guest_to_tx->descs[tx_desc_entry]);
        vq_guest_to_tx->descs[tx_desc_entry].entry_index = id[i];

        int rx_desc_entry = (last_avail_idx_shadow + i) % VQ_ENYRY_NUM;
        set_avail_flag(&vq_rx_to_guest->descs[rx_desc_entry]);
    }
#endif
}

