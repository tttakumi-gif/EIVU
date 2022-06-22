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

#ifdef RANDOM
char ids[32] = {21, 10, 24, 22, 15, 31, 0, 30, 14, 1, 11, 2, 13, 23, 12, 3, 25, 17, 4, 16, 26, 19, 5, 28, 20, 6, 27, 7,
                8, 18, 29, 9};
#endif

void send_rx_to_guest(vq *vq_rx_to_guest, buf **pool_host_addr, void **pool_copy_dest, int num_fin, bool is_stream) {
    int last_avail_idx_shadow = vq_rx_to_guest->last_avail_idx;

    for (int i = 0; i < num_fin; i++) {
        // ディスクリプタとパケットプールが確保できるまでwait
        wait_push(vq_rx_to_guest, vq_rx_to_guest->last_avail_idx);

        memcpy((void *) pool_copy_dest[i], (void *) pool_host_addr[i]->addr, SIZE_PACKET);

	    // update mbuf header
        set_id(pool_host_addr[i], (get_id(pool_host_addr[i]) + 1) & 2047);
        set_len(pool_host_addr[i], 1);

        // index更新
        vq_rx_to_guest->last_avail_idx += 1;
        if (VQ_ENYRY_NUM <= vq_rx_to_guest->last_avail_idx) {
            vq_rx_to_guest->last_avail_idx = 0;
        }
    }

    for(int i = 0; i < num_fin; i++) {
        set_param(&vq_rx_to_guest->descs[last_avail_idx_shadow], vq_rx_to_guest->last_pool_idx + ids[i]);

        // index更新
        last_avail_idx_shadow += 1;
        if (VQ_ENYRY_NUM <= last_avail_idx_shadow) {
            last_avail_idx_shadow = 0;
        }
    }
}

void send_guest_to_tx(vq *vq_guest_to_tx, buf **pool_host_addr, void **pool_copy_src, int num_fin, bool is_stream) {
    int last_used_idx_shadow = vq_guest_to_tx->last_used_idx;

    for (int i = 0; i < num_fin; i++) {
        // ディスクリプタにバッファが割り当てられるまでwait
        wait_pull(vq_guest_to_tx, vq_guest_to_tx->last_used_idx);

        memcpy((void *) (pool_host_addr[i]->addr), pool_copy_src[i], SIZE_PACKET);

		// update mbuf header
        set_id(pool_host_addr[i], (get_id(pool_host_addr[i]) + 1) & 2047);
        set_len(pool_host_addr[i], 0);

        // index更新
        vq_guest_to_tx->last_used_idx += 1;
        if (VQ_ENYRY_NUM <= vq_guest_to_tx->last_used_idx) {
            vq_guest_to_tx->last_used_idx = 0;
        }
    }

    for(int i = 0; i < num_fin; i++) {
        // パケットの取得, ディスクリプタの紐づけ解除
        delete_info(&vq_guest_to_tx->descs[last_used_idx_shadow]);

        // index更新
        last_used_idx_shadow += 1;
        if (VQ_ENYRY_NUM <= last_used_idx_shadow) {
            last_used_idx_shadow = 0;
        }
    }
}

void guest_recv_process(vq *vq_rx_to_guest, vq *vq_guest_to_tx, buf *pool_guest_addr, int num_fin) {
    int id[num_fin];
    int last_used_idx_shadow = vq_rx_to_guest->last_used_idx;
    int last_avail_idx_shadow = vq_guest_to_tx->last_avail_idx;

    for (int i = 0; i < num_fin; i++) {
			  // wait sending from rx
        wait_pull(vq_rx_to_guest, vq_rx_to_guest->last_used_idx);

        id[i] = vq_rx_to_guest->descs[vq_rx_to_guest->last_used_idx].entry_index;
        buf *packet_buffer = &pool_guest_addr[id[i]];

        vq_rx_to_guest->last_used_idx += 1;
        if (VQ_ENYRY_NUM <= vq_rx_to_guest->last_used_idx) {
            vq_rx_to_guest->last_used_idx = 0;
        }

				// wait receiving from tx
        wait_push(vq_guest_to_tx, vq_guest_to_tx->last_avail_idx);

        vq_guest_to_tx->last_avail_idx += 1;
        if (VQ_ENYRY_NUM <= vq_guest_to_tx->last_avail_idx) {
            vq_guest_to_tx->last_avail_idx = 0;
        }

				// update mbuf header
        set_id(packet_buffer, (get_id(packet_buffer) + 1) & 2047);
        set_len(packet_buffer, 2);
    }

    for(int i = 0; i < num_fin; i++) {
			  // update tx vq
        set_param(&vq_guest_to_tx->descs[last_avail_idx_shadow], id[i]);

        last_avail_idx_shadow += 1;
        if (VQ_ENYRY_NUM <= last_avail_idx_shadow) {
            last_avail_idx_shadow = 0;
        }

				// update rx vq
        delete_info(&vq_rx_to_guest->descs[last_used_idx_shadow]);
        last_used_idx_shadow += 1;
        if (VQ_ENYRY_NUM <= last_used_idx_shadow) {
            last_used_idx_shadow = 0;
        }
    }
}

