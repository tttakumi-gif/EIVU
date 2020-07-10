#pragma once

inline void desc::delete_info() {
	entry->len = 0;
	__atomic_store_n(&id, -1, __ATOMIC_RELEASE);
}

inline void desc::set_param(int_fast32_t this_id, packet *pool) {
	entry = pool + this_id;
	__atomic_store_n(&id, this_id, __ATOMIC_RELEASE);
}

inline ring::ring() {
	size = SIZE_RING;
	rsrv_idx = 0;
	proc_idx = 0;
	init_descs();
}

inline void ring::operator=(ring&& r) {
	size = r.size;
	rsrv_idx = r.rsrv_idx;
	proc_idx = r.proc_idx;
	memcpy(descs, r.descs, sizeof(desc) * SIZE_RING);
}

inline void ring::init_descs() {
	memset(descs, 0, sizeof(desc) * SIZE_RING);
	for(int i = 0; i < SIZE_RING; i++) {
		descs[i].id = -1;
	}
}

inline void ring::ipush(packet parray[SIZE_BATCH], packet pool[SIZE_POOL], rsource source, uint_fast8_t num_fin) {
	uint_fast8_t prev_idx = rsrv_idx;
	uint_fast32_t index = (source == CLT) ? 0 : SIZE_RING;

	for(uint_fast8_t i = 0; i < num_fin; i++, index++) {
		while(0 <= __atomic_load_n(&descs[prev_idx].id, __ATOMIC_ACQUIRE)) {
			do_none();
		}

		while(0 < pool[index].len) {
			do_none();
		}

		pool[index] = parray[i];
		descs[prev_idx].set_param(index, pool);

		prev_idx = (prev_idx + 1) & NUM_MOD;
	}

	rsrv_idx = prev_idx;
	recv_idx = prev_idx;
}

inline void ring::pull(packet parray[SIZE_BATCH], packet pool[SIZE_POOL], uint_fast8_t num_fin) {
	uint_fast32_t prev_idx = proc_idx;

	for(uint_fast8_t i = 0; i < num_fin; i++) {
		while(__atomic_load_n(&descs[prev_idx].id, __ATOMIC_ACQUIRE) < 0) {
			do_none();
		}

		descs[prev_idx].entry = pool + descs[prev_idx].id;
		parray[prev_idx] = *(descs[prev_idx].entry);
		descs[prev_idx].delete_info();

		prev_idx = (prev_idx + 1) & NUM_MOD;
	}

	proc_idx = prev_idx;
}
