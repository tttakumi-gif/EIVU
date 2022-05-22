#pragma once

#include <sched.h>
#include <string.h>
#include <iostream>
#include <bitset>
#include <thread>
#include <atomic>
#include <mutex>
#include <sys/syscall.h>
#include <time.h>
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

//#define VQENTRY_SIZE 8 
#define VQENTRY_SIZE 16
//#define VQENTRY_SIZE 64
//#define VQENTRY_SIZE 128

//#define HEADER_SIZE 0
//#define HEADER_SIZE 64
#define HEADER_SIZE 128
//#define HEADER_SIZE 1024 

//constexpr int32_t SIZE_PADDING = 0;
constexpr int32_t SIZE_PADDING = 128;

constexpr int32_t NUM_VQ_STRIDE = 4096;

//#define SKIP_CLT
#if 0
#if 1
//#define ZERO_COPY
//#define AVOID_SRV
//#define AVOID_TX
#else 
#define READ_SRV
#endif
#endif

#define RANDOM

#if 1
//#define STRIDE_VQ
#else
//#define SKIP_INDEX 
#endif

//#define VERIFICATION
