#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include <iostream>
#include <cstring>
#include <typeinfo>

#define FILE_MODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define SIZE_SHM 65536

int open_shmfile(std::string, uint32_t, bool);
