#pragma once

#include <string.h>
#include <iostream>
#include <bitset>
#include <thread>
#include <atomic>
#include <mutex>

#define DUMMY_SIZE 22
//#define NUM_PACKET 10000000
#define NUM_PACKET 100000000

class packet {
public:
	uint32_t id;
	uint16_t len;
	char dummy[DUMMY_SIZE];
	uint32_t verification;

	packet();
	packet(uint32_t, const char*);

	void print();
	void set_verification();
};
