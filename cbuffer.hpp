#pragma once

inline desc::desc() {
}

inline void desc::delete_info(dstatus _status) {
	entry->len = 0;
	//len = 0;
	__atomic_store_n(&status, _status, __ATOMIC_RELEASE);
}

inline void desc::set_param(uint_fast8_t this_id, dstatus _status) {
	id = this_id;
	//len = SIZE_PACKET;
	__atomic_store_n(&status, _status, __ATOMIC_RELEASE);
}

inline ring::ring() {
	size = SIZE_RING;
	rsrv_idx = 0;
	proc_idx = 0;
	memset(descs, 0, sizeof(desc) * SIZE_RING);
	init_descs();
}

inline void ring::operator=(ring&& r) {
	size = r.size;
	rsrv_idx = r.rsrv_idx;
	proc_idx = r.proc_idx;
	memcpy(descs, r.descs, sizeof(desc) * SIZE_RING);
}

inline void ring::init_descs() {
	for(int i = 0; i < SIZE_RING; i++) {
		descs[i].status = PULL;
	}
}

inline void ring::ipush(packet parray[SIZE_BATCH], packet pool[SIZE_POOL], rsource source, uint_fast8_t num_fin) {
	uint_fast8_t prev_idx = rsrv_idx;
	uint_fast8_t index = (source == CLT) ? 0 : SIZE_RING;

	for(uint_fast8_t i = 0; i < num_fin; i++, index++) {
		while(__atomic_load_n(&descs[prev_idx].status, __ATOMIC_ACQUIRE) != PULL) {
			do_none();
		}

		while(0 < pool[index].len) {
			do_none();
		}
		pool[index] = parray[i];

		descs[prev_idx].entry = pool + index;
		descs[prev_idx].set_param(index, PUSH);

		prev_idx = (prev_idx + 1) & NUM_MOD;
	}

	rsrv_idx = prev_idx;
	recv_idx = prev_idx;
}

inline void ring::pull(packet parray[SIZE_BATCH], packet pool[SIZE_POOL], uint_fast8_t num_fin) {
	uint_fast8_t prev_idx = proc_idx;

	for(uint_fast8_t i = 0; i < num_fin; i++) {
		while(__atomic_load_n(&descs[prev_idx].status, __ATOMIC_ACQUIRE) != PUSH) {
			do_none();
		}

		descs[prev_idx].entry = pool + descs[prev_idx].id;
		parray[prev_idx] = *(descs[prev_idx].entry);
		descs[prev_idx].delete_info(PULL);

		prev_idx = (prev_idx + 1) & NUM_MOD;
	}

	proc_idx = prev_idx;
}
