#include <iostream>

#include "packet.hpp"

#define SIZE_RING 16

class desc {
public:
	uint16_t id;
	uint16_t len;
	packet *buf;

	void set_param(packet, uint16_t);
};

class ring {
private:
	uint16_t id;
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

	bool push(packet);
	bool dinit();
	bool del();
};
