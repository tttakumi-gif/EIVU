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
#include <x86intrin.h>

#include <cstring>
#include <typeinfo>
#include <iostream>
#include <chrono>


//#define SKIP_CLT
//#define pattern1
#if 0
#if 1
#define AVOID_CLT
#define AVOID_SRV
#else 
#define READ_SRV
#endif
#endif

//#define ZERO_COPY
