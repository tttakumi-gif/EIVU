#pragma once

inline void desc::delete_info() {
	__atomic_store_n(&entry->len, 0, __ATOMIC_RELEASE);
	__atomic_store_n(&id, -1, __ATOMIC_RELEASE);
}

inline void desc::set_param(int_fast32_t this_id, packet *pool) {
	entry = pool + this_id;
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

inline void ring::ipush(packet parray[], packet pool[SIZE_POOL], rsource source, int_fast32_t num_fin) {
	int_fast32_t prev_idx = rsrv_idx;
	int_fast32_t pool_idx = source + pindex;

#if 0
	for(int_fast32_t i = 0; i < num_fin; i++) {
		// ディスクリプタとパケットプールが確保できるまでwait
		wait_push(prev_idx, pool_idx, pool);

		// パケットの紐づけ
		pool[pool_idx] = parray[i];
		descs[prev_idx].set_param(pool_idx, pool);

		// index更新
		if(++pool_idx % NUM_PMOD == 0) {
			pool_idx = source;
		}
		//prev_idx = (prev_idx + 1) & NUM_MOD;
		prev_idx = (prev_idx + 1) % SIZE_RING;
	}
#else
	int iii = pool_idx;
	for(int_fast32_t i = 0; i < num_fin; i++) {
		// ディスクリプタとパケットプールが確保できるまでwait
		wait_push(prev_idx, pool_idx, pool);
		auto* xmm01 = pool + pool_idx;
		auto* xmm02 = parray + i;

		// パケットの紐づけ
		for(int_fast32_t j = 0; j < NUM_LOOP; j++) {
			if(!IS_PSMALL) {
				_mm256_stream_si256((__m256i*)xmm01 + j, _mm256_stream_load_si256((__m256i*)xmm02 + j));
			}
			else {
				_mm_stream_si128((__m128i*)xmm01 + j, _mm_stream_load_si128((__m128i*)xmm02 + j));
			}
		}

		if(++pool_idx % NUM_PMOD == 0) {
			pool_idx = source;
		}
	}
	_mm_sfence();

	pool_idx = iii;
	for(int_fast32_t i = 0; i < num_fin; i++) {

		descs[prev_idx].set_param(pool_idx, pool);

		// index更新
		if(++pool_idx % NUM_PMOD == 0) {
			pool_idx = source;
		}
		if(SIZE_RING <= ++prev_idx) {
			prev_idx = 0;
		}
	}
#endif

	// 共有変数に反映
	rsrv_idx = prev_idx;
	recv_idx = prev_idx;
	pindex = pool_idx % NUM_PMOD;
}

inline void ring::pull(packet parray[], packet pool[SIZE_POOL], int_fast32_t num_fin) {
	int_fast32_t prev_idx = proc_idx;

	_mm_lfence();
	for(int_fast32_t i = 0; i < num_fin; i++) {
		// ディスクリプタにバッファが割り当てられるまでwait
		wait_pull(prev_idx);

		// パケットの取得, ディスクリプタの紐づけ解除
		descs[prev_idx].entry = pool + descs[prev_idx].id;
		parray[i] = *(descs[prev_idx].entry);
		descs[prev_idx].delete_info();

		// index更新
		if(SIZE_RING <= ++prev_idx) {
			prev_idx = 0;
		}
	}

	// 共有変数に反映
	proc_idx = prev_idx;
}

inline void ring::move_packet(packet pool[SIZE_POOL], int_fast32_t num_fin, int_fast32_t id[]) {
	int_fast32_t i;
	int_fast32_t prev_idx = proc_idx;

	for(i = 0; i < num_fin; i++) {
		wait_pull(prev_idx);

		id[i] = descs[prev_idx].id;
		__atomic_store_n(&descs[prev_idx].id, -1, __ATOMIC_RELEASE);
		prev_idx = (prev_idx + 1) % SIZE_RING;
	}
	proc_idx = prev_idx;

	prev_idx = ring_pair->rsrv_idx;
	for(i = 0; i < num_fin; i++) {
		while(0 <= __atomic_load_n(&ring_pair->descs[prev_idx].id, __ATOMIC_ACQUIRE)) {
			do_none();
		}
		(pool + id[i])->set_verification();
		ring_pair->descs[prev_idx].set_param(id[i], pool);

		prev_idx = (prev_idx + 1) % SIZE_RING;
	}
	ring_pair->rsrv_idx = prev_idx;
	ring_pair->recv_idx = prev_idx;
}

inline void ring::wait_push(int_fast32_t prev_idx, int_fast32_t pool_idx, packet pool[SIZE_POOL]) {
	// ディスクリプタが空くまでwait
	while(0 <= __atomic_load_n(&descs[prev_idx].id, __ATOMIC_ACQUIRE)) {
		do_none();
	}

	// パケットを格納するバッファが空くまでwait
	while(0 < __atomic_load_n(&pool[pool_idx].len, __ATOMIC_ACQUIRE)) {
		do_none();
	}
}

inline void ring::wait_pull(int_fast32_t prev_idx) {
	// ディスクリプタにバッファが割り当てられるまでwait
	while(__atomic_load_n(&descs[prev_idx].id, __ATOMIC_ACQUIRE) < 0) {
		do_none();
	}
}
