#pragma once

#include "../header.hpp"

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

#ifdef HUGE_PAGE
#define SHM_FLAG (MAP_SHARED | MAP_POPULATE | MAP_HUGETLB)
#define SHM_FILE "/mnt/hugepages/eivu"
#else
#define SHM_FLAG (MAP_SHARED)
#define SHM_FILE "eivu"
#endif

#define SIZE_SHM 5000000000

int open_shmfile(const std::string&, uint64_t, bool);
#ifdef HUGE_PAGE
int close_shmfile(int);
#endif
