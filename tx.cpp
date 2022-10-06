#include "virtqueue.hpp"
#include "shm.hpp"

namespace {
    void recv_packet(vq *vq_tx, buffer_pool *pool_guest, info_opt opt) {
#ifdef CPU_BIND
        bind_core(2);
#endif

        int32_t num_fin = opt.size_batch;
        bool is_stream = opt.stream == ON;

        auto *pool = (struct buffer_pool *) malloc(sizeof(buffer_pool));
        init(pool);
        buf **recv_addrs = new buf *[num_fin];

        int last_pring_idx = 0;
        buf **dummy_physical_ring = new buf *[128];
        for (int j = 0; j < 128; j++) {
            dummy_physical_ring[j] = get_buffer(pool);
        }

        for (int i = NUM_PACKET; 0 < i; i -= num_fin) {
            // 受信パケット数の決定
            if (i < num_fin) {
                num_fin = i;
            }

            // パケット受信
            for (int j = 0; j < num_fin; j++) {
#ifdef RANDOM_TX
                recv_addrs[j] = &pool_tx_addr[local_pool_index + (int) ids[j]];
#else
                add_to_cache(pool, dummy_physical_ring[last_pring_idx]);
                dummy_physical_ring[last_pring_idx] = get_buffer(pool);
                recv_addrs[j] = dummy_physical_ring[last_pring_idx];
                last_pring_idx = (last_pring_idx + 1) % 128;
//                recv_addrs[j] = get_buffer(pool);
#endif
            }

            send_guest_to_tx(vq_tx, recv_addrs, pool_guest, num_fin, is_stream);
//            for (int j = 0; j < num_fin; j++) {
//                add_to_cache(pool, recv_addrs[j]);
//            }

#ifdef PRINT
            if ((i & 8388607) == 0) {
                print((packet *) &recv_addrs[0]->addr);
            }
#endif
        }

        free(pool);
        delete[](recv_addrs);
        delete[](dummy_physical_ring);
    }
}

int main(int argc, char **argv) {

    // 初期設定
    int bfd = open_shmfile("shm_buf", SIZE_SHM, false);
    auto *pool = (buffer_pool *) mmap(nullptr, SIZE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, bfd, 0);
    auto *descs_rx = (desc*)(pool + 1);
    auto *descs_tx = (desc*)(descs_rx + VQ_ENYRY_NUM);

    vq vq_rx{};
    init_ring(&vq_rx, descs_tx);

    info_opt opt = get_opt(argc, argv);

    bool *flag = (bool *) (descs_tx + VQ_ENYRY_NUM);

    *flag = true;

    // 計測開始
    std::chrono::system_clock::time_point start, end;
    start = std::chrono::system_clock::now();

    recv_packet(&vq_rx, pool, opt);

    // 計測終了
    end = std::chrono::system_clock::now();

    double elapsed = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
    double second = elapsed / 1000.0;
    std::printf("result: %.3fsec (%.3fMpps)\n", second, NUM_PACKET / second / 1000000);

    shm_unlink("shm_buf");

    return 0;
}
