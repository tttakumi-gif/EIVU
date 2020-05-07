#include <iostream>
#include <vector>

#include "packet.hpp"

#define SIZE_RING 16
#define NUM_MOD (SIZE_RING - 1)
#define SIZE_POOL 32

class desc {
public:
	uint16_t id;
	uint16_t len;
	bool is_used;
	packet *entry;

	void set_param(packet, uint16_t);
};

class ring {
private:
public:
	uint16_t size;
	std::atomic<uint16_t> rsrv_idx;
	std::atomic<uint16_t> recv_idx;
	std::atomic<uint16_t> proc_idx;
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
};
