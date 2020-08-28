#include "shm.hpp"

int open_shmfile(std::string filename, uint32_t size, bool is_create) {
	int fd;
	if(is_create) {
		fd = shm_open(filename.c_str(), O_CREAT | O_RDWR, FILE_MODE);
	}
	else {
		fd = shm_open(filename.c_str(), O_RDWR, FILE_MODE);
	}

	int fde = errno;
	if(fd == -1) {
		fprintf(stderr, "shm_open failed. err = %s\n\n", std::strerror(fde));
		exit(0);
		//return -1;
	}

	//int ret = ftruncate(fd, size);
	//int rete = errno;
	int no = ftruncate(fd, size);
	assert(no != -1);

	return fd;
}
