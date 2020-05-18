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
	rsrv_idx = 0;
	recv_idx = 0;
	proc_idx = 0;
	size = SIZE_RING;
	memset(descs, 0, sizeof(desc) * SIZE_RING);
	init_descs();
}

ring&& ring::operator=(ring&& r) {
	rsrv_idx = r.rsrv_idx;
	recv_idx = r.recv_idx;
	proc_idx = r.proc_idx;
	size = r.size;
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

bool ring::push(packet p, packet *pool, int qw) {
	uint16_t prev_idx;
	uint16_t index;
	{
		std::lock_guard<std::mutex> lock(recv_mtx);

		if(descs[recv_idx].status != INIT) {
			return false;
		}

		index = get_index(pool, qw);
		if(SIZE_POOL * 2 <= index) {
			return false;
		}
		pool[index] = p;

		prev_idx = recv_idx;
		recv_idx = (recv_idx + 1) & NUM_MOD;
	}

	descs[prev_idx].entry = pool + index;
	descs[prev_idx].set_param(pool[index], index, PUSH);

	return true;
}

bool ring::dinit() {
	uint16_t prev_idx;
	{
		std::lock_guard<std::mutex> lock(rsrv_mtx);

		if(descs[rsrv_idx].status != PULL) {
			return false;
		}

		prev_idx = rsrv_idx;
		rsrv_idx = (rsrv_idx + 1) & NUM_MOD;
	}

	descs[prev_idx] = desc(INIT);

	return true;
}

packet ring::pull(packet *pool) {
	uint16_t prev_idx;
	{
		std::lock_guard<std::mutex> lock(proc_mtx);

		if(descs[proc_idx].status != PUSH) {
			return packet();
		}

		prev_idx = proc_idx;
		proc_idx = (proc_idx + 1) & NUM_MOD;
	}

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
