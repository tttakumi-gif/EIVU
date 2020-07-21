#pragma once

#include "header.hpp"

#define FILE_MODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
//#define SIZE_SHM 65536
#define SIZE_SHM 75536

int open_shmfile(std::string, uint32_t, bool);
