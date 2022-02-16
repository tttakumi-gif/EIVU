#pragma once

inline void desc::delete_info() {
	entry->packet_len = 0;
	id = -1;
}

inline void desc::delete_info_avoid() {
	id = -1;
}

inline void desc::set_param(int32_t this_id, buf *pool) {
	entry = (packet*)(pool + this_id);
	id = this_id;
}

inline void desc::set_param_avoid(int32_t this_id) {
	id = this_id;
}

inline ring::ring() {
	size = SIZE_RING;
	pindex = 0;
	rsrv_idx = 0;
	proc_idx = 0;
	ring_pair = nullptr;
	init_descs();
}

inline void ring::operator=(ring&& r) {
	size = r.size;
	pindex = r.pindex;
	rsrv_idx = r.rsrv_idx;
	proc_idx = r.proc_idx;
	ring_pair = r.ring_pair;
	memcpy(descs, r.descs, sizeof(desc) * SIZE_RING);
	rand.set_range(0, SIZE_POOL - 1);
}

inline void ring::init_descs() {
	memset(descs, 0, sizeof(desc) * SIZE_RING);
	for(int i = 0; i < SIZE_RING; i++) {
		__atomic_store_n(&descs[i].id, -1, __ATOMIC_RELEASE);
	}
}

inline void ring::set_ringaddr(ring *r) {
	ring_pair = r;
}

inline void ring::zero_push(buf *pool, int32_t num_fin, bool is_stream) {
	int32_t prev_idx = rsrv_idx;
	int32_t pool_idx = pindex;

#if 1
	int32_t prev_idx_shadow = prev_idx;
	int32_t pool_idx_shadow = pool_idx;
	int32_t pool_idx_shadow2 = pool_idx;
#endif

	for(int32_t i = 0; i < num_fin; i++) {
		// ディスクリプタとパケットプールが確保できるまでwait
#if defined(AVOID_CLT) || defined(SKIP_CLT)
		wait_push(prev_idx);
#else
		//packet *p = (packet*)(pool + pool_idx);
		wait_push(prev_idx);
#endif

//		packet *p = (packet*)(pool + pool_idx);
//		//memcpy(p, p + 1, SIZE_PACKET);
//		for(int32_t j = 0; j < NUM_LOOP; j++) {
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
	for(int32_t i = 0; i < num_fin; i++) {
		descs[prev_idx_shadow].set_param(pool_idx_shadow, pool);
		// index更新
		if(++pool_idx_shadow % SIZE_POOL == 0) {
			pool_idx_shadow = 0;
		}
		if(SIZE_RING <= ++prev_idx_shadow) {
			prev_idx_shadow = 0;
		}
	}
	if(is_stream) {
		for(int32_t i = 0; i < num_fin; i++) {
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
	pindex = pool_idx % SIZE_POOL;
}

#ifdef RANDOM
char ids[32] = {21, 10, 24, 22, 15, 31, 0, 30, 14, 1, 11, 2, 13, 23, 12, 3, 25, 17, 4, 16, 26, 19, 5, 28, 20, 6, 27, 7, 8, 18, 29, 9};
#endif

inline void ring::ipush(packet **parray, buf *pool, int32_t num_fin, bool is_stream) {
	int32_t recv_idx_shadow = recv_idx;
	int32_t pool_idx = pindex;

	if(!is_stream) {
		for(int32_t i = 0; i < num_fin; i++) {
#ifdef RANDOM
			buf* buffer = &pool[pool_idx + ids[i]];
#else
			buf* buffer = &pool[pool_idx + i];
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
			set_id(buffer, (get_id(buffer) + 1) & 2047);
			set_len(buffer, (get_len(buffer) + 1) & 2047);
//			_mm_clflushopt(buffer->id_addr);
//			_mm_clflushopt(buffer->len_addr);
//
			memcpy((void*)p, (void*)parray[i], SIZE_PACKET);
//			for(int32_t j = 0; j < NUM_LOOP2; j++) {
//				__asm__("cldemote (%0)" :: "r" ((int64_t*)&p + j));
//				__asm__("cldemote (%0)" :: "r" ((int64_t*)&parray[i] + j));
//			}
			//descs[prev_idx].set_param(pool_idx, pool);

			// index更新
			if(SIZE_RING <= ++recv_idx) {
				recv_idx = 0;
			}

		}

		for(int32_t i = 0; i < num_fin; i++) {
#ifdef RANDOM
			descs[recv_idx_shadow].set_param(pool_idx + ids[i], pool);
#else
			descs[recv_idx_shadow].set_param(pool_idx + i, pool);
#endif

			// index更新
			if(SIZE_RING <= ++recv_idx_shadow) {
				recv_idx_shadow = 0;
			}
		}
	}
	else {
		for(int32_t i = 0; i < num_fin; i++) {
			// ディスクリプタとパケットプールが確保できるまでwait
#ifdef RANDOM
			buf* buffer = &pool[pool_idx + ids[i]];
#else
			buf* buffer = &pool[pool_idx + i];
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
			set_id(buffer, (get_id(buffer) + 1) & 2047);
			set_len(buffer, (get_len(buffer) + 1) & 2047);
			for(int32_t j = 0; j < NUM_LOOP; j++) {
				if(!IS_PSMALL) {
					_mm256_stream_si256((__m256i*)xmm01 + j, _mm256_stream_load_si256((__m256i*)xmm02 + j));
				}
				else {
					_mm_stream_si128((__m128i*)xmm01 + j, _mm_stream_load_si128((__m128i*)xmm02 + j));
				}
			}
			//descs[prev_idx].set_param(pool_idx, pool);

			// index更新
			if(SIZE_RING <= ++recv_idx) {
				recv_idx = 0;
			}
		}

		for(int32_t i = 0; i < num_fin; i++) {
#ifdef RANDOM
			descs[recv_idx_shadow].set_param(pool_idx + ids[i], pool);
#else
			descs[recv_idx_shadow].set_param(pool_idx + i, pool);
#endif

			// index更新
			if(SIZE_RING <= ++recv_idx_shadow) {
				recv_idx_shadow = 0;
			}
		}
	}

	// 共有変数に反映
	pindex = (pindex + num_fin) % SIZE_POOL;
}

inline void ring::ipush_avoid(int32_t num_fin, bool is_stream) {
	int32_t prev_idx = rsrv_idx;
	int32_t prev_idx_shadow = prev_idx;
	int32_t pool_idx = pindex;
	int32_t pool_idx_shadow = pool_idx;

	if(!is_stream) {
		for(int32_t i = 0; i < num_fin; i++) {
			// ディスクリプタとパケットプールが確保できるまでwait
			wait_push(prev_idx);

			// パケットの紐づけ
			descs[prev_idx].set_param_avoid(pool_idx);

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
		for(int32_t i = 0; i < num_fin; i++) {
			// ディスクリプタとパケットプールが確保できるまでwait
			wait_push(prev_idx);

			// パケットの紐づけ
			//for(volatile int32_t j = 0; j < NUM_LOOP; j++) {
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

		for(int32_t i = 0; i < num_fin; i++) {
			descs[prev_idx_shadow].set_param_avoid(pool_idx_shadow);
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
	pindex = pool_idx % SIZE_POOL;
}

inline void ring::pull(packet* parray[], buf *pool, int32_t num_fin, bool is_stream) {
	if(!is_stream) {
		for(int32_t i = 0; i < num_fin; i++) {
			// ディスクリプタにバッファが割り当てられるまでwait
			wait_pull(proc_idx);
			buf* buffer = &pool[descs[proc_idx].id];
			packet* p = get_packet_addr(buffer);
			//std::cout << descs[prev_idx].id << std::endl;

			//memcpy((void*)(parray[i]), (void*)buffer->id_addr, 64);
			//memcpy((void*)(parray[i]), (void*)buffer->len_addr, 64);
			set_id(buffer, (get_id(buffer) + 1) & 2047);
			set_len(buffer, (get_len(buffer) + 1) & 2047);
			//_mm_clflushopt(buffer->id_addr);
			//_mm_clflushopt(buffer->len_addr);

			memcpy((void*)(parray[i]), (void*)p, SIZE_PACKET);

			// パケットの取得, ディスクリプタの紐づけ解除
			descs[proc_idx].entry = p;
			descs[proc_idx].delete_info();

			// index更新
			if(SIZE_RING <= ++proc_idx) {
				proc_idx = 0;
			}
		}
	}
	else {
		for(int32_t i = 0; i < num_fin; i++) {
			// ディスクリプタにバッファが割り当てられるまでwait
			wait_pull(proc_idx);
			buf* buffer = &pool[descs[proc_idx].id];
			packet *p = get_packet_addr(buffer);

			set_id(buffer, (get_id(buffer) + 1) & 2047);
			set_len(buffer, (get_len(buffer) + 1) & 2047);

			auto* xmm01 = parray[i];
			auto* xmm02 = p;
			for(int32_t j = 0; j < NUM_LOOP; j++) {
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
			descs[proc_idx].entry = p;
			descs[proc_idx].delete_info();

			// index更新
			if(SIZE_RING <= ++proc_idx) {
				proc_idx = 0;
			}
		}
	}
}

inline void ring::pull_avoid(int32_t num_fin) {
	int32_t prev_idx = proc_idx;

	for(int32_t i = 0; i < num_fin; i++) {
		// ディスクリプタにバッファが割り当てられるまでwait
		wait_pull(prev_idx);

		// パケットの取得, ディスクリプタの紐づけ解除
		descs[prev_idx].delete_info_avoid();

		// index更新
		if(SIZE_RING <= ++prev_idx) {
			prev_idx = 0;
		}
	}

	// 共有変数に反映
	proc_idx = prev_idx;
}

#ifdef AVOID_SRV
inline void ring::move_packet(int32_t num_fin) {
#else 
inline void ring::move_packet(buf *pool, int32_t num_fin) {
#endif
	int32_t id[32];
	int32_t proc_idx_shadow = proc_idx;
	int32_t rsrv_idx_shadow = ring_pair->rsrv_idx; 

	for(int i = 0; i < num_fin; i++) {

#ifdef AVOID_SRV
		wait_pull_avoid(proc_idx);
#else
		wait_pull(proc_idx);
		buf* buffer = &pool[descs[proc_idx].id];
		packet *p = get_packet_addr(buffer);
#endif
		p = get_packet_addr(&pool[descs[proc_idx].id]);
		//std::cout << descs[prev_idx].id <<std::endl;
		id[i] = descs[proc_idx].id;

		proc_idx = (proc_idx + 1) % SIZE_RING;

		ring_pair->wait_push(ring_pair->rsrv_idx);
		ring_pair->rsrv_idx = (ring_pair->rsrv_idx + 1) % SIZE_RING;

		set_id(buffer, (get_id(buffer) + 1) & 2047);
		set_len(buffer, (get_len(buffer) + 1) & 2047);
//		_mm_clflushopt(buffer->id_addr);
//		_mm_clflushopt(buffer->len_addr);

#if !defined(READ_SRV) && !defined(AVOID_SRV)
		set_verification(p);
		//__asm__("cldemote (%0)" :: "r" (&p->verification));
#endif
	}

	for(int i = 0; i < num_fin; i++) {
#ifdef AVOID_SRV
		ring_pair->descs[rsrv_idx_shadow].set_param_avoid(id[i]);
#else
		ring_pair->descs[rsrv_idx_shadow].set_param(id[i], pool);
#endif
		rsrv_idx_shadow = (rsrv_idx_shadow + 1) % SIZE_RING;

		descs[proc_idx_shadow].id = -1;
		proc_idx_shadow = (proc_idx_shadow + 1) % SIZE_RING;
	}
}

inline void ring::wait_push(int32_t prev_idx) {
	// ディスクリプタが空くまでwait
	while(0 <= __atomic_load_n(&descs[prev_idx].id, __ATOMIC_ACQUIRE)) {
		do_none();
	}
}

inline void ring::wait_pull(int32_t prev_idx) {
	// ディスクリプタにバッファが割り当てられるまでwait
	while(__atomic_load_n(&descs[prev_idx].id, __ATOMIC_ACQUIRE) < 0) {
		do_none();
	}
}

