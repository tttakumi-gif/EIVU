#pragma once
//#include "packet.hpp"

inline packet::packet() {
	len = 0;
}

inline packet::packet(uint32_t this_id) {
	id = this_id;
	len = sizeof(packet);
}

inline packet::packet(uint32_t this_id, const char* this_dummy) {
	id = this_id;
	len = sizeof(packet);
	strncpy(dummy, this_dummy, DUMMY_SIZE);
}

inline void packet::print() {
	printf("id: %d, len: %d, dummy: %s, verification: 0x%x\n", id, len, dummy, verification);
	//std::cout << "id: " << std::dec << id << ", len: " << std::dec << len << ", dummy: "<< dummy << ", verification: 0x" << std::hex << verification << std::endl;
}

inline void packet::set_verification() {
	verification = id ^ 0xffffffff;
}
