#include "buffer.hpp"
#include "shm.hpp"

namespace {
    void rs_packet(vq *vq_rx_to_guest, vq *ring_guest_to_tx, buf *pool_guest_addr, info_opt opt) {
#ifdef CPU_BIND
        bind_core(1);
#endif

        if (opt.process == MOVE) {
            int32_t num_fin = opt.size_batch;

            for (int i = NUM_PACKET; 0 < i; i -= num_fin) {
                if (unlikely(i < num_fin)) {
                    num_fin = i;
                }

                guest_recv_process(vq_rx_to_guest, ring_guest_to_tx, pool_guest_addr, num_fin);
            }
        } else {
            exit(1);
        }
    }
}

int main(int argc, char *argv[]) {
    // 初期設定
    int bfd = open_shmfile("shm_buf", SIZE_SHM, true);
    buf *pool_guest_addr = (buf *) mmap(NULL, SIZE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, bfd, 0);

    vq *vq_rx_to_guest = (vq *) (pool_guest_addr + POOL_ENTRY_NUM);
    init_ring(vq_rx_to_guest);

    vq *vq_guest_to_tx = (vq *) (vq_rx_to_guest + 1);
    init_ring(vq_guest_to_tx);

    memset(pool_guest_addr, 0, sizeof(buf) * POOL_ENTRY_NUM);

    info_opt opt = get_opt(argc, argv);

    volatile bool *flag = (volatile bool *) (vq_guest_to_tx + 1);
    *flag = false;

#ifdef PRINT
    std::printf("srv: \n  - pool_guest_addr: %p\n  - RxRing: %p\n  - TxRing: %p\n  - end: %p\n", pool_guest_addr, vq_rx_to_guest, vq_guest_to_tx, flag);
#endif

    while (!*flag) {
    }

    // 送受信開始
#ifndef SKIP_CLT
    rs_packet(vq_rx_to_guest, vq_guest_to_tx, pool_guest_addr, opt);
#else
    while(*(volatile bool*)flag == true) {
        ;
    }
#endif

    shm_unlink("shm_buf");

    return 0;
}

