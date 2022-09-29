#include "buffer.hpp"
#include "shm.hpp"

namespace {
    void recv_packet(vq *vq_guest_to_tx, buf *pool_guest_addr, info_opt opt) {
#ifdef CPU_BIND
        bind_core(2);
#endif

        int32_t num_fin = opt.size_batch;
        bool is_stream = opt.stream == ON;

        int local_pool_index = 0;
        buf *pool_tx_addr = new(std::align_val_t{64}) buf[POOL_ENTRY_NUM];
        buf **recv_addrs = new buf *[num_fin];

        for (int i = NUM_PACKET; 0 < i; i -= num_fin) {
            // 受信パケット数の決定
            if (unlikely(i < num_fin)) {
                num_fin = i;
            }

            // パケット受信
            for (int j = 0; j < num_fin; j++) {
#ifdef RANDOM_TX
                recv_addrs[j] = &pool_tx_addr[local_pool_index + (int) ids[j]];
#else
                recv_addrs[j] = &pool_tx_addr[local_pool_index + j];
#endif
            }

            send_guest_to_tx(vq_guest_to_tx, recv_addrs, pool_guest_addr, num_fin, is_stream);

            local_pool_index += num_fin;
            if (POOL_ENTRY_NUM <= local_pool_index) {
                local_pool_index = 0;
            }


            if (unlikely((i & 8388607) == 0)) {
#ifdef PRINT
            print((packet*)&recv_addrs[0]->addr);
#endif
            }
        }

#ifndef ZERO_COPY_TX
        delete[](recv_addrs);
        delete[](pool_tx_addr);
#endif
    }
}

int main(int argc, char **argv) {

    // 初期設定
    int bfd = open_shmfile("shm_buf", SIZE_SHM, false);
    buf *pool_guest_addr = (buf *) mmap(NULL, SIZE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, bfd, 0);
    vq *vq_rx_to_guest = (vq *) (pool_guest_addr + POOL_ENTRY_NUM);
    vq *vq_guest_to_tx = (vq *) (vq_rx_to_guest + 1);

    info_opt opt = get_opt(argc, argv);

    bool *flag = (bool *) (vq_guest_to_tx + 1);

#ifdef PRINT
    std::printf("recv: \n  - pool_guest_addr: %p\n  - RxRing: %p\n  - TxRing: %p\n  - end: %p\n", pool_guest_addr, vq_rx_to_guest, vq_guest_to_tx, flag);
    //std::printf("recv: \n  - pool_guest_addr: %p\n  - RxRing: %p\n  - TxRing: %p\n  - end: %p\n", &vq_guest_to_tx->size, &vq_guest_to_tx->last_avail_idx, &vq_guest_to_tx->last_used_idx, &vq_guest_to_tx->last_pool_idx);
#endif

    *flag = true;

    // 計測開始
    std::chrono::system_clock::time_point start, end;
    start = std::chrono::system_clock::now();

    recv_packet(vq_guest_to_tx, pool_guest_addr, opt);

    // 計測終了
    end = std::chrono::system_clock::now();

    double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    double second = elapsed / 1000;
    std::printf("result: %.3fsec (%.3fMpps)\n", second, NUM_PACKET / second / 1000000);

    shm_unlink("shm_buf");

    return 0;
}
