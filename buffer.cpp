#include "buffer.hpp"

desc::desc() {
	len = 0;
}

void desc::set_param(packet p, uint16_t this_id) {
	id = this_id;
	len = sizeof(p);
}

ring::ring() {
	rsrv_idx = 0;
	recv_idx = 0;
	proc_idx = 0;
	size = SIZE_RING;
	memset(descs, 0, sizeof(desc) * SIZE_RING);
}

ring&& ring::operator=(ring&& r) {
#if 0
	rsrv_idx = r.rsrv_idx.load();
	recv_idx = r.recv_idx.load();
	proc_idx = r.proc_idx.load();
#else
	rsrv_idx = r.rsrv_idx;
	recv_idx = r.recv_idx;
	proc_idx = r.proc_idx;
#endif
	size = r.size;
	memcpy(descs, r.descs, sizeof(desc) * SIZE_RING);
}

uint16_t ring::get_index(packet *pool, int qw) {
	for(int i = qw; i < SIZE_POOL * 2; i += 2) {
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
	int index;
	{
		std::lock_guard<std::mutex> lock(recv_mtx);

		if(0 < descs[recv_idx].len) {
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
	descs[prev_idx].set_param(pool[index], index);

	return true;
}

bool ring::dinit() {
	uint16_t prev_idx;
	{
		std::lock_guard<std::mutex> lock(rsrv_mtx);

		if(0 < descs[rsrv_idx].len) {
			return false;
		}

		prev_idx = rsrv_idx;
		rsrv_idx = (rsrv_idx + 1) & NUM_MOD;
	}
	descs[prev_idx] = desc();

	return true;
}

packet ring::pull(packet *pool) {
	uint16_t prev_idx;
	{
		std::lock_guard<std::mutex> lock(proc_mtx);

		if(descs[proc_idx].len == 0) {
			return packet();
		}

		prev_idx = proc_idx;
		proc_idx = (proc_idx + 1) & NUM_MOD;
	}
	descs[prev_idx].entry = pool + descs[prev_idx].id;
	packet ret = *(descs[prev_idx].entry);
	descs[prev_idx].entry->len = 0;
	descs[prev_idx].len = 0;

	return ret;
}
