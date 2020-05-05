#include "packet.hpp"

packet::packet() {
	len = 0;
}

packet::packet(uint32_t this_id, const char* this_dummy) {
	id = this_id;
	len = sizeof(packet);
	strncpy(dummy, this_dummy, DUMMY_SIZE);
}

void packet::print() {
	std::cout << "id: " << id << ", len: " << len << ", dummy: "<< dummy << ", verification: 0x" << std::hex << verification << std::endl;
}

void packet::set_verification() {
	verification = id ^ 0xffffffff;
}
