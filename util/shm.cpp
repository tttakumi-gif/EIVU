#include "shm.hpp"

int open_shmfile(const std::string& filename, uint64_t size, bool is_create) {
    int fd;
    if (is_create) {
        fd = open(filename.c_str(), O_CREAT | O_RDWR, FILE_MODE);
    } else {
        fd = open(filename.c_str(), O_RDWR, FILE_MODE);
    }

    int fde = errno;
    if (fd == -1) {
        fprintf(stderr, "shm_open failed. err = %s\n\n", std::strerror(fde));
        exit(0);
    }

//    int no = ftruncate(fd, size);
//    assert(no != -1);

    return fd;
}

int close_shmfile(int fd) {
    return close(fd);
}
