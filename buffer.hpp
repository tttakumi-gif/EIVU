#include <iostream>
#include <vector>

#include "packet.hpp"

#define NUM_THREAD 2
#define SIZE_RING 16
#define NUM_MOD (SIZE_RING - 1)
#define SIZE_POOL 32

enum dstatus {
	INIT,
	PUSH,
	PULL,
};

class desc {
public:
	dstatus status;
	uint16_t id;
	uint16_t len;
	bool is_used;
	packet *entry;

	desc();
	desc(dstatus);

	void set_param(packet, uint16_t, dstatus);
	void delete_info(dstatus);
};

class ring {
private:
	std::mutex rsrv_mtx;
	std::mutex recv_mtx;
	std::mutex proc_mtx;
public:
#if 0
	std::atomic<uint16_t> rsrv_idx;
	std::atomic<uint16_t> recv_idx;
	std::atomic<uint16_t> proc_idx;
#else
	uint16_t rsrv_idx;
	uint16_t recv_idx;
	uint16_t proc_idx;
#endif
	uint16_t size;
	desc descs[SIZE_RING];
	ring();
	ring(const ring&);
	ring(ring&&);
	ring& operator=(const ring&);
	ring&& operator=(ring&&);

	uint16_t get_index(packet*, int);
	bool push(packet, packet*, int);
	bool dinit();
	packet pull(packet*);
	void init_descs();
};

void set_packet_nums(uint32_t*);
