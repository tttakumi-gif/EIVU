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

void wait_push(vq *v, int prev_idx) {
    // ディスクリプタが空くまでwait
    while ((*(volatile int16_t *) &v->descs[prev_idx].flags & USED_FLAG) == 0) {
        do_none();
    }
}

void wait_pull(vq *v, int prev_idx) {
    // ディスクリプタにバッファが割り当てられるまでwait
    while ((*(volatile int16_t *) &v->descs[prev_idx].flags & AVAIL_FLAG) == 0) {
        do_none();
    }
}

void init_descs(vq *v) {
    memset(v->descs, 0, sizeof(desc) * VQ_ENYRY_NUM);
    for (int i = 0; i < VQ_ENYRY_NUM; i++) {
        v->descs[i].id = i;
        v->descs[i].flags = v->descs[i].flags | USED_FLAG;
    }
}

void init_ring(vq *v) {
    v->last_pool_idx = 0;
    v->size = VQ_ENYRY_NUM;
    v->last_avail_idx = 0;
    v->last_used_idx = 0;
    init_descs(v);
}

char ids[32] = {21, 10, 24, 22, 15, 31, 0, 30, 14, 1, 11, 2, 13, 23, 12, 3, 25, 17, 4, 16, 26, 19, 5, 28, 20, 6, 27, 7,
                8, 18, 29, 9};

void send_rx_to_guest(vq *vq_rx_to_guest, buf **pool_host_addr, void **pool_guest_addr, int num_fin,
                      bool is_stream) {
    for (int i = 0; i < num_fin; i++) {
        PREFETCH_MBUF(pool_host_addr[i]->header.id_addr, pool_host_addr[i]->header.len_addr);
        //PREFETCH_MBUF(pool_host_addr[i]->header.id_addr, pool_host_addr[i]->header.id_addr);
    //    PREFETCH_POOL(pool_guest_addr[i]);
    }

    for (int i = 0; i < num_fin; i++) {
        wait_push(vq_rx_to_guest, vq_rx_to_guest->last_avail_idx + i);
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

    int last_avail_idx_shadow = vq_rx_to_guest->last_avail_idx;
    for (int i = 0; i < num_fin; i++) {
        if (!is_stream) {
        //if (false) {
#ifndef ZERO_COPY_RX
            memcpy(pool_guest_addr[i], (void *) pool_host_addr[i]->addr, SIZE_PACKET);
            //cldemote(pool_guest_addr[i]);
            //_mm_clflushopt(pool_guest_addr[i]);
#endif
        } else {
            void *xmm01 = pool_guest_addr[i];
            packet *xmm02 = (packet *) pool_host_addr[i]->addr;

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
#ifdef STRIDE_VQ
        vq_rx_to_guest->last_avail_idx += NUM_VQ_STRIDE;
        if(VQ_ENYRY_NUM <= vq_rx_to_guest->last_avail_idx) {
                vq_rx_to_guest->last_avail_idx = vq_rx_to_guest->last_avail_idx % NUM_VQ_STRIDE + 1;
        }
#else
        vq_rx_to_guest->last_avail_idx += 1;
        if (VQ_ENYRY_NUM <= vq_rx_to_guest->last_avail_idx) {
            vq_rx_to_guest->last_avail_idx = 0;
        }
#endif
    }

#ifdef SKIP_INDEX
    int skipped_index = 64 / VQ_ENTRY_SIZE;
    int a = last_avail_idx_shadow;
    last_avail_idx_shadow = (last_avail_idx_shadow + skipped_index) % VQ_ENYRY_NUM;
    for (int i = skipped_index; i < num_fin; i++) {
#else
        for(int i = 0; i < num_fin; i++) {
#endif
#ifdef RANDOM
        set_param(&vq_rx_to_guest->descs[last_avail_idx_shadow], vq_rx_to_guest->last_pool_idx + ids[i]);
#else
        set_param(&vq_rx_to_guest->descs[last_avail_idx_shadow], vq_rx_to_guest->last_pool_idx + i);
#endif

        // index更新
#ifdef STRIDE_VQ
        last_avail_idx_shadow += NUM_VQ_STRIDE;
        if(VQ_ENYRY_NUM <= last_avail_idx_shadow) {
            last_avail_idx_shadow = last_avail_idx_shadow % NUM_VQ_STRIDE + 1;
        }
#else
        last_avail_idx_shadow += 1;
        if (VQ_ENYRY_NUM <= last_avail_idx_shadow) {
            last_avail_idx_shadow = 0;
        }
#endif
    }

#ifdef SKIP_INDEX
    for (int i = 0; i < skipped_index; i++) {
#ifdef RANDOM
        set_param(&vq_rx_to_guest->descs[a + i], vq_rx_to_guest->last_pool_idx + ids[i]);
#else
        set_param(&vq_rx_to_guest->descs[a + i], vq_rx_to_guest->last_pool_idx + i);
#endif
    }
#endif
}

void send_guest_to_tx(vq *vq_guest_to_tx, buf **pool_host_addr, void **pool_guest_addr, int num_fin,
                      bool is_stream) {
    for (int i = 0; i < num_fin; i++) {
        wait_pull(vq_guest_to_tx, vq_guest_to_tx->last_used_idx + i);
    }

    for (int i = 0; i < num_fin; i++) {
        PREFETCH_MBUF(pool_host_addr[i]->header.id_addr, pool_host_addr[i]->header.len_addr);
        //PREFETCH_MBUF(pool_host_addr[i]->header.id_addr, pool_host_addr[i]->header.id_addr);
       //PREFETCH_POOL(pool_guest_addr[i]);
    }

    int last_used_idx_shadow = vq_guest_to_tx->last_used_idx;
    for (int i = 0; i < num_fin; i++) {
        if (!is_stream) {
        //if (false) {
#ifndef ZERO_COPY_TX
            memcpy((void *) (pool_host_addr[i]->addr), pool_guest_addr[i], SIZE_PACKET);
            //cldemote(pool_host_addr[i]->addr);
            //_mm_clflushopt(pool_host_addr[i]->addr);
#endif
        } else {
            auto *xmm01 = (packet *) pool_host_addr[i]->addr;
            auto *xmm02 = pool_guest_addr[i];
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
#ifdef STRIDE_VQ
        vq_guest_to_tx->last_used_idx += NUM_VQ_STRIDE;
        if(VQ_ENYRY_NUM <= vq_guest_to_tx->last_used_idx) {
                vq_guest_to_tx->last_used_idx = vq_guest_to_tx->last_used_idx % NUM_VQ_STRIDE + 1;
        }
#else
        vq_guest_to_tx->last_used_idx += 1;
        if (VQ_ENYRY_NUM <= vq_guest_to_tx->last_used_idx) {
            vq_guest_to_tx->last_used_idx = 0;
        }
#endif
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

#ifdef SKIP_INDEX
    int skipped_index = 64 / VQ_ENTRY_SIZE;
    int a = last_used_idx_shadow;
    last_used_idx_shadow = (last_used_idx_shadow + skipped_index) % VQ_ENYRY_NUM;
    for (int i = skipped_index; i < num_fin; i++) {
#else
        for(int i = 0; i < num_fin; i++) {
#endif
        // パケットの取得, ディスクリプタの紐づけ解除
        delete_info(&vq_guest_to_tx->descs[last_used_idx_shadow]);

        // index更新
#ifdef STRIDE_VQ
        last_used_idx_shadow += NUM_VQ_STRIDE;
        if(VQ_ENYRY_NUM <= last_used_idx_shadow) {
            last_used_idx_shadow = last_used_idx_shadow % NUM_VQ_STRIDE + 1;
        }
#else
        last_used_idx_shadow += 1;
        if (VQ_ENYRY_NUM <= last_used_idx_shadow) {
            last_used_idx_shadow = 0;
        }
#endif
    }

#ifdef SKIP_INDEX
    for (int i = 0; i < skipped_index; i++) {
        delete_info(&vq_guest_to_tx->descs[a + i]);
    }
#endif
}

void guest_recv_process(vq *vq_rx_to_guest, vq *vq_guest_to_tx, buf *pool_guest_addr, int num_fin) {
    for (int i = 0; i < num_fin; i++) {
        wait_pull(vq_rx_to_guest, vq_rx_to_guest->last_used_idx + i);
    }

    for (int i = 0; i < num_fin; i++) {
        int index = vq_rx_to_guest->descs[vq_rx_to_guest->last_used_idx + i].entry_index;
        PREFETCH_MBUF(pool_guest_addr[index].header.id_addr, pool_guest_addr[index].header.len_addr)
        //PREFETCH_MBUF(pool_guest_addr[index].header.id_addr, pool_guest_addr[index].header.id_addr)
    }

    int id[num_fin];
    int last_used_idx_shadow = vq_rx_to_guest->last_used_idx;
    int last_avail_idx_shadow = vq_guest_to_tx->last_avail_idx;

    for (int i = 0; i < num_fin; i++) {
        id[i] = vq_rx_to_guest->descs[vq_rx_to_guest->last_used_idx].entry_index;
        buf *packet_buffer = &pool_guest_addr[id[i]];
        //if(((packet*)(packet_buffer->addr))->packet_len == 999999) {
        //    exit(1);
        //}
        //memset(&((packet*)(packet_buffer->addr))->packet_len, i, 4);

#if MBUF_HEADER_SIZE > 0
        PROC_MBUF_HEADER(packet_buffer);
        //if(*(int*)(packet_buffer->header.id_addr) == 999999) {
        //    exit(1);
        //}
        //memset(packet_buffer->header.id_addr, i, 4);
#endif

#ifdef STRIDE_VQ
        vq_rx_to_guest->last_used_idx += NUM_VQ_STRIDE;
        if(VQ_ENYRY_NUM <= vq_rx_to_guest->last_used_idx) {
            vq_rx_to_guest->last_used_idx = vq_rx_to_guest->last_used_idx % NUM_VQ_STRIDE + 1;
        }
#else
        vq_rx_to_guest->last_used_idx += 1;
        if (VQ_ENYRY_NUM <= vq_rx_to_guest->last_used_idx) {
            vq_rx_to_guest->last_used_idx = 0;
        }
#endif

        wait_push(vq_guest_to_tx, vq_guest_to_tx->last_avail_idx);
#ifdef STRIDE_VQ
        vq_guest_to_tx->last_avail_idx += NUM_VQ_STRIDE;
        if(VQ_ENYRY_NUM <= vq_guest_to_tx->last_avail_idx) {
            vq_guest_to_tx->last_avail_idx = vq_guest_to_tx->last_avail_idx % NUM_VQ_STRIDE + 1;
        }
#else
        vq_guest_to_tx->last_avail_idx += 1;
        if (VQ_ENYRY_NUM <= vq_guest_to_tx->last_avail_idx) {
            vq_guest_to_tx->last_avail_idx = 0;
        }
#endif
    }

#ifdef SKIP_INDEX
    int skipped_index = 64 / VQ_ENTRY_SIZE;
    int a = last_avail_idx_shadow;
    int b = last_used_idx_shadow;
    last_avail_idx_shadow = (last_avail_idx_shadow + skipped_index) % VQ_ENYRY_NUM;
    last_used_idx_shadow = (last_used_idx_shadow + skipped_index) % VQ_ENYRY_NUM;
    for (int i = skipped_index; i < num_fin; i++) {
#else
        for(int i = 0; i < num_fin; i++) {
#endif
        set_param(&vq_guest_to_tx->descs[last_avail_idx_shadow], id[i]);
#ifdef STRIDE_VQ
        last_avail_idx_shadow += NUM_VQ_STRIDE;
        if(VQ_ENYRY_NUM <= last_avail_idx_shadow) {
            last_avail_idx_shadow = last_avail_idx_shadow % NUM_VQ_STRIDE + 1;
        }
#else
        last_avail_idx_shadow += 1;
        if (VQ_ENYRY_NUM <= last_avail_idx_shadow) {
            last_avail_idx_shadow = 0;
        }
#endif

        delete_info(&vq_rx_to_guest->descs[last_used_idx_shadow]);
#ifdef STRIDE_VQ
        last_used_idx_shadow += NUM_VQ_STRIDE;
        if(VQ_ENYRY_NUM <= last_used_idx_shadow) {
            last_used_idx_shadow = last_used_idx_shadow % NUM_VQ_STRIDE + 1;
        }
#else
        last_used_idx_shadow += 1;
        if (VQ_ENYRY_NUM <= last_used_idx_shadow) {
            last_used_idx_shadow = 0;
        }
#endif
    }

#ifdef SKIP_INDEX
    for (int i = 0; i < skipped_index; i++) {
        set_param(&vq_guest_to_tx->descs[a + i], id[i]);
        delete_info(&vq_rx_to_guest->descs[b + i]);
    }
#endif
}

