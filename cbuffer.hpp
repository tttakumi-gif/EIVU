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

inline void desc::set_param(packet p, uint16_t this_id, dstatus _status) {
	id = this_id;
	len = sizeof(p);
	__atomic_store_n(&status, _status, __ATOMIC_RELEASE);
}

inline ring::ring() {
	size = SIZE_RING;
	for(int i = 0; i < NUM_THREAD; i++) {
		rsrv_idx[i] = i * SIZE_BATCH;
		recv_idx[i] = i * SIZE_BATCH;
		proc_idx[i] = i * SIZE_BATCH;
	}
	memset(descs, 0, sizeof(desc) * SIZE_RING);
	init_descs();
}

inline ring&& ring::operator=(ring&& r) {
	size = r.size;
	memcpy(rsrv_idx, r.rsrv_idx, sizeof(uint16_t) * NUM_THREAD);
	memcpy(recv_idx, r.recv_idx, sizeof(uint16_t) * NUM_THREAD);
	memcpy(proc_idx, r.proc_idx, sizeof(uint16_t) * NUM_THREAD);
	memcpy(descs, r.descs, sizeof(desc) * SIZE_RING);
}

inline void ring::init_descs() {
	for(int i = 0; i < SIZE_RING; i++) {
		descs[i].status = PULL;
	}
}

inline uint16_t ring::get_index(packet *pool, rsource source, uint_fast8_t id) {
	int num = (source == CLT) ? 0 : SIZE_RING;
	int root = id * SIZE_BATCH + num;
	for(uint_fast16_t i = 0; i < SIZE_BATCH; i++) {
		uint_fast16_t index = root + i;

		if(pool[index].len == 0) {
			return index;
		}
	}

	std::cout << "recv: " << recv_idx << ", rsrv: " << rsrv_idx << ", proc: " << proc_idx << std::endl;
	return SIZE_POOL;
}

inline bool ring::dinit(uint_fast8_t id) {
	uint_fast16_t prev_idx = rsrv_idx[id];
	while(descs[prev_idx].status != PULL) {
	}

	if(unlikely((prev_idx & MOD_ACCESS) == MOD_ACCESS)) {
		rsrv_idx[id] = id * SIZE_BATCH;
	}
	else {
		rsrv_idx[id]++;
	}

	return true;
}

inline bool ring::push(packet p, packet *pool, rsource source, uint_fast8_t id) {
	uint_fast16_t prev_idx = recv_idx[id];

	uint16_t index = get_index(pool, source, id);
	while(SIZE_POOL <= index) {
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

inline void ring::ipush(packet p, packet *pool, rsource source, uint_fast8_t id) {
	uint_fast16_t prev_idx = rsrv_idx[id];
	while(__atomic_load_n(&descs[prev_idx].status, __ATOMIC_ACQUIRE) != PULL) {
	}

	if(unlikely((prev_idx & MOD_ACCESS) == MOD_ACCESS)) {
		rsrv_idx[id] = id * SIZE_BATCH;
	}
	else {
		rsrv_idx[id]++;
	}

	prev_idx = recv_idx[id];
	uint16_t index = get_index(pool, source, id);
	while(SIZE_POOL <= index) {
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
}

inline packet ring::pull(packet *pool, uint_fast8_t id) {
	uint_fast16_t prev_idx = proc_idx[id];
	while(__atomic_load_n(&descs[prev_idx].status, __ATOMIC_ACQUIRE) != PUSH) {
	}

	if(unlikely((prev_idx & MOD_ACCESS) == MOD_ACCESS)) {
		proc_idx[id] = id * SIZE_BATCH;
	}
	else {
		proc_idx[id]++;
	}

	descs[prev_idx].entry = pool + descs[prev_idx].id;
	packet ret = *(descs[prev_idx].entry);
	descs[prev_idx].delete_info(PULL);

	return ret;
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
