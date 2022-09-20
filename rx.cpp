#include "buffer.hpp"
#include "shm.hpp"

namespace {
    void send_packet(vq *vq_rx_to_guest, buf *pool_guest_addr, info_opt opt) {
#ifdef CPU_BIND
        bind_core(0);
#endif
        assert((intptr_t(pool_guest_addr) & 63) == 0);

        int32_t i = NUM_PACKET;
        // int32_t num_fin = opt.size_batch;
        int32_t num_fin = 32;
        bool is_stream = (opt.stream == ON) ? true : false;

        int local_pool_index = 0;
        buf *pool_rx_addr = new(std::align_val_t{64}) buf[POOL_ENTRY_NUM];
        buf **send_addrs = new buf *[num_fin];
        void **send_addrs_dest = new void *[num_fin];

        while (0 < i) {
            // 受信パケット数の決定
            if (unlikely(i < num_fin)) {
                num_fin = i;
            }

            for (int j = 0; j < num_fin; j++, i--) {
#ifdef RANDOM
                send_addrs[j] = &pool_rx_addr[local_pool_index + (int) ids[j]];
#else
                send_addrs[j] = &pool_rx_addr[local_pool_index + j];
#endif
                ((packet *) (send_addrs[j]->addr))->packet_id = i;
                ((packet *) (send_addrs[j]->addr))->packet_len = SIZE_PACKET;

#ifdef RANDOM
                int offset = sizeof(buf) * (vq_rx_to_guest->last_pool_idx + ids[j]) + sizeof(mbuf_header) +
                             PACKET_BUFFER_PADDING;
#else
                int offset = sizeof(buf) * (vq_rx_to_guest->last_pool_idx + j) + sizeof(mbuf_header) + PACKET_BUFFER_PADDING;
#endif
                send_addrs_dest[j] = (void *) ((char *) pool_guest_addr + offset);
            }

            send_rx_to_guest(vq_rx_to_guest, send_addrs, send_addrs_dest, num_fin, is_stream);
            vq_rx_to_guest->last_pool_idx = (vq_rx_to_guest->last_pool_idx + num_fin) % POOL_ENTRY_NUM;

            local_pool_index += num_fin;
            if (POOL_ENTRY_NUM <= local_pool_index) {
                local_pool_index = 0;
            }
        }

#ifndef ZERO_COPY_RX
        delete[](send_addrs);
        delete[](pool_rx_addr);
#endif
    }

    void init_resource() {
#if INFO_CPU == PROC_CLT_S || INFO_CPU == PROC_CLT_R || INFO_CPU == PROC_SRV
        init_p();
#elif INFO_CPU == TOTAL_CLT || INFO_CPU == TOTAL_SRV
        init_t();
#endif
    }
}

int main(int argc, char **argv) {
    {
        constexpr int size = sizeof(vq) * 2 + sizeof(buf) * POOL_ENTRY_NUM + sizeof(volatile bool);
#ifdef PRINT
        std::cout << "size: " << size << std::endl;
        std::cout << "packet size: " << sizeof(packet) << ", " << SIZE_PACKET << std::endl;
        std::cout << "packet buffer size: " << sizeof(buf) << std::endl;
        std::cout << "desc size: " << sizeof(desc) << std::endl;
        std::cout << "vq size: " << sizeof(vq) << std::endl;
#endif
        static_assert(size <= SIZE_SHM, "over packet size");
    }

    // 初期設定
    int bfd = open_shmfile("shm_buf", SIZE_SHM, false);
    buf *pool_guest_addr = (buf *) mmap(NULL, SIZE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, bfd, 0);
    vq *vq_rx_to_guest = (vq *) (pool_guest_addr + POOL_ENTRY_NUM);
    vq *vq_guest_to_tx = (vq *) (vq_rx_to_guest + 1);

    info_opt opt = get_opt(argc, argv);

    volatile bool *flag = (volatile bool *) (vq_guest_to_tx + 1);
    *flag = false;

#ifdef PRINT
    std::printf("clt: \n  - pool_guest_addr: %p\n  - RxRing: %p\n  - TxRing: %p\n  - end: %p\n", pool_guest_addr, vq_rx_to_guest, vq_guest_to_tx, flag);
#endif

    init_resource();

    while (!*flag) {
    }

    // 送受信開始
    send_packet(vq_rx_to_guest, pool_guest_addr, opt);

    shm_unlink("shm_buf");

    return 0;
}

