#pragma once

void delete_info(desc *d) {
    d->entry_index = -1;
    d->flags = d->flags | USED_FLAG;
    d->flags = d->flags & ~AVAIL_FLAG;
}

void set_param(desc *d, int this_id) {
    d->entry_index = this_id;
    d->flags = d->flags & ~USED_FLAG;
    d->flags = d->flags | AVAIL_FLAG;
}

void wait_push(ring *r, int prev_idx) {
    // ディスクリプタが空くまでwait
    while ((*(volatile int16_t *) &r->descs[prev_idx].flags & USED_FLAG) == 0) {
        do_none();
    }
}

void wait_pull(ring *r, int prev_idx) {
    // ディスクリプタにバッファが割り当てられるまでwait
    while ((*(volatile int16_t *) &r->descs[prev_idx].flags & AVAIL_FLAG) == 0) {
        do_none();
    }
}

void init_descs(ring *r) {
    memset(r->descs, 0, sizeof(desc) * SIZE_RING);
    for (int i = 0; i < SIZE_RING; i++) {
        r->descs[i].id = i;
        r->descs[i].flags = r->descs[i].flags | USED_FLAG;
    }
}

void init_ring(ring *r) {
    r->pool_index = 0;
    r->size = SIZE_RING;
    r->last_avail_idx = 0;
    r->last_used_idx = 0;
    init_descs(r);
}

#ifdef RANDOM
char ids[32] = {21, 10, 24, 22, 15, 31, 0, 30, 14, 1, 11, 2, 13, 23, 12, 3, 25, 17, 4, 16, 26, 19, 5, 28, 20, 6, 27, 7,
                8, 18, 29, 9};
#endif

void send_rx_to_guest(ring *r, buf **pool_host_addr, buf *pool_guest_addr, int num_fin, bool is_stream) {
    int last_avail_idx_shadow = r->last_avail_idx;

    if (!is_stream) {
        for (int i = 0; i < num_fin; i++) {
#ifdef RANDOM
            buf *packet_buffer_dest = &pool_guest_addr[r->pool_index + ids[i]];
#else
            buf* packet_buffer_dest = &pool_guest_addr[r->pool_index + i];
#endif
            packet *packet_addr_dest = get_packet_addr(packet_buffer_dest);

            // ディスクリプタとパケットプールが確保できるまでwait
#ifndef SKIP_CLT
            wait_push(r, r->last_avail_idx);
#endif

#ifndef ZERO_COPY
            memcpy((void *) packet_addr_dest, (void *) pool_host_addr[i]->addr, SIZE_PACKET);
#endif
//			for(int j = 0; j < NUM_LOOP2; j++) {
//				__asm__("cldemote (%0)" :: "r" ((int64_t*)&packet_addr_dest + j));
//				__asm__("cldemote (%0)" :: "r" ((int64_t*)&pool_host_addr[i] + j));
//			}

#if HEADER_SIZE > 0
            set_id(pool_host_addr[i], (get_id(pool_host_addr[i]) + 1) & 2047);
            set_len(pool_host_addr[i], 1);
            //if(get_id(pool_host_addr[i]) < 0 || get_len(pool_host_addr[i]) < 0) {
            //	exit(1);
            //}
#endif

            // index更新
#ifdef STRIDE_VQ
            r->last_avail_idx += NUM_VQ_STRIDE;
            if(SIZE_RING <= r->last_avail_idx) {
                r->last_avail_idx = r->last_avail_idx % NUM_VQ_STRIDE + 1;
            }
#else
            r->last_avail_idx += 1;
            if (SIZE_RING <= r->last_avail_idx) {
                r->last_avail_idx = 0;
            }
#endif
        }
    } else {
        for (int i = 0; i < num_fin; i++) {
            // ディスクリプタとパケットプールが確保できるまでwait
#ifdef RANDOM
            buf *packet_buffer_dest = &pool_guest_addr[r->pool_index + ids[i]];
#else
            buf* packet_buffer_dest = &pool_guest_addr[r->pool_index + i];
#endif
            packet *xmm01 = get_packet_addr(packet_buffer_dest);
            packet *xmm02 = (packet *) pool_host_addr[i]->addr;

#ifndef SKIP_CLT
            wait_push(r, r->last_avail_idx);
#endif

#if HEADER_SIZE > 0
            set_id(pool_host_addr[i], (get_id(pool_host_addr[i]) + 1) & 2047);
            set_len(pool_host_addr[i], 1);
#endif

            for (int j = 0; j < NUM_LOOP; j++) {
                if (!IS_PSMALL) {
                    _mm256_stream_si256((__m256i *) xmm01 + j, _mm256_stream_load_si256((__m256i *) xmm02 + j));
                } else {
                    _mm_stream_si128((__m128i *) xmm01 + j, _mm_stream_load_si128((__m128i *) xmm02 + j));
                }
            }

            // index更新
#ifdef STRIDE_VQ
            r->last_avail_idx += NUM_VQ_STRIDE;
            if(SIZE_RING <= r->last_avail_idx) {
                r->last_avail_idx = r->last_avail_idx % NUM_VQ_STRIDE + 1;
            }
#else
            r->last_avail_idx += 1;
            if (SIZE_RING <= r->last_avail_idx) {
                r->last_avail_idx = 0;
            }
#endif
        }
    }

#ifdef SKIP_INDEX
    int skipped_index = 64 / VQENTRY_SIZE;
    int a = last_avail_idx_shadow;
    last_avail_idx_shadow = (last_avail_idx_shadow + skipped_index) % SIZE_RING;
    for (int i = skipped_index; i < num_fin; i++) {
#else
        for(int i = 0; i < num_fin; i++) {
#endif
#ifdef RANDOM
        set_param(&r->descs[last_avail_idx_shadow], r->pool_index + ids[i]);
#else
        set_param(&r->descs[last_avail_idx_shadow], r->pool_index + i);
#endif

        // index更新
#ifdef STRIDE_VQ
        last_avail_idx_shadow += NUM_VQ_STRIDE;
        if(SIZE_RING <= last_avail_idx_shadow) {
            last_avail_idx_shadow = last_avail_idx_shadow % NUM_VQ_STRIDE + 1;
        }
#else
        last_avail_idx_shadow += 1;
        if (SIZE_RING <= last_avail_idx_shadow) {
            last_avail_idx_shadow = 0;
        }
#endif
    }

#ifdef SKIP_INDEX
    for (int i = 0; i < skipped_index; i++) {
        set_param(&r->descs[a + i], r->pool_index + ids[i]);
    }
#endif

    // 共有変数に反映
    r->pool_index = (r->pool_index + num_fin) % SIZE_POOL;
}

void send_guest_to_tx(ring *r, buf **pool_host_addr, buf *pool_guest_addr, int num_fin, bool is_stream) {
    int last_used_idx_shadow = r->last_used_idx;
    if (!is_stream) {
        for (int i = 0; i < num_fin; i++) {
            // ディスクリプタにバッファが割り当てられるまでwait
            wait_pull(r, r->last_used_idx);

            buf *packet_buffer_src = &pool_guest_addr[r->descs[r->last_used_idx].entry_index];
#ifndef AVOID_TX
            packet *packet_addr_src = get_packet_addr(packet_buffer_src);
            memcpy((void *) (pool_host_addr[i]->addr), (void *) packet_addr_src, SIZE_PACKET);
#endif

#if HEADER_SIZE > 0
            set_id(pool_host_addr[i], (get_id(pool_host_addr[i]) + 1) & 2047);
            set_len(pool_host_addr[i], 0);
            //if(get_id(pool_host_addr[i]) < 0 || get_len(pool_host_addr[i]) < 0) {
            //	exit(1);
            //}
#endif

            // index更新
#ifdef STRIDE_VQ
            r->last_used_idx += NUM_VQ_STRIDE;
            if(SIZE_RING <= r->last_used_idx) {
                r->last_used_idx = r->last_used_idx % NUM_VQ_STRIDE + 1;
            }
#else
            r->last_used_idx += 1;
            if (SIZE_RING <= r->last_used_idx) {
                r->last_used_idx = 0;
            }
#endif
        }
    } else {
        for (int i = 0; i < num_fin; i++) {
            // ディスクリプタにバッファが割り当てられるまでwait
            wait_pull(r, r->last_used_idx);
            buf *packet_buffer_src = &pool_guest_addr[r->descs[r->last_used_idx].entry_index];
            packet *packet_addr_src = get_packet_addr(packet_buffer_src);

            auto *xmm01 = (packet *) pool_host_addr[i]->addr;
            auto *xmm02 = packet_addr_src;
            for (int j = 0; j < NUM_LOOP; j++) {
                if (!IS_PSMALL) {
                    //_mm256_stream_si256((__m256i*)xmm01 + j, _mm256_load_si256((__m256i*)xmm02 + j));
                    _mm256_stream_si256((__m256i *) xmm01 + j, _mm256_stream_load_si256((__m256i *) xmm02 + j));
                    //_mm256_store_si256((__m256i*)xmm01 + j, _mm256_stream_load_si256((__m256i*)xmm02 + j));
                } else {
                    //_mm_stream_si128((__m128i*)xmm01 + j, _mm_load_si128((__m128i*)xmm02 + j));
                    _mm_store_si128((__m128i *) xmm01 + j, _mm_stream_load_si128((__m128i *) xmm02 + j));
                }
            }

#if HEADER_SIZE > 0
            set_id(pool_host_addr[i], (get_id(pool_host_addr[i]) + 1) & 2047);
            set_len(pool_host_addr[i], 0);
#endif

            // index更新
#ifdef STRIDE_VQ
            r->last_used_idx += NUM_VQ_STRIDE;
            if(SIZE_RING <= r->last_used_idx) {
                r->last_used_idx = r->last_used_idx % NUM_VQ_STRIDE + 1;
            }
#else
            r->last_used_idx += 1;
            if (SIZE_RING <= r->last_used_idx) {
                r->last_used_idx = 0;
            }
#endif
        }
    }

#ifdef SKIP_INDEX
    int skipped_index = 64 / VQENTRY_SIZE;
    int a = last_used_idx_shadow;
    last_used_idx_shadow = (last_used_idx_shadow + skipped_index) % SIZE_RING;
    for (int i = skipped_index; i < num_fin; i++) {
#else
        for(int i = 0; i < num_fin; i++) {
#endif
        // パケットの取得, ディスクリプタの紐づけ解除
        delete_info(&r->descs[last_used_idx_shadow]);

        // index更新
#ifdef STRIDE_VQ
        last_used_idx_shadow += NUM_VQ_STRIDE;
        if(SIZE_RING <= last_used_idx_shadow) {
            last_used_idx_shadow = last_used_idx_shadow % NUM_VQ_STRIDE + 1;
        }
#else
        last_used_idx_shadow += 1;
        if (SIZE_RING <= last_used_idx_shadow) {
            last_used_idx_shadow = 0;
        }
#endif
    }

#ifdef SKIP_INDEX
    for (int i = 0; i < skipped_index; i++) {
        delete_info(&r->descs[a + i]);
    }
#endif
}

void move_packet(ring *ring_rx_to_guest, ring *ring_guest_to_tx, buf *pool_guest_addr, int num_fin) {
    int id[num_fin];
    int last_used_idx_shadow = ring_rx_to_guest->last_used_idx;
    int last_avail_idx_shadow = ring_guest_to_tx->last_avail_idx;

    for (int i = 0; i < num_fin; i++) {
        wait_pull(ring_rx_to_guest, ring_rx_to_guest->last_used_idx);
        id[i] = ring_rx_to_guest->descs[ring_rx_to_guest->last_used_idx].entry_index;
        buf *packet_buffer = &pool_guest_addr[id[i]];

#ifdef STRIDE_VQ
        ring_rx_to_guest->last_used_idx += NUM_VQ_STRIDE;
        if(SIZE_RING <= ring_rx_to_guest->last_used_idx) {
            ring_rx_to_guest->last_used_idx = ring_rx_to_guest->last_used_idx % NUM_VQ_STRIDE + 1;
        }
#else
        ring_rx_to_guest->last_used_idx += 1;
        if (SIZE_RING <= ring_rx_to_guest->last_used_idx) {
            ring_rx_to_guest->last_used_idx = 0;
        }
#endif

        wait_push(ring_guest_to_tx, ring_guest_to_tx->last_avail_idx);
#ifdef STRIDE_VQ
        ring_guest_to_tx->last_avail_idx += NUM_VQ_STRIDE;
        if(SIZE_RING <= ring_guest_to_tx->last_avail_idx) {
            ring_guest_to_tx->last_avail_idx = ring_guest_to_tx->last_avail_idx % NUM_VQ_STRIDE + 1;
        }
#else
        ring_guest_to_tx->last_avail_idx += 1;
        if (SIZE_RING <= ring_guest_to_tx->last_avail_idx) {
            ring_guest_to_tx->last_avail_idx = 0;
        }
#endif

#if HEADER_SIZE > 0
        set_id(packet_buffer, (get_id(packet_buffer) + 1) & 2047);
        set_len(packet_buffer, 2);
        //if(get_id(packet_buffer) < 0 || get_len(packet_buffer) < 0) {
        //	exit(1);
        //}
        //_mm_clflushopt(packet_buffer->header.id_addr);
        //_mm_clflushopt(packet_buffer->header.len_addr);
#endif

#if !defined(READ_SRV) && !defined(AVOID_SRV) && defined(VERIFICATION)
        packet* p = get_packet_addr(&pool_guest_addr[id[i]]);
        set_verification(p);
        //__asm__("cldemote (%0)" :: "ring_rx_to_guest" (&p->verification));
#endif
    }

#ifdef SKIP_INDEX
    int skipped_index = 64 / VQENTRY_SIZE;
    int a = last_avail_idx_shadow;
    int b = last_used_idx_shadow;
    last_avail_idx_shadow = (last_avail_idx_shadow + skipped_index) % SIZE_RING;
    last_used_idx_shadow = (last_used_idx_shadow + skipped_index) % SIZE_RING;
    for (int i = skipped_index; i < num_fin; i++) {
#else
        for(int i = 0; i < num_fin; i++) {
#endif
        set_param(&ring_guest_to_tx->descs[last_avail_idx_shadow], id[i]);
#ifdef STRIDE_VQ
        last_avail_idx_shadow += NUM_VQ_STRIDE;
        if(SIZE_RING <= last_avail_idx_shadow) {
            last_avail_idx_shadow = last_avail_idx_shadow % NUM_VQ_STRIDE + 1;
        }
#else
        last_avail_idx_shadow += 1;
        if (SIZE_RING <= last_avail_idx_shadow) {
            last_avail_idx_shadow = 0;
        }
#endif

        delete_info(&ring_rx_to_guest->descs[last_used_idx_shadow]);
#ifdef STRIDE_VQ
        last_used_idx_shadow += NUM_VQ_STRIDE;
        if(SIZE_RING <= last_used_idx_shadow) {
            last_used_idx_shadow = last_used_idx_shadow % NUM_VQ_STRIDE + 1;
        }
#else
        last_used_idx_shadow += 1;
        if (SIZE_RING <= last_used_idx_shadow) {
            last_used_idx_shadow = 0;
        }
#endif
    }

#ifdef SKIP_INDEX
    for (int i = 0; i < skipped_index; i++) {
        set_param(&ring_guest_to_tx->descs[a + i], id[i]);
        delete_info(&ring_rx_to_guest->descs[b + i]);
    }
#endif
}

