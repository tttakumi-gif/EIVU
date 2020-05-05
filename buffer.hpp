#include <iostream>
#include <vector>

#include "packet.hpp"

#define SIZE_RING 16
#define SIZE_POOL 32

class desc {
public:
	uint16_t id;
	uint16_t len;
	packet *entry;

	void set_param(packet, uint16_t);
};

class ring {
private:
	bool ifull;
	bool pfull;
	bool dfull;
public:
	uint16_t size;
	uint16_t rsrv_idx;
	uint16_t recv_idx;
	uint16_t proc_idx;
	desc descs[SIZE_RING];

	ring();

	uint16_t get_index(packet*, int);
	bool push(packet, packet*, int);
	bool dinit();
	packet pull(packet*);
};
