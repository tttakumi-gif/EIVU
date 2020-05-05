#include "packet.hpp"

packet::packet(uint32_t this_id, uint16_t this_len, char* this_dummy) {
	id = this_id;
	len = this_len;
	strncpy(dummy, this_dummy, DUMMY_SIZE);
}

void packet::print() {
	std::cout << "id = " << id << ", len = " << len << ", dummy = "<< dummy << ", verification = 0x" << std::hex << verification << std::endl;
}

void packet::set_verification() {
	verification = id ^ 0xffffffff;
}
