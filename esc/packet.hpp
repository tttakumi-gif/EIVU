#pragma once

#include <string.h>
#include <iostream>
#include <bitset>

#define DUMMY_SIZE 22

class packet {
public:
	uint32_t id;
	uint16_t len;
	char dummy[DUMMY_SIZE];
	uint32_t verification;

	packet(uint32_t, uint16_t, char*);

	void print();
	void set_verification();
};
