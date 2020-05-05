#include "buffer.hpp"

void desc::set_param(packet p, uint16_t this_id) {
	id = this_id;
	len = sizeof(p);
	buf = (packet*)malloc(sizeof(packet));
	*buf = p;
}

ring::ring() {
	rsrv_idx = 0;
	recv_idx = 0;
	proc_idx = 0;
	id = 0;
	ifull = false;
	pfull = false;
	dfull = false;
	size = SIZE_RING;
	//descs = (desc*)malloc(size * sizeof(desc));
}

bool ring::push(packet p) {
	if(pfull) {
		puts("perror");
		return false;
	}

	descs[recv_idx].set_param(p, id);
	recv_idx++;
	id++;

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
		puts("ierror");
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

bool ring::del() {
	if(dfull) {
		puts("derror");
		return false;
	}

	free(descs[proc_idx].buf);
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
	return true;
}
