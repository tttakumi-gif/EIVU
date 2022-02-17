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

ring::ring() {
	size = SIZE_RING;
	pool_index = 0;
	rsrv_idx = 0;
	proc_idx = 0;
}

void ring::operator=(ring&& r) {
	size = r.size;
	pool_index = r.pool_index;
	rsrv_idx = r.rsrv_idx;
	proc_idx = r.proc_idx;
//	memcpy(descs, r.descs, sizeof(desc) * SIZE_RING);
}

void ring::init_descs() {
	memset(descs, 0, sizeof(desc) * SIZE_RING);
	for(int i = 0; i < SIZE_RING; i++) {
		descs[i].id = i;
		descs[i].flags = descs[i].flags | USED_FLAG;
	}
}

void ring::zero_push(buf *pool, int num_fin, bool is_stream) {
	int prev_idx = rsrv_idx;
	int pool_idx = pool_index;

#if 1
	int prev_idx_shadow = prev_idx;
	int pool_idx_shadow = pool_idx;
	int pool_idx_shadow2 = pool_idx;
#endif

	for(int i = 0; i < num_fin; i++) {
		// ディスクリプタとパケットプールが確保できるまでwait
#if defined(AVOID_CLT) || defined(SKIP_CLT)
		wait_push(prev_idx);
#else
		//packet *p = (packet*)(pool + pool_idx);
		wait_push(prev_idx);
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
		set_param(&descs[prev_idx_shadow], pool_idx_shadow);
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
			_mm_clflushopt((void*)(pool + pool_idx_shadow2));
			if(++pool_idx_shadow2 % SIZE_POOL == 0) {
				pool_idx_shadow2 = 0;
			}
		}
	}
#endif

	rsrv_idx = prev_idx;
	recv_idx = prev_idx;

	// 共有変数に反映
	pool_index = pool_idx % SIZE_POOL;
}

#ifdef RANDOM
char ids[32] = {21, 10, 24, 22, 15, 31, 0, 30, 14, 1, 11, 2, 13, 23, 12, 3, 25, 17, 4, 16, 26, 19, 5, 28, 20, 6, 27, 7, 8, 18, 29, 9};
#endif

void ring::ipush(packet **parray, buf *pool, int num_fin, bool is_stream) {
	int recv_idx_shadow = recv_idx;

	if(!is_stream) {
		for(int i = 0; i < num_fin; i++) {
#ifdef RANDOM
			buf* buffer = &pool[pool_index + ids[i]];
#else
			buf* buffer = &pool[pool_index + i];
#endif
			packet* p = get_packet_addr(buffer);
			
			// ディスクリプタとパケットプールが確保できるまでwait
#if defined(AVOID_CLT) || defined(SKIP_CLT)
			wait_push(rsrv_idx);
#else
			wait_push(rsrv_idx);
#endif
			if(SIZE_RING <= ++rsrv_idx) {
				rsrv_idx = 0;
			}

			// パケットの紐づけ
			while(*(volatile int*)buffer->len_addr > 0) {
				do_none();
			}
			set_id(buffer, (get_id(buffer) + 1) & 2047);
			set_len(buffer, 1);
//			_mm_clflushopt(buffer->id_addr);
//			_mm_clflushopt(buffer->len_addr);

			memcpy((void*)p, (void*)parray[i], SIZE_PACKET);
//			for(int j = 0; j < NUM_LOOP2; j++) {
//				__asm__("cldemote (%0)" :: "r" ((int64_t*)&p + j));
//				__asm__("cldemote (%0)" :: "r" ((int64_t*)&parray[i] + j));
//			}

			// index更新
			if(SIZE_RING <= ++recv_idx) {
				recv_idx = 0;
			}

		}

		for(int i = 0; i < num_fin; i++) {
#ifdef RANDOM
			set_param(&descs[recv_idx_shadow], pool_index + ids[i]);
#else
			set_param(&descs[recv_idx_shadow], pool_index + i);
#endif

			// index更新
			if(SIZE_RING <= ++recv_idx_shadow) {
				recv_idx_shadow = 0;
			}
		}
	}
	else {
		for(int i = 0; i < num_fin; i++) {
			// ディスクリプタとパケットプールが確保できるまでwait
#ifdef RANDOM
			buf* buffer = &pool[pool_index + ids[i]];
#else
			buf* buffer = &pool[pool_index + i];
#endif
			packet* xmm01 = get_packet_addr(buffer);
			packet* xmm02 = parray[i];

#if defined(AVOID_CLT) || defined(SKIP_CLT)
			wait_push(rsrv_idx);
#else
			wait_push(rsrv_idx);
#endif
			if(SIZE_RING <= ++rsrv_idx) {
				rsrv_idx = 0;
			}

			// パケットの紐づけ
			while(*(volatile int32_t*)buffer->len_addr > 0) {
				do_none();
			}
			set_id(buffer, (get_id(buffer) + 1) & 2047);
			set_len(buffer, 1);

			for(int j = 0; j < NUM_LOOP; j++) {
				if(!IS_PSMALL) {
					_mm256_stream_si256((__m256i*)xmm01 + j, _mm256_stream_load_si256((__m256i*)xmm02 + j));
				}
				else {
					_mm_stream_si128((__m128i*)xmm01 + j, _mm_stream_load_si128((__m128i*)xmm02 + j));
				}
			}

			// index更新
			if(SIZE_RING <= ++recv_idx) {
				recv_idx = 0;
			}
		}

		for(int i = 0; i < num_fin; i++) {
#ifdef RANDOM
			set_param(&descs[recv_idx_shadow], pool_index + ids[i]);
#else
			set_param(&descs[recv_idx_shadow], pool_index + i);
#endif

			// index更新
			if(SIZE_RING <= ++recv_idx_shadow) {
				recv_idx_shadow = 0;
			}
		}
	}

	// 共有変数に反映
	pool_index = (pool_index + num_fin) % SIZE_POOL;
}

void ring::ipush_avoid(int num_fin, bool is_stream) {
	int prev_idx = rsrv_idx;
	int prev_idx_shadow = prev_idx;
	int pool_idx = pool_index;
	int pool_idx_shadow = pool_idx;

	if(!is_stream) {
		for(int i = 0; i < num_fin; i++) {
			// ディスクリプタとパケットプールが確保できるまでwait
			wait_push(prev_idx);

			// パケットの紐づけ
			set_param_avoid(&descs[prev_idx], pool_idx);

			// index更新
			if(++pool_idx % SIZE_POOL == 0) {
				pool_idx = 0;
			}
			if(SIZE_RING <= ++prev_idx) {
				prev_idx = 0;
			}
		}

		rsrv_idx = prev_idx;
		recv_idx = prev_idx;
	}
	else {
		for(int i = 0; i < num_fin; i++) {
			// ディスクリプタとパケットプールが確保できるまでwait
			wait_push(prev_idx);

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
			set_param_avoid(&descs[prev_idx_shadow], pool_idx_shadow);
			// index更新
			if(++pool_idx_shadow % SIZE_POOL == 0) {
				pool_idx_shadow = 0;
			}
			if(SIZE_RING <= ++prev_idx_shadow) {
				prev_idx_shadow = 0;
			}
		}

		rsrv_idx = prev_idx;
		recv_idx = prev_idx;
	}

	// 共有変数に反映
	pool_index = pool_idx % SIZE_POOL;
}

void ring::pull(packet* parray[], buf *pool, int num_fin, bool is_stream) {
	if(!is_stream) {
		for(int i = 0; i < num_fin; i++) {
			// ディスクリプタにバッファが割り当てられるまでwait
			wait_pull(proc_idx);
			buf* buffer = &pool[descs[proc_idx].entry_index];
			packet* p = get_packet_addr(buffer);
			//std::cout << descs[prev_idx].id << std::endl;

			memcpy((void*)(parray[i]), (void*)p, SIZE_PACKET);

			// パケットの取得, ディスクリプタの紐づけ解除
			delete_info(&descs[proc_idx]);

			//memcpy((void*)(parray[i]), (void*)buffer->id_addr, 64);
			//memcpy((void*)(parray[i]), (void*)buffer->len_addr, 64);
			set_id(buffer, (get_id(buffer) + 1) & 2047);
			set_len(buffer, 0);
			//_mm_clflushopt(buffer->id_addr);
			//_mm_clflushopt(buffer->len_addr);

			// index更新
			if(SIZE_RING <= ++proc_idx) {
				proc_idx = 0;
			}
		}
	}
	else {
		for(int i = 0; i < num_fin; i++) {
			// ディスクリプタにバッファが割り当てられるまでwait
			wait_pull(proc_idx);
			buf* buffer = &pool[descs[proc_idx].entry_index];
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

			// パケットの取得, ディスクリプタの紐づけ解除
			delete_info(&descs[proc_idx]);

			set_id(buffer, (get_id(buffer) + 1) & 2047);
			set_len(buffer, 0);

			// index更新
			if(SIZE_RING <= ++proc_idx) {
				proc_idx = 0;
			}
		}
	}
}

void ring::pull_avoid(int num_fin) {
	int prev_idx = proc_idx;

	for(int i = 0; i < num_fin; i++) {
		// ディスクリプタにバッファが割り当てられるまでwait
		wait_pull(prev_idx);

		// パケットの取得, ディスクリプタの紐づけ解除
		delete_info_avoid(&descs[proc_idx]);

		// index更新
		if(SIZE_RING <= ++prev_idx) {
			prev_idx = 0;
		}
	}

	// 共有変数に反映
	proc_idx = prev_idx;
}

#ifdef AVOID_SRV
void ring::move_packet(ring* ring_pair, int num_fin) {
#else 
void ring::move_packet(ring* ring_pair, buf *pool, int num_fin) {
#endif
	int id[32];
	int proc_idx_shadow = proc_idx;
	int rsrv_idx_shadow = ring_pair->rsrv_idx; 

	for(int i = 0; i < num_fin; i++) {

#ifdef AVOID_SRV
		wait_pull_avoid(proc_idx);
#else
		wait_pull(proc_idx);
		id[i] = descs[proc_idx].entry_index;
		buf* buffer = &pool[id[i]];
		packet *p = get_packet_addr(buffer);
#endif
		p = get_packet_addr(&pool[id[i]]);
		//std::cout << descs[prev_idx].id <<std::endl;

		proc_idx = (proc_idx + 1) % SIZE_RING;

		ring_pair->wait_push(ring_pair->rsrv_idx);
		ring_pair->rsrv_idx = (ring_pair->rsrv_idx + 1) % SIZE_RING;

		set_id(buffer, (get_id(buffer) + 1) & 2047);
		set_len(buffer, 2);
//		_mm_clflushopt(buffer->id_addr);
//		_mm_clflushopt(buffer->len_addr);

#if !defined(READ_SRV) && !defined(AVOID_SRV)
		set_verification(p);
		//__asm__("cldemote (%0)" :: "r" (&p->verification));
#endif
	}

	for(int i = 0; i < num_fin; i++) {
#ifdef AVOID_SRV
		set_param_avoid(&ring_pair->descs[rsrv_idx_shadow], id[i]);
#else
		set_param(&ring_pair->descs[rsrv_idx_shadow], id[i]);
#endif
		rsrv_idx_shadow = (rsrv_idx_shadow + 1) % SIZE_RING;

		delete_info(&descs[proc_idx_shadow]);
		proc_idx_shadow = (proc_idx_shadow + 1) % SIZE_RING;
	}
}

void ring::wait_push(int prev_idx) {
	// ディスクリプタが空くまでwait
	while((*(volatile int16_t*)&descs[prev_idx].flags & USED_FLAG) == 0) {
		do_none();
	}
}

void ring::wait_pull(int prev_idx) {
	// ディスクリプタにバッファが割り当てられるまでwait
	while((*(volatile int16_t*)&descs[prev_idx].flags & AVAIL_FLAG) == 0) {
		do_none();
	}
}

