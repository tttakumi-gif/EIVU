#include "shm.hpp"

int open_shmfile(const std::string& filename, uint64_t size, bool is_create) {
    int fd;
#ifdef HUGE_PAGE
    if (is_create) {
        fd = open(filename.c_str(), O_CREAT | O_RDWR, FILE_MODE);
    } else {
        fd = open(filename.c_str(), O_RDWR, FILE_MODE);
    }
#else
    if (is_create) {
        fd = shm_open(filename.c_str(), O_CREAT | O_RDWR, FILE_MODE);
    } else {
        fd = shm_open(filename.c_str(), O_RDWR, FILE_MODE);
    }
#endif

    int fde = errno;
    if (fd == -1) {
        fprintf(stderr, "shm_open failed. err = %s\n\n", std::strerror(fde));
        exit(0);
    }

#ifndef HUGE_PAGE
    int no = ftruncate(fd, size);
    assert(no != -1);
#endif

    return fd;
}

#ifdef HUGE_PAGE
int close_shmfile(int fd) {
    return close(fd);
}
#endif
