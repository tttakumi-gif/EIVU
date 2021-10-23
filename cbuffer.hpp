#pragma once

inline void desc::delete_info() {
	__atomic_store_n(&entry->len, 0, __ATOMIC_RELEASE);
	__atomic_store_n(&id, -1, __ATOMIC_RELEASE);
}

inline void desc::delete_info_avoid() {
	__atomic_store_n(&id, -1, __ATOMIC_RELEASE);
}

inline void desc::set_param(int_fast32_t this_id, packet *pool) {
	entry = pool + this_id;
	__atomic_store_n(&id, this_id, __ATOMIC_RELEASE);
}

inline void desc::set_param_avoid(int_fast32_t this_id) {
	__atomic_store_n(&id, this_id, __ATOMIC_RELEASE);
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

inline void ring::zero_push(packet pool[], rsource source, int_fast32_t num_fin, bool is_stream) {
	int_fast32_t prev_idx = rsrv_idx;
	int_fast32_t pool_idx = source + pindex;

#if 1
	int_fast32_t prev_idx_shadow = prev_idx;
	int_fast32_t pool_idx_shadow = pool_idx;
#endif

	for(int_fast32_t i = 0; i < num_fin; i++) {
		packet *p = pool + pool_idx;
		// ディスクリプタとパケットプールが確保できるまでwait
		wait_push(prev_idx, p);

		//descs[prev_idx].set_param(pool_idx, pool);

		// index更新
		if(++pool_idx % NUM_PMOD == 0) {
			pool_idx = source;
		}
		if(SIZE_RING <= ++prev_idx) {
			prev_idx = 0;
		}
	}
#if 1
	for(int_fast32_t i = 0; i < num_fin; i++) {
		descs[prev_idx_shadow].set_param(pool_idx_shadow, pool);
		// index更新
		if(++pool_idx_shadow % SIZE_POOL == 0) {
			pool_idx_shadow = source;
		}
		if(SIZE_RING <= ++prev_idx_shadow) {
			prev_idx_shadow = 0;
		}
		if(is_stream) {
			_mm_clflushopt((void*)(pool + pool_idx_shadow));
			//_mm_clflush((void*)(pool + pool_idx_shadow));
		}
	}
//	if(is_stream) {
//		for(int_fast32_t i = 0; i < num_fin; i++) {
//			_mm_clflushopt((void*)(pool + pool_idx_shadow2));
//			if(++pool_idx_shadow % NUM_PMOD == 0) {
//				pool_idx_shadow2 = source;
//			}
//			//if(SIZE_RING <= ++prev_idx_shadow) {
//			//	prev_idx_shadow = 0;
//			//}
//		}
//	}
#endif

	rsrv_idx = prev_idx;
	recv_idx = prev_idx;

	// 共有変数に反映
	pindex = pool_idx % NUM_PMOD;
}

inline void ring::ipush(packet parray[], packet pool[SIZE_POOL], rsource source, int_fast32_t num_fin, bool is_stream) {
	int_fast32_t prev_idx = rsrv_idx;
	int_fast32_t pool_idx = source + pindex;

	if(!is_stream) {
#ifndef pattern1
		int_fast32_t prev_idx_shadow = prev_idx;
		int_fast32_t pool_idx_shadow = pool_idx;
#endif

		for(int_fast32_t i = 0; i < num_fin; i++) {
			packet *p = pool + pool_idx;
			// ディスクリプタとパケットプールが確保できるまでwait
#if defined(AVOID_CLT) || defined(SKIP_CLT)
			wait_push(prev_idx);
#else
			wait_push(prev_idx, p);
#endif

			// パケットの紐づけ
			//pool[pool_idx] = parray[i];
			memcpy(p, parray + i, SIZE_PACKET);
#ifdef pattern1
			descs[prev_idx].set_param(pool_idx, pool);
#endif

			// index更新
			if(++pool_idx % NUM_PMOD == 0) {
				pool_idx = source;
			}
			if(SIZE_RING <= ++prev_idx) {
				prev_idx = 0;
			}
		}
#ifndef pattern1
		for(int_fast32_t i = 0; i < num_fin; i++) {
			descs[prev_idx_shadow].set_param(pool_idx_shadow, pool);
			//_mm_clflushopt((void*)(pool + pool_idx_shadow));
			//_mm_clflush((void*)(pool + pool_idx_shadow));
			// index更新
			if(++pool_idx_shadow % NUM_PMOD == 0) {
				pool_idx_shadow = source;
			}
			if(SIZE_RING <= ++prev_idx_shadow) {
				prev_idx_shadow = 0;
			}
		}
#endif
//		for(int_fast32_t i = 0; i < num_fin; i++) {
//			_mm_clflush(&pool[pool_idx_shadow]);
//			// index更新
//			if(++pool_idx_shadow % NUM_PMOD == 0) {
//				pool_idx_shadow = source;
//			}
//			if(SIZE_RING <= ++prev_idx_shadow) {
//				prev_idx_shadow = 0;
//			}
//		}

		rsrv_idx = prev_idx;
		recv_idx = prev_idx;
	}
	else {
#ifndef pattern1
		int_fast32_t prev_idx_shadow = prev_idx;
		int_fast32_t pool_idx_shadow = pool_idx;
#endif

		for(int_fast32_t i = 0; i < num_fin; i++) {
			// ディスクリプタとパケットプールが確保できるまでwait
			auto* xmm01 = pool + pool_idx;
			auto* xmm02 = parray + i;
#if defined(AVOID_CLT) || defined(SKIP_CLT)
			wait_push(prev_idx);
#else
			wait_push(prev_idx, xmm01);
#endif

			// パケットの紐づけ
			//for(volatile int_fast32_t j = 0; j < NUM_LOOP; j++) {
			for(int_fast32_t j = 0; j < NUM_LOOP; j++) {
				if(!IS_PSMALL) {
					_mm256_stream_si256((__m256i*)xmm01 + j, _mm256_load_si256((__m256i*)xmm02 + j));
				}
				else {
					_mm_stream_si128((__m128i*)xmm01 + j, _mm_load_si128((__m128i*)xmm02 + j));
				}
			}
#ifdef pattern1
			descs[prev_idx].set_param(pool_idx, pool);
#endif

			// index更新
			if(++pool_idx % NUM_PMOD == 0) {
				pool_idx = source;
			}
			if(SIZE_RING <= ++prev_idx) {
				prev_idx = 0;
			}
		}

#ifndef pattern1
		for(int_fast32_t i = 0; i < num_fin; i++) {
			descs[prev_idx_shadow].set_param(pool_idx_shadow, pool);
			// index更新
			if(++pool_idx_shadow % NUM_PMOD == 0) {
				pool_idx_shadow = source;
			}
			if(SIZE_RING <= ++prev_idx_shadow) {
				prev_idx_shadow = 0;
			}
		}
#endif

		rsrv_idx = prev_idx;
		recv_idx = prev_idx;
	}

	// 共有変数に反映
	pindex = pool_idx % NUM_PMOD;
}

inline void ring::ipush_avoid(rsource source, int_fast32_t num_fin, bool is_stream) {
	int_fast32_t prev_idx = rsrv_idx;
	int_fast32_t prev_idx_shadow = prev_idx;
	int_fast32_t pool_idx = source + pindex;
	int_fast32_t pool_idx_shadow = pool_idx;

	if(!is_stream) {
		for(int_fast32_t i = 0; i < num_fin; i++) {
			// ディスクリプタとパケットプールが確保できるまでwait
			wait_push(prev_idx);

			// パケットの紐づけ
			descs[prev_idx].set_param_avoid(pool_idx);

			// index更新
			if(++pool_idx % NUM_PMOD == 0) {
				pool_idx = source;
			}
			if(SIZE_RING <= ++prev_idx) {
				prev_idx = 0;
			}
		}

		rsrv_idx = prev_idx;
		recv_idx = prev_idx;
	}
	else {
		for(int_fast32_t i = 0; i < num_fin; i++) {
			// ディスクリプタとパケットプールが確保できるまでwait
			wait_push(prev_idx);

			// パケットの紐づけ
			for(volatile int_fast32_t j = 0; j < NUM_LOOP; j++) {
				;
			}

			// index更新
			if(++pool_idx % NUM_PMOD == 0) {
				pool_idx = source;
			}
			if(SIZE_RING <= ++prev_idx) {
				prev_idx = 0;
			}
		}

		for(int_fast32_t i = 0; i < num_fin; i++) {
			descs[prev_idx_shadow].set_param_avoid(pool_idx_shadow);
			// index更新
			if(++pool_idx_shadow % NUM_PMOD == 0) {
				pool_idx_shadow = source;
			}
			if(SIZE_RING <= ++prev_idx_shadow) {
				prev_idx_shadow = 0;
			}
		}

		rsrv_idx = prev_idx;
		recv_idx = prev_idx;
	}

	// 共有変数に反映
	pindex = pool_idx % NUM_PMOD;
}

inline void ring::pull(packet parray[], packet pool[SIZE_POOL], int_fast32_t num_fin, bool is_stream) {
	int_fast32_t prev_idx = proc_idx;

	if(!is_stream) {
		for(int_fast32_t i = 0; i < num_fin; i++) {
			// ディスクリプタにバッファが割り当てられるまでwait
			packet *p = wait_pull(prev_idx, pool);

			//parray[i] = *p;
			memcpy(parray + i, p, SIZE_PACKET);

			// パケットの取得, ディスクリプタの紐づけ解除
			descs[prev_idx].entry = p;
			descs[prev_idx].delete_info();

			// index更新
			if(SIZE_RING <= ++prev_idx) {
				prev_idx = 0;
			}
		}
	}
	else {
		for(int_fast32_t i = 0; i < num_fin; i++) {
			// ディスクリプタにバッファが割り当てられるまでwait
			packet *p = wait_pull(prev_idx, pool);

			//parray[i] = *p;
			//memcpy(parray + i, p, SIZE_PACKET);
			auto* xmm01 = parray + i;
			auto* xmm02 = p;
			for(int_fast32_t j = 0; j < NUM_LOOP; j++) {
				if(!IS_PSMALL) {
					//_mm256_stream_si256((__m256i*)xmm01 + j, _mm256_load_si256((__m256i*)xmm02 + j));
					_mm256_store_si256((__m256i*)xmm01 + j, _mm256_stream_load_si256((__m256i*)xmm02 + j));
				}
				else {
					//_mm_stream_si128((__m128i*)xmm01 + j, _mm_load_si128((__m128i*)xmm02 + j));
					_mm_store_si128((__m128i*)xmm01 + j, _mm_stream_load_si128((__m128i*)xmm02 + j));
				}
			}

			// パケットの取得, ディスクリプタの紐づけ解除
			descs[prev_idx].entry = p;
			descs[prev_idx].delete_info();

			// index更新
			if(SIZE_RING <= ++prev_idx) {
				prev_idx = 0;
			}
		}
	}

	// 共有変数に反映
	proc_idx = prev_idx;
}

inline void ring::pull_avoid(int_fast32_t num_fin) {
	int_fast32_t prev_idx = proc_idx;

	for(int_fast32_t i = 0; i < num_fin; i++) {
		// ディスクリプタにバッファが割り当てられるまでwait
		wait_pull_avoid(prev_idx);

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

inline void ring::move_packet(packet pool[SIZE_POOL], int_fast32_t num_fin) {
	int_fast32_t i;
	int_fast32_t prev_idx = proc_idx;
	int_fast32_t prev_idx2 = ring_pair->rsrv_idx;
	int_fast32_t prev_idx_shadow = prev_idx;
	int_fast32_t prev_idx2_shadow = prev_idx2;
	int_fast32_t id[32];

	for(i = 0; i < num_fin; i++) {
		wait_pull(prev_idx, pool);

		id[i] = descs[prev_idx].id;
		while(0 <= __atomic_load_n(&ring_pair->descs[prev_idx2].id, __ATOMIC_ACQUIRE)) {
			do_none();
		}

		(pool + id[i])->set_verification();
		//ring_pair->descs[prev_idx2].set_param(id[i], pool);

		prev_idx2 = (prev_idx2 + 1) % SIZE_RING;
		//__atomic_store_n(&descs[prev_idx].id, -1, __ATOMIC_RELEASE);
		prev_idx = (prev_idx + 1) % SIZE_RING;
	}

	for(i = 0; i < num_fin; i++) {
		ring_pair->descs[prev_idx2_shadow].set_param(id[i], pool);
		prev_idx2_shadow = (prev_idx2_shadow + 1) % SIZE_RING;
		__atomic_store_n(&descs[prev_idx_shadow].id, -1, __ATOMIC_RELEASE);
		prev_idx_shadow = (prev_idx_shadow + 1) % SIZE_RING;
	}

	proc_idx = prev_idx_shadow;

	ring_pair->rsrv_idx = prev_idx2_shadow;
	ring_pair->recv_idx = prev_idx2_shadow;
}

inline void ring::move_packet_avoid(int_fast32_t num_fin) {
	int_fast32_t i, id;
	int_fast32_t prev_idx = proc_idx;
	int_fast32_t prev_idx2 = ring_pair->rsrv_idx;

	for(i = 0; i < num_fin; i++) {
		wait_pull_avoid(prev_idx);

		id = descs[prev_idx].id;
		while(0 <= __atomic_load_n(&ring_pair->descs[prev_idx2].id, __ATOMIC_ACQUIRE)) {
			do_none();
		}

		ring_pair->descs[prev_idx2].set_param_avoid(id);

		prev_idx2 = (prev_idx2 + 1) % SIZE_RING;
		__atomic_store_n(&descs[prev_idx].id, -1, __ATOMIC_RELEASE);
		prev_idx = (prev_idx + 1) % SIZE_RING;
	}
	proc_idx = prev_idx;

	ring_pair->rsrv_idx = prev_idx2;
	ring_pair->recv_idx = prev_idx2;
}

inline void ring::move_packet_read(packet pool[SIZE_POOL], int_fast32_t num_fin) {
	int_fast32_t i, id;
	int_fast32_t prev_idx = proc_idx;
	int_fast32_t prev_idx2 = ring_pair->rsrv_idx;

	for(i = 0; i < num_fin; i++) {
		wait_pull(prev_idx, pool);

		id = descs[prev_idx].id;
		while(0 <= __atomic_load_n(&ring_pair->descs[prev_idx2].id, __ATOMIC_ACQUIRE)) {
			do_none();
		}

		//(pool + id)->set_verification();
		ring_pair->descs[prev_idx2].set_param(id, pool);

		prev_idx2 = (prev_idx2 + 1) % SIZE_RING;
		__atomic_store_n(&descs[prev_idx].id, -1, __ATOMIC_RELEASE);
		prev_idx = (prev_idx + 1) % SIZE_RING;
	}
	proc_idx = prev_idx;

	ring_pair->rsrv_idx = prev_idx2;
	ring_pair->recv_idx = prev_idx2;
}

inline void ring::wait_push(int_fast32_t prev_idx) {
	// ディスクリプタが空くまでwait
	while(0 <= __atomic_load_n(&descs[prev_idx].id, __ATOMIC_ACQUIRE)) {
		do_none();
	}
}

inline void ring::wait_push(int_fast32_t prev_idx, packet *p) {
	// ディスクリプタが空くまでwait
	while(0 <= __atomic_load_n(&descs[prev_idx].id, __ATOMIC_ACQUIRE)) {
		do_none();
	}

#ifndef ZERO_COPY
	while(0 < __atomic_load_n(&p->len, __ATOMIC_ACQUIRE)) {
		do_none();
	}
#else
	__atomic_load_n(&p->len, __ATOMIC_ACQUIRE);
#endif
}

inline packet* ring::wait_pull(int_fast32_t prev_idx, packet *pool) {
	// ディスクリプタにバッファが割り当てられるまでwait
	while(__atomic_load_n(&descs[prev_idx].id, __ATOMIC_ACQUIRE) < 0) {
		do_none();
	}

	packet *ret = &pool[descs[prev_idx].id];
#ifndef ZERO_COPY
	while(__atomic_load_n(&ret->len, __ATOMIC_ACQUIRE) <= 0) {
		do_none();
	}
#endif
	return ret;
}

inline void ring::wait_pull_avoid(int_fast32_t prev_idx) {
	// ディスクリプタにバッファが割り当てられるまでwait
	while(__atomic_load_n(&descs[prev_idx].id, __ATOMIC_ACQUIRE) < 0) {
		do_none();
	}
}
