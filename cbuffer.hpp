#pragma once

void delete_info(desc* d) {
	d->entry_index = -1;
	d->flags = d->flags | USED_FLAG;
	d->flags = d->flags & ~AVAIL_FLAG;
}

void set_param(desc* d, int this_id) {
	d->entry_index = this_id;
	d->flags = d->flags & ~USED_FLAG;
	d->flags = d->flags | AVAIL_FLAG;
}

void wait_push(ring* r, int prev_idx) {
	// ディスクリプタが空くまでwait
	while((*(volatile int16_t*)&r->descs[prev_idx].flags & USED_FLAG) == 0) {
		do_none();
	}
}

void wait_pull(ring* r, int prev_idx) {
	// ディスクリプタにバッファが割り当てられるまでwait
	while((*(volatile int16_t*)&r->descs[prev_idx].flags & AVAIL_FLAG) == 0) {
		do_none();
	}
}

void init_descs(ring* r) {
	memset(r->descs, 0, sizeof(desc) * SIZE_RING);
	for(int i = 0; i < SIZE_RING; i++) {
		r->descs[i].id = i;
		r->descs[i].flags = r->descs[i].flags | USED_FLAG;
	}
}

void init_ring(ring* r) {
	r->pool_index = 0;
	r->size = SIZE_RING;
	r->last_avail_idx = 0;
	r->last_used_idx = 0;
	init_descs(r);
}

#ifdef RANDOM
char ids[32] = {21, 10, 24, 22, 15, 31, 0, 30, 14, 1, 11, 2, 13, 23, 12, 3, 25, 17, 4, 16, 26, 19, 5, 28, 20, 6, 27, 7, 8, 18, 29, 9};
#endif

void ipush(ring* r, packet **parray, buf *pool, int num_fin, bool is_stream) {
	int last_avail_idx_shadow = r->last_avail_idx;

	if(!is_stream) {
		for(int i = 0; i < num_fin; i++) {
#ifdef RANDOM
			buf* buffer = &pool[r->pool_index + ids[i]];
#else
			buf* buffer = &pool[r->pool_index + i];
#endif
			packet* p = get_packet_addr(buffer);
			
			// ディスクリプタとパケットプールが確保できるまでwait
#if defined(AVOID_CLT) || defined(SKIP_CLT)
			wait_push(r, r->last_avail_idx);
#else
			wait_push(r, r->last_avail_idx);
#endif

#if HEADER_SIZE > 0
			// パケットの紐づけ
			//volatile int id = get_id(buffer);
			//volatile int len = get_len(buffer);
			while(*(volatile int*)buffer->header.len_addr > 0) {
				do_none();
			}
			set_id(buffer, (get_id(buffer) + 1) & 2047);
			set_len(buffer, 1);
			//_mm_clflushopt(buffer->header.id_addr);
			//_mm_clflushopt(buffer->header.len_addr);
#endif

#ifndef ZERO_COPY
			memcpy((void*)p, (void*)parray[i], SIZE_PACKET);
#endif
//			for(int j = 0; j < NUM_LOOP2; j++) {
//				__asm__("cldemote (%0)" :: "r" ((int64_t*)&p + j));
//				__asm__("cldemote (%0)" :: "r" ((int64_t*)&parray[i] + j));
//			}

			// index更新
#ifdef STRIDE_VQ
			r->last_avail_idx += 4;
			if(SIZE_RING <= r->last_avail_idx) {
				r->last_avail_idx = r->last_avail_idx % 4 + 1;
			}
#else
			r->last_avail_idx += 1;
			if(SIZE_RING <= r->last_avail_idx) {
				r->last_avail_idx = 0;
			}
#endif
		}
	}
	else {
		for(int i = 0; i < num_fin; i++) {
			// ディスクリプタとパケットプールが確保できるまでwait
#ifdef RANDOM
			buf* buffer = &pool[r->pool_index + ids[i]];
#else
			buf* buffer = &pool[r->pool_index + i];
#endif
			packet* xmm01 = get_packet_addr(buffer);
			packet* xmm02 = parray[i];

#if defined(AVOID_CLT) || defined(SKIP_CLT)
			wait_push(r, r->last_avail_idx);
#else
			wait_push(r, r->last_avail_idx);
#endif

#if HEADER_SIZE > 0
			// パケットの紐づけ
			while(*(volatile int32_t*)buffer->header.len_addr > 0) {
				do_none();
			}
			set_id(buffer, (get_id(buffer) + 1) & 2047);
			set_len(buffer, 1);
#endif

			for(int j = 0; j < NUM_LOOP; j++) {
				if(!IS_PSMALL) {
					_mm256_stream_si256((__m256i*)xmm01 + j, _mm256_stream_load_si256((__m256i*)xmm02 + j));
				}
				else {
					_mm_stream_si128((__m128i*)xmm01 + j, _mm_stream_load_si128((__m128i*)xmm02 + j));
				}
			}

			// index更新
#ifdef STRIDE_VQ
			r->last_avail_idx += 4;
			if(SIZE_RING <= r->last_avail_idx) {
				r->last_avail_idx = r->last_avail_idx % 4 + 1;
			}
#else
			r->last_avail_idx += 1;
			if(SIZE_RING <= r->last_avail_idx) {
				r->last_avail_idx = 0;
			}
#endif
		}
	}

#ifdef SKIP_INDEX
	int skipped_index = 64 / VQENTRY_SIZE;
	int a = last_avail_idx_shadow;
	last_avail_idx_shadow = (last_avail_idx_shadow + skipped_index) % SIZE_RING;
	for(int i = skipped_index; i < num_fin; i++) {
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
		last_avail_idx_shadow += 4;
		if(SIZE_RING <= last_avail_idx_shadow) {
			last_avail_idx_shadow = last_avail_idx_shadow % 4 + 1;
		}
#else
		last_avail_idx_shadow += 1;
		if(SIZE_RING <= last_avail_idx_shadow) {
			last_avail_idx_shadow = 0;
		}
#endif
	}

#ifdef SKIP_INDEX
	for(int i = 0; i < skipped_index; i++) {
		set_param(&r->descs[a + i], r->pool_index + ids[i]);
	}
#endif

	// 共有変数に反映
	r->pool_index = (r->pool_index + num_fin) % SIZE_POOL;
}

void pull(ring* r, packet* parray[], buf *pool, int num_fin, bool is_stream) {
	int last_used_idx_shadow = r->last_used_idx;
	if(!is_stream) {
		for(int i = 0; i < num_fin; i++) {
			// ディスクリプタにバッファが割り当てられるまでwait
			wait_pull(r, r->last_used_idx);

			buf* buffer = &pool[r->descs[r->last_used_idx].entry_index];
#ifndef AVOID_CLT
			packet* p = get_packet_addr(buffer);
			memcpy((void*)(parray[i]), (void*)p, SIZE_PACKET);
#endif

#if HEADER_SIZE > 0
			//volatile int id = get_id(buffer);
			//volatile int len = get_len(buffer);
			set_id(buffer, (get_id(buffer) + 1) & 2047);
			set_len(buffer, 0);
			//_mm_clflushopt(buffer->header.id_addr);
			//_mm_clflushopt(buffer->header.len_addr);
#endif

			// index更新
#ifdef STRIDE_VQ
			r->last_used_idx += 4;
			if(SIZE_RING <= r->last_used_idx) {
				r->last_used_idx = r->last_used_idx % 4 + 1;
			}
#else
			r->last_used_idx += 1;
			if(SIZE_RING <= r->last_used_idx) {
				r->last_used_idx = 0;
			}
#endif
		}
	}
	else {
		for(int i = 0; i < num_fin; i++) {
			// ディスクリプタにバッファが割り当てられるまでwait
			wait_pull(r, r->last_used_idx);
			buf* buffer = &pool[r->descs[r->last_used_idx].entry_index];
			packet *p = get_packet_addr(buffer);

			auto* xmm01 = parray[i];
			auto* xmm02 = p;
			for(int j = 0; j < NUM_LOOP; j++) {
				if(!IS_PSMALL) {
					//_mm256_stream_si256((__m256i*)xmm01 + j, _mm256_load_si256((__m256i*)xmm02 + j));
					_mm256_stream_si256((__m256i*)xmm01 + j, _mm256_stream_load_si256((__m256i*)xmm02 + j));
					//_mm256_store_si256((__m256i*)xmm01 + j, _mm256_stream_load_si256((__m256i*)xmm02 + j));
				}
				else {
					//_mm_stream_si128((__m128i*)xmm01 + j, _mm_load_si128((__m128i*)xmm02 + j));
					_mm_store_si128((__m128i*)xmm01 + j, _mm_stream_load_si128((__m128i*)xmm02 + j));
				}
			}

#if HEADER_SIZE > 0
			set_id(buffer, (get_id(buffer) + 1) & 2047);
			set_len(buffer, 0);
#endif

			// index更新
#ifdef STRIDE_VQ
			r->last_used_idx += 4;
			if(SIZE_RING <= r->last_used_idx) {
				r->last_used_idx = r->last_used_idx % 4 + 1;
			}
#else
			r->last_used_idx += 1;
			if(SIZE_RING <= r->last_used_idx) {
				r->last_used_idx = 0;
			}
#endif
		}
	}

#ifdef SKIP_INDEX
	int skipped_index = 64 / VQENTRY_SIZE;
	int a = last_used_idx_shadow;
	last_used_idx_shadow = (last_used_idx_shadow + skipped_index) % SIZE_RING;
	for(int i = skipped_index; i < num_fin; i++) {
#else
	for(int i = 0; i < num_fin; i++) {
#endif
		// パケットの取得, ディスクリプタの紐づけ解除
		delete_info(&r->descs[last_used_idx_shadow]);

		// index更新
#ifdef STRIDE_VQ
		last_used_idx_shadow += 4;
		if(SIZE_RING <= last_used_idx_shadow) {
			last_used_idx_shadow = last_used_idx_shadow % 4 + 1;
		}
#else
		last_used_idx_shadow += 1;
		if(SIZE_RING <= last_used_idx_shadow) {
			last_used_idx_shadow = 0;
		}
#endif
	}

#ifdef SKIP_INDEX
	for(int i = 0; i < skipped_index; i++) {
		delete_info(&r->descs[a + i]);
	}
#endif
}

void move_packet(ring* r, ring* ring_pair, buf *pool, int num_fin) {
	int id[num_fin];
	int last_used_idx_shadow = r->last_used_idx;
	int last_avail_idx_shadow = ring_pair->last_avail_idx; 

	for(int i = 0; i < num_fin; i++) {
		wait_pull(r, r->last_used_idx);
		id[i] = r->descs[r->last_used_idx].entry_index;
		buf* buffer = &pool[id[i]];

#ifdef STRIDE_VQ
		r->last_used_idx += 4;
		if(SIZE_RING <= r->last_used_idx) {
			r->last_used_idx = r->last_used_idx % 4 + 1;
		}
#else
		r->last_used_idx += 1;
		if(SIZE_RING <= r->last_used_idx) {
			r->last_used_idx = 0;
		}
#endif

		wait_push(ring_pair, ring_pair->last_avail_idx);
#ifdef STRIDE_VQ
		ring_pair->last_avail_idx += 4;
		if(SIZE_RING <= ring_pair->last_avail_idx) {
			ring_pair->last_avail_idx = ring_pair->last_avail_idx % 4 + 1;
		}
#else
		ring_pair->last_avail_idx += 1;
		if(SIZE_RING <= ring_pair->last_avail_idx) {
			ring_pair->last_avail_idx = 0;
		}
#endif

#if HEADER_SIZE > 0
		//volatile int id = get_id(buffer);
		//volatile int len = get_len(buffer);
		set_id(buffer, (get_id(buffer) + 1) & 2047);
		set_len(buffer, 2);
		//_mm_clflushopt(buffer->header.id_addr);
		//_mm_clflushopt(buffer->header.len_addr);
#endif

#if !defined(READ_SRV) && !defined(AVOID_SRV) && defined(VERIFICATION)
		packet* p = get_packet_addr(&pool[id[i]]);
		set_verification(p);
		//__asm__("cldemote (%0)" :: "r" (&p->verification));
#endif
	}

#ifdef SKIP_INDEX
	int skipped_index = 64 / VQENTRY_SIZE;
	int a = last_avail_idx_shadow;
	int b = last_used_idx_shadow;
	last_avail_idx_shadow = (last_avail_idx_shadow + skipped_index) % SIZE_RING;
	last_used_idx_shadow = (last_used_idx_shadow + skipped_index) % SIZE_RING;
	for(int i = skipped_index; i < num_fin; i++) {
#else
	for(int i = 0; i < num_fin; i++) {
#endif
		set_param(&ring_pair->descs[last_avail_idx_shadow], id[i]);
#ifdef STRIDE_VQ
		last_avail_idx_shadow += 4;
		if(SIZE_RING <= last_avail_idx_shadow) {
			last_avail_idx_shadow = last_avail_idx_shadow % 4 + 1;
		}
#else
		last_avail_idx_shadow += 1;
		if(SIZE_RING <= last_avail_idx_shadow) {
			last_avail_idx_shadow = 0;
		}
#endif

		delete_info(&r->descs[last_used_idx_shadow]);
#ifdef STRIDE_VQ
		last_used_idx_shadow += 4;
		if(SIZE_RING <= last_used_idx_shadow) {
			last_used_idx_shadow = last_used_idx_shadow % 4 + 1;
		}
#else
		last_used_idx_shadow += 1;
		if(SIZE_RING <= last_used_idx_shadow) {
			last_used_idx_shadow = 0;
		}
#endif
	}

#ifdef SKIP_INDEX
	for(int i = 0; i < skipped_index; i++) {
		set_param(&ring_pair->descs[a + i], id[i]);
		delete_info(&r->descs[b + i]);
	}
#endif
}

