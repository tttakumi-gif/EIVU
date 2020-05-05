#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include <iostream>
#include <cstring>
#include <typeinfo>

#define FILE_MODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)

int open_shmfile(std::string, uint32_t, bool);
