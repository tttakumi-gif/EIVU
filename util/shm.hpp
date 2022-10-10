#pragma once

#include "../header.hpp"

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define SHM_FLAG (MAP_SHARED | MAP_POPULATE | MAP_HUGETLB)
#define SHM_FILE "/mnt/huge/mock-vhost-user"

#define SIZE_SHM 5000000000

int open_shmfile(const std::string&, uint64_t, bool);
int close_shmfile(int);
