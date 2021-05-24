#pragma once

#include <sched.h>
#include <string.h>
#include <iostream>
#include <bitset>
#include <thread>
#include <atomic>
#include <mutex>
#include <sys/syscall.h>
//#include <omp.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/times.h>
#include <sys/vtimes.h>
#include <sys/vtimes.h>
#include <immintrin.h>

#include <cstring>
#include <typeinfo>
#include <iostream>
#include <chrono>


#if 1
#define AVOID_CLT
#define AVOID_SRV
#elif 0
#define READ_SRV
#endif

