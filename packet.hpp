#pragma once

#include "bindcpu.hpp"

constexpr int32_t SIZE_PACKET = 64;
constexpr bool IS_PSMALL = SIZE_PACKET < 32;
constexpr int32_t NUM_LOOP = SIZE_PACKET / (IS_PSMALL ? 16 : 32) + (SIZE_PACKET % 32 == 0 ? 0 : 1);

constexpr int32_t NUM_PACKET = 100000000;

struct packet {
    int32_t packet_id;
    int32_t packet_len;
    uint64_t rx_time; //rxプロセス用のタイムスタンプ
    uint64_t nf_time; //nfプロセス用のタイムスタンプ、今は使っていない
    uint64_t tx_time; //txプロセス用のタイムスタンプ、今は使っていない
    char dummy[SIZE_PACKET - sizeof(int32_t) * 2 - sizeof(uint64_t) * 3];
};

#if 1
constexpr int32_t SIZE_BUFFER = 2176;
constexpr int32_t PACKET_BUFFER_PADDING = 128;
#else
constexpr int32_t PACKET_BUFFER_PADDING = VIO_HEADER_SIZE == 0 ? 0 : 64;
constexpr int32_t SIZE_BUFFER = sizeof(packet) + PACKET_BUFFER_PADDING;
#endif

struct buf_hdr {
    char id_addr[BUF_HEADER_SIZE / 2];
    char len_addr[BUF_HEADER_SIZE / 2];
};

struct buf {
    buf_hdr header;
#if BUF_HEADER_SIZE > 0 && BUF_HEADER_SIZE < 128
    char padding_extra[128 - BUF_HEADER_SIZE];
#endif
    char padding[PACKET_BUFFER_PADDING];
    char addr[SIZE_BUFFER - PACKET_BUFFER_PADDING];
};

void do_none() {
}

void print(packet *p) {
    std::printf("id: %d, len: %d, dummy: %s\n", p->packet_id, p->packet_len, p->dummy);
}

packet *get_packet_addr(buf *buffer) {
    return (packet *) buffer->addr;
}

static inline void *__movsb(void *d, const void *s, size_t n) {
    asm volatile ("rep movsb"
            : "=D" (d),
    "=S" (s),
    "=c" (n)
            : "0" (d),
    "1" (s),
    "2" (n)
            : "memory");
    return d;
}

void set_id(buf *buffer, int32_t id) {
    *(int32_t *) (buffer->header.id_addr) = id;
    memset((char *) buffer->header.id_addr + 4, id, BUF_HEADER_SIZE / 2 - 4);
}

int32_t get_id(buf *buffer) {
    return *(int32_t *) buffer->header.id_addr;
}

void set_len(buf *buffer, int32_t len) {
    *(int32_t *) (buffer->header.len_addr) = len;
    memset((char *) buffer->header.len_addr + 4, len, BUF_HEADER_SIZE / 2 - 4);
}

int32_t get_len(buf *buffer) {
    return *(int32_t *) buffer->header.len_addr;
}

constexpr int VIO_HEADER_OFFSET = PACKET_BUFFER_PADDING - VIO_HEADER_SIZE;

void load_vio_header(buf *buffer, void *dest) {
#if VIO_HEADER_SIZE > 0
    memcpy(dest, (&buffer->padding) + VIO_HEADER_OFFSET, VIO_HEADER_SIZE);
#endif
}

void store_vio_header(buf *buffer, void *src) {
#if VIO_HEADER_SIZE > 0
    memcpy((&buffer->padding) + VIO_HEADER_OFFSET, src, VIO_HEADER_SIZE);
#endif
}
