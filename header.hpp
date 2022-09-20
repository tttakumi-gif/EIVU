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
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/times.h>
#include <immintrin.h>
#include <x86intrin.h>

#include <cstring>
#include <typeinfo>
#include <iostream>
#include <chrono>

//#define VQ_ENTRY_SIZE 8
#define VQ_ENTRY_SIZE 16
//#define VQ_ENTRY_SIZE 64
//#define VQ_ENTRY_SIZE 128

//#define MBUF_HEADER_SIZE 0
#define MBUF_HEADER_SIZE 128
//#define MBUF_HEADER_SIZE 1024

//#define ZERO_COPY_RX
//#define ZERO_COPY_TX

// #define RANDOM

#if 0
#define STRIDE_VQ
constexpr int32_t NUM_VQ_STRIDE = 4;
#else
// #define SKIP_INDEX 
#endif

// #define PRINT
