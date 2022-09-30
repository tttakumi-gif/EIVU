#include "virtqueue.hpp"
#include "shm.hpp"

namespace {
    void recv_packet(vq *vq_guest_to_tx, buffer_pool *pool_guest, info_opt opt) {
#ifdef CPU_BIND
        bind_core(2);
#endif

        int32_t num_fin = opt.size_batch;
        bool is_stream = opt.stream == ON;

        auto *pool = (struct buffer_pool *) malloc(sizeof(buffer_pool));
        init(pool);
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
                recv_addrs[j] = get_buffer(pool);
#endif
            }

            send_guest_to_tx(vq_guest_to_tx, recv_addrs, pool_guest, num_fin, is_stream);
            for (int j = 0; j < num_fin; j++) {
                add_to_cache(pool, recv_addrs[j]);
            }

#ifdef PRINT
            if (unlikely((i & 8388607) == 0)) {
                print((packet *) &recv_addrs[0]->addr);
            }
#endif
        }

        free(pool);
        delete[](recv_addrs);
    }
}

int main(int argc, char **argv) {

    // 初期設定
    int bfd = open_shmfile("shm_buf", SIZE_SHM, false);
    auto *pool = (buffer_pool *) mmap(nullptr, SIZE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, bfd, 0);
    vq *vq_rx_to_guest = (vq *) (pool + 1);
    vq *vq_guest_to_tx = (vq *) (vq_rx_to_guest + 1);

    info_opt opt = get_opt(argc, argv);

    bool *flag = (bool *) (vq_guest_to_tx + 1);

#ifdef PRINT
    std::printf("recv: \n  - pool_guest_addr: %p\n  - RxRing: %p\n  - TxRing: %p\n  - end: %p\n", pool,
                vq_rx_to_guest, vq_guest_to_tx, flag);
#endif

    *flag = true;

    // 計測開始
    std::chrono::system_clock::time_point start, end;
    start = std::chrono::system_clock::now();

    recv_packet(vq_guest_to_tx, pool, opt);

    // 計測終了
    end = std::chrono::system_clock::now();

    double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    double second = elapsed / 1000;
    std::printf("result: %.3fsec (%.3fMpps)\n", second, NUM_PACKET / second / 1000000);

    shm_unlink("shm_buf");

    return 0;
}
