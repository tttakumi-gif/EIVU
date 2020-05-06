#include "buffer.hpp"

void desc::set_param(packet p, uint16_t this_id) {
	id = this_id;
	len = sizeof(p);
}

ring::ring() {
	rsrv_idx = 0;
	recv_idx = 0;
	proc_idx = 0;
	ifull = false;
	pfull = true;
	dfull = true;
	size = SIZE_RING;
}

ring::ring(const ring& r) : rsrv_idx(r.rsrv_idx.load()), recv_idx(r.recv_idx.load()), proc_idx(r.proc_idx.load()) {
}

ring::ring(ring&& r) : rsrv_idx(r.rsrv_idx.load()), recv_idx(r.recv_idx.load()), proc_idx(r.proc_idx.load()) {
}

ring& ring::operator=(const ring& r) {
	rsrv_idx = r.rsrv_idx.load();
	recv_idx = r.recv_idx.load();
	proc_idx = r.proc_idx.load();
}

ring&& ring::operator=(ring&& r) {
	rsrv_idx = r.rsrv_idx.load();
	recv_idx = r.recv_idx.load();
	proc_idx = r.proc_idx.load();
}

uint16_t ring::get_index(packet *pool, int qw) {
	for(int i = qw; i < SIZE_POOL * 2; i += 2) {
		if(pool[i].len == 0) {
			return i;
		}
	}
	puts("くぁｗせｄｒｆｔｇｙふじこｌｐ；＠：「」");
	return SIZE_POOL * 2;
}

bool ring::push(packet p, packet *pool, int qw) {
	if(pfull) {
		return false;
	}

	int index = get_index(pool, qw);
	if(SIZE_POOL * 2 <= index) {
		return false;
	}

	pool[index] = p;
	descs[recv_idx].entry = pool + index;
	/*descs[recv_idx].entry->len = 1212;
	puts("a");
	std::cout << pool[index].len << std::endl;
	puts("a");*/
	descs[recv_idx].set_param(pool[index], index);
	recv_idx++;

	if(size <= recv_idx) {
		recv_idx = 0;
	}
	if(recv_idx == rsrv_idx) {
		pfull = true;
	}
	if(dfull) {
		dfull = false;
	}

	return true;
}

bool ring::dinit() {
	if(ifull) {
		return false;
	}

	descs[rsrv_idx] = desc();
	rsrv_idx++;

	if(size <= rsrv_idx) {
		rsrv_idx = 0;
	}
	if(proc_idx == rsrv_idx) {
		ifull = true;
	}
	if(pfull) {
		pfull = false;
	}

	return true;
}

packet ring::pull(packet *pool) {
	if(dfull) {
		return packet();
	}

	descs[proc_idx].entry = pool + descs[proc_idx].id;
	packet ret = *(descs[proc_idx].entry);
	descs[proc_idx].entry->len = 0;
	proc_idx++;

	if(size <= proc_idx) {
		proc_idx = 0;
	}
	if(proc_idx == recv_idx) {
		dfull = true;
	}
	if(ifull) {
		ifull = false;
	}
	return ret;
}
