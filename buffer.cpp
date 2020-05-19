#include "buffer.hpp"

desc::desc() {
}

desc::desc(dstatus _status) {
	status = _status;
}

void desc::delete_info(dstatus _status) {
	entry->len = 0;
	len = 0;
	status = _status;
}

void desc::set_param(packet p, uint16_t this_id, dstatus _status) {
	id = this_id;
	len = sizeof(p);
	status = _status;
}

ring::ring() {
	size = SIZE_RING;
	for(int i = 0; i < NUM_THREAD; i++) {
		rsrv_idx[i] = i;
		recv_idx[i] = i;
		proc_idx[i] = i;
	}
	memset(descs, 0, sizeof(desc) * SIZE_RING);
	init_descs();
}

ring&& ring::operator=(ring&& r) {
	size = r.size;
	memcpy(rsrv_idx, r.rsrv_idx, sizeof(uint16_t) * NUM_THREAD);
	memcpy(recv_idx, r.recv_idx, sizeof(uint16_t) * NUM_THREAD);
	memcpy(proc_idx, r.proc_idx, sizeof(uint16_t) * NUM_THREAD);
	memcpy(descs, r.descs, sizeof(desc) * SIZE_RING);
}

void ring::init_descs() {
	for(int i = 0; i < SIZE_RING; i++) {
		descs[i].status = PULL;
	}
}

uint16_t ring::get_index(packet *pool, int qw) {
	for(uint16_t i = qw; i < SIZE_POOL * 2; i += 2) {
		if(pool[i].len == 0) {
			return i;
		}
	}
	//puts("くぁｗせｄｒｆｔｇｙふじこｌｐ；＠：「」");
	std::cout << "recv: " << recv_idx << ", rsrv: " << rsrv_idx << ", proc: " << proc_idx << std::endl;
	//exit(1);

	return SIZE_POOL * 2;
}

bool ring::push(packet p, packet *pool, int qw, short id) {
	if(descs[recv_idx[id]].status != INIT) {
		return false;
	}

	uint16_t index = get_index(pool, qw);
	if(SIZE_POOL * 2 <= index) {
		return false;
	}
	pool[index] = p;

	uint16_t prev_idx = recv_idx[id];
	recv_idx[id] = (recv_idx[id] + NUM_THREAD) & NUM_MOD;

	descs[prev_idx].entry = pool + index;
	descs[prev_idx].set_param(pool[index], index, PUSH);

	return true;
}

bool ring::dinit(short id) {
	if(descs[rsrv_idx[id]].status != PULL) {
		return false;
	}

	uint16_t prev_idx = rsrv_idx[id];
	rsrv_idx[id] = (rsrv_idx[id] + NUM_THREAD) & NUM_MOD;

	descs[prev_idx] = desc(INIT);

	return true;
}

packet ring::pull(packet *pool, short id) {
	if(descs[proc_idx[id]].status != PUSH) {
		return packet();
	}

	uint16_t prev_idx = proc_idx[id];
	proc_idx[id] = (proc_idx[id] + NUM_THREAD) & NUM_MOD;

	descs[prev_idx].entry = pool + descs[prev_idx].id;
	packet ret = *(descs[prev_idx].entry);
	descs[prev_idx].delete_info(PULL);

	return ret;
}

void set_packet_nums(uint32_t *nums) {
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
