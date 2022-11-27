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

#define VIRTIO_HEADER_SIZE 16

//#define ZERO_COPY_RX
//#define ZERO_COPY_TX

//#define SIMD_COPY
//#define NON_TEMPORAL_COPY

//#define READ_HEADER4_RX
//#define READ_HEADER4_NF
//#define READ_BODY4_NF
//#define READ_HEADER4_Tx

//#define WRITE_HEADER4_RX
//#define WRITE_HEADER4_NF
//#define WRITE_BODY4_NF
//#define WRITE_HEADER4_Tx

//#define RANDOM_RX
//#define RANDOM_NF
//#define RANDOM_TX

//#define SKIP_INDEX_RX
//#define SKIP_INDEX_NF
//#define SKIP_INDEX_TX

#define PRING_SIZE_RX 128
#define PRING_SIZE_TX 128

//#define HUGE_PAGE

//#define PRINT
