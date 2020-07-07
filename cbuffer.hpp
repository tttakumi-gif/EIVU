#pragma once

uint32_t nums[NUM_THREAD + 1];

inline desc::desc() {
}

inline desc::desc(dstatus _status) {
	status = _status;
}

inline void desc::delete_info(dstatus _status) {
	entry->len = 0;
	len = 0;
	__atomic_store_n(&status, _status, __ATOMIC_RELEASE);
}

inline void desc::set_param(packet p, uint_fast16_t this_id, dstatus _status) {
	id = this_id;
	len = sizeof(p);
	__atomic_store_n(&status, _status, __ATOMIC_RELEASE);
}

inline ring::ring() {
	size = SIZE_RING;
	rsrv_idx = 0;
	memset(descs, 0, sizeof(desc) * SIZE_RING);
	init_descs();
}

inline void ring::operator=(ring&& r) {
	size = r.size;
	rsrv_idx = r.rsrv_idx;
	memcpy(descs, r.descs, sizeof(desc) * SIZE_RING);
}

inline void ring::init_descs() {
	for(int i = 0; i < SIZE_RING; i++) {
		descs[i].status = PULL;
	}
}

inline void ring::ipush(packet parray[SIZE_BATCH], packet pool[NUM_THREAD], rsource source, int_fast32_t num_fin) {
	uint_fast16_t prev_idx = rsrv_idx;
	uint_fast16_t index = (source == CLT) ? 0 : SIZE_RING;

	for(int_fast32_t i = 0; i < num_fin; i++, index++) {
		while(__atomic_load_n(&descs[prev_idx].status, __ATOMIC_ACQUIRE) != PULL) {
			do_none();
		}

		while(0 < pool[index].len) {
			do_none();
		}
		pool[index] = parray[i];

		descs[prev_idx].entry = pool + index;
		descs[prev_idx].set_param(parray[i], index, PUSH);

		prev_idx = (prev_idx + 1) & NUM_MOD;
	}

	rsrv_idx = prev_idx;
}

inline void ring::pull(packet parray[SIZE_BATCH], packet pool[NUM_THREAD], int_fast32_t num_fin) {
	for(int_fast32_t i = 0; i < num_fin; i++) {
		while(__atomic_load_n(&descs[i].status, __ATOMIC_ACQUIRE) != PUSH) {
			do_none();
		}

		descs[i].entry = pool + descs[i].id;
		parray[i] = *(descs[i].entry);

		descs[i].delete_info(PULL);
	}
}

inline void set_packet_nums(uint32_t *nums) {
	int mod = NUM_PACKET % NUM_THREAD;
	int result = NUM_PACKET / NUM_THREAD;
	int temp = result;

	nums[0] = 0;
	for(int i = 0; i < NUM_THREAD; i++) {
		if(i < mod) {
			temp++;
		}
		nums[i + 1] = temp;
		temp += result;
	}
}

// 未使用
#if 0
inline uint_fast32_t ring::get_index(packet pool[SIZE_POOL], rsource source, uint_fast8_t id) {
	int_fast32_t num = (source == CLT) ? 0 : SIZE_RING;
	int_fast32_t root = num + id * SIZE_BATCH;
	for(uint_fast32_t i = 0; i < SIZE_BATCH; i++) {
		uint_fast32_t index = root + i;

		if(pool[index].len == 0) {
			return index;
		}
	}

	std::cout << "recv: " << recv_idx << ", rsrv: " << rsrv_idx << ", proc: " << proc_idx << std::endl;
	return SIZE_POOL;
}

inline bool ring::dinit(uint_fast8_t id) {
	uint_fast16_t prev_idx = rsrv_idx[id];
	while(__atomic_load_n(&descs[prev_idx].status, __ATOMIC_ACQUIRE) != PULL) {
		do_none();
	}

	if(unlikely((prev_idx & MOD_ACCESS) == MOD_ACCESS)) {
		rsrv_idx[id] = id * SIZE_BATCH;
	}
	else {
		rsrv_idx[id]++;
	}

	descs[prev_idx].status = INIT;
	return true;
}

inline bool ring::push(packet p, packet pool[NUM_THREAD], rsource source, uint_fast8_t id) {
	uint_fast16_t prev_idx = recv_idx[id];
	while(__atomic_load_n(&descs[prev_idx].status, __ATOMIC_ACQUIRE) != INIT) {
		do_none();
	}

	uint16_t index = get_index(pool, source, id);
	while(SIZE_POOL <= index) {
		do_none();
		index = get_index(pool, source, id);
	}
	pool[index] = p;

	if(unlikely((prev_idx & MOD_ACCESS) == MOD_ACCESS)) {
		recv_idx[id] = id * SIZE_BATCH;
	}
	else {
		recv_idx[id]++;
	}

	descs[prev_idx].entry = pool + index;
	descs[prev_idx].set_param(p, index, PUSH);

	return true;
}
#endif
