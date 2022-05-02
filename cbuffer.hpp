#pragma once

void delete_info(desc* d) {
	d->entry_index = -1;
	d->flags = d->flags | USED_FLAG;
	d->flags = d->flags & ~AVAIL_FLAG;
}

void delete_info_avoid(desc* d) {
	d->id = -1;
}

void set_param(desc* d, int this_id) {
	d->entry_index = this_id;
	d->flags = d->flags & ~USED_FLAG;
	d->flags = d->flags | AVAIL_FLAG;
}

void set_param_avoid(desc* d, int16_t this_id) {
	d->id = this_id;
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

void zero_push(ring* r, buf *pool, int num_fin, bool is_stream) {
	int prev_idx = r->last_avail_idx;
	int pool_idx = r->pool_index;

#if 1
	int prev_idx_shadow = prev_idx;
	int pool_idx_shadow = pool_idx;
	int pool_idx_shadow2 = pool_idx;
#endif

	for(int i = 0; i < num_fin; i++) {
		// ディスクリプタとパケットプールが確保できるまでwait
#if defined(AVOID_CLT) || defined(SKIP_CLT)
		wait_push(r, prev_idx);
#else
		//packet *p = (packet*)(pool + pool_idx);
		wait_push(r, prev_idx);
#endif

//		packet *p = (packet*)(pool + pool_idx);
//		//memcpy(p, p + 1, SIZE_PACKET);
//		for(int j = 0; j < NUM_LOOP; j++) {
//			_mm256_stream_si256((__m256i*)p + j, _mm256_stream_load_si256((__m256i*)(p + 1) + j));
//		}

		// index更新
		if(++pool_idx % SIZE_POOL == 0) {
			pool_idx = 0;
		}
		if(SIZE_RING <= ++prev_idx) {
			prev_idx = 0;
		}
	}
#if 1
	for(int i = 0; i < num_fin; i++) {
		set_param(&r->descs[prev_idx_shadow], pool_idx_shadow);
		// index更新
		if(++pool_idx_shadow % SIZE_POOL == 0) {
			pool_idx_shadow = 0;
		}
		if(SIZE_RING <= ++prev_idx_shadow) {
			prev_idx_shadow = 0;
		}
	}
	if(is_stream) {
		for(int i = 0; i < num_fin; i++) {
			//_mm_clflushopt((void*)(pool + pool_idx_shadow2));
			if(++pool_idx_shadow2 % SIZE_POOL == 0) {
				pool_idx_shadow2 = 0;
			}
		}
	}
#endif

	r->last_avail_idx = prev_idx;

	// 共有変数に反映
	r->pool_index = pool_idx % SIZE_POOL;
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
			while(*(volatile int*)buffer->header.len_addr > 0) {
				do_none();
			}
			set_id(buffer, (get_id(buffer) + 1) & 2047);
			set_len(buffer, 1);
#endif
//			_mm_clflushopt(buffer->id_addr);
//			_mm_clflushopt(buffer->len_addr);

			memcpy((void*)p, (void*)parray[i], SIZE_PACKET);
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

	//for(int i = 0; i < num_fin; i++) {
	int i = 1;
	int a = last_avail_idx_shadow;
	last_avail_idx_shadow = (last_avail_idx_shadow + 1) % SIZE_RING;
	for(; i < num_fin; i++) {
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
	set_param(&r->descs[a], r->pool_index + ids[0]);

	// 共有変数に反映
	r->pool_index = (r->pool_index + num_fin) % SIZE_POOL;
}

void ipush_avoid(ring* r, int num_fin, bool is_stream) {
	int prev_idx = r->last_avail_idx;
	int prev_idx_shadow = prev_idx;
	int pool_idx = r->pool_index;
	int pool_idx_shadow = pool_idx;

	if(!is_stream) {
		for(int i = 0; i < num_fin; i++) {
			// ディスクリプタとパケットプールが確保できるまでwait
			wait_push(r, prev_idx);

			// パケットの紐づけ
			set_param_avoid(&r->descs[prev_idx], pool_idx);

			// index更新
			if(++pool_idx % SIZE_POOL == 0) {
				pool_idx = 0;
			}
			if(SIZE_RING <= ++prev_idx) {
				prev_idx = 0;
			}
		}

		r->last_avail_idx = prev_idx;
	}
	else {
		for(int i = 0; i < num_fin; i++) {
			// ディスクリプタとパケットプールが確保できるまでwait
			wait_push(r, prev_idx);

			// パケットの紐づけ
			//for(volatile int j = 0; j < NUM_LOOP; j++) {
			//	;
			//}

			// index更新
			if(++pool_idx % SIZE_POOL == 0) {
				pool_idx = 0;
			}
			if(SIZE_RING <= ++prev_idx) {
				prev_idx = 0;
			}
		}

		for(int i = 0; i < num_fin; i++) {
			set_param_avoid(&r->descs[prev_idx_shadow], pool_idx_shadow);
			// index更新
			if(++pool_idx_shadow % SIZE_POOL == 0) {
				pool_idx_shadow = 0;
			}
			if(SIZE_RING <= ++prev_idx_shadow) {
				prev_idx_shadow = 0;
			}
		}

		r->last_avail_idx = prev_idx;
	}

	// 共有変数に反映
	r->pool_index = pool_idx % SIZE_POOL;
}

void pull(ring* r, packet* parray[], buf *pool, int num_fin, bool is_stream) {
	int last_used_idx_shadow = r->last_used_idx;
	if(!is_stream) {
		for(int i = 0; i < num_fin; i++) {
			// ディスクリプタにバッファが割り当てられるまでwait
			wait_pull(r, r->last_used_idx);
			buf* buffer = &pool[r->descs[r->last_used_idx].entry_index];
			packet* p = get_packet_addr(buffer);
			//std::cout << r->descs[r->last_used_idx].flags << std::endl;

			memcpy((void*)(parray[i]), (void*)p, SIZE_PACKET);

			//memcpy((void*)(parray[i]), (void*)buffer->id_addr, 64);
			//memcpy((void*)(parray[i]), (void*)buffer->len_addr, 64);
#if HEADER_SIZE > 0
			set_id(buffer, (get_id(buffer) + 1) & 2047);
			set_len(buffer, 0);
#endif
			//_mm_clflushopt(buffer->id_addr);
			//_mm_clflushopt(buffer->len_addr);

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

	//for(int i = 0; i < num_fin; i++) {
	int i = 1;
	int a = last_used_idx_shadow;
	last_used_idx_shadow = (last_used_idx_shadow + 1) % SIZE_RING;
	for(; i < num_fin; i++) {
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
	delete_info(&r->descs[a]);
}

void pull_avoid(ring* r, int num_fin) {
	int prev_idx = r->last_used_idx;

	for(int i = 0; i < num_fin; i++) {
		// ディスクリプタにバッファが割り当てられるまでwait
		wait_pull(r, prev_idx);

		// パケットの取得, ディスクリプタの紐づけ解除
		delete_info_avoid(&r->descs[prev_idx]);

		// index更新
		if(SIZE_RING <= ++prev_idx) {
			prev_idx = 0;
		}
	}

	// 共有変数に反映
	r->last_used_idx = prev_idx;
}

#ifdef AVOID_SRV
void move_packet(ring* r, ring* ring_pair, int num_fin) {
#else 
void move_packet(ring* r, ring* ring_pair, buf *pool, int num_fin) {
#endif
	int id[32];
	int last_used_idx_shadow = r->last_used_idx;
	int last_avail_idx_shadow = ring_pair->last_avail_idx; 

	for(int i = 0; i < num_fin; i++) {

#ifdef AVOID_SRV
		wait_pull_avoid(r->last_used_idx);
#else
		wait_pull(r, r->last_used_idx);
		id[i] = r->descs[r->last_used_idx].entry_index;
		buf* buffer = &pool[id[i]];
#endif

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
		set_id(buffer, (get_id(buffer) + 1) & 2047);
		set_len(buffer, 2);
#endif
//		_mm_clflushopt(buffer->id_addr);
//		_mm_clflushopt(buffer->len_addr);

#if !defined(READ_SRV) && !defined(AVOID_SRV) && defined(VERIFICATION)
		packet* p = get_packet_addr(&pool[id[i]]);
		set_verification(p);
		//__asm__("cldemote (%0)" :: "r" (&p->verification));
#endif
	}

	//for(int i = 0; i < num_fin; i++) {
	int i = 1;
	int a = last_avail_idx_shadow;
	int b = last_used_idx_shadow;
	last_avail_idx_shadow = (last_avail_idx_shadow + 1) % SIZE_RING;
	last_used_idx_shadow = (last_used_idx_shadow + 1) % SIZE_RING;
	for(; i < num_fin; i++) {
#ifdef AVOID_SRV
		set_param_avoid(&ring_pair->descs[last_avail_idx_shadow], id[i]);
#else
		set_param(&ring_pair->descs[last_avail_idx_shadow], id[i]);
#endif
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
	set_param(&ring_pair->descs[a], id[0]);
	delete_info(&r->descs[b]);
}

