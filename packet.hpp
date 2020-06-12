#include <string.h>
#include <iostream>
#include <bitset>
#include <thread>
#include <atomic>
#include <mutex>

//#define NUM_PACKET 5000000
//#define NUM_PACKET 10000000
//#define NUM_PACKET 20000000
#define NUM_PACKET 100000000
#define SIZE_PACKET 16

constexpr uint16_t DUMMY_SIZE = SIZE_PACKET - sizeof(uint32_t) - sizeof(uint32_t) - sizeof(uint16_t);

class packet {
public:
	uint32_t id;
	uint8_t len;
	char dummy[DUMMY_SIZE];
	uint_fast32_t verification;

	packet();
	packet(uint32_t);
	packet(uint32_t, const char*);

	void print();
	void set_verification();
};
#include "cpacket.hpp"
