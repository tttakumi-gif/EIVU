#include "virtqueue.hpp"
#include "shm.hpp"

namespace {
    void send_packet(vq *vq_rx, buffer_pool *pool_guest, info_opt opt) {
#ifdef CPU_BIND
        bind_core(0);
#endif
        assert((intptr_t(pool_guest) & 63) == 0);

        int32_t i = NUM_PACKET;
        int32_t num_fin = opt.size_batch;
        bool is_stream = opt.stream == ON;

        auto *pool = (struct buffer_pool *) malloc(sizeof(buffer_pool));
        init(pool);
        buf **send_addrs = new buf *[num_fin];

        int last_pring_idx = 0;
        int pring_size = 128;
        buf **dummy_physical_ring = new buf *[pring_size];
        for (int j = 0; j < pring_size; j++) {
            dummy_physical_ring[j] = get_buffer(pool);
        }

        while (0 < i) {
            // 受信パケット数の決定
            if (i < num_fin) {
                num_fin = i;
            }

            for (int j = 0; j < num_fin; j++, i--) {
#ifdef RANDOM_RX
                send_addrs[j] = &pool_rx_addr[local_pool_index + (int) ids[j]];
#else
                add_to_cache(pool, dummy_physical_ring[last_pring_idx]);
                dummy_physical_ring[last_pring_idx] = get_buffer(pool);
                send_addrs[j] = dummy_physical_ring[last_pring_idx];
                last_pring_idx = (last_pring_idx + 1) % pring_size;
//                send_addrs[j] = get_buffer(pool);
#endif
                ((packet *) (send_addrs[j]->addr))->packet_id = i;
                ((packet *) (send_addrs[j]->addr))->packet_len = SIZE_PACKET;
            }

            send_rx_to_guest(vq_rx, send_addrs, pool_guest, num_fin, is_stream);
//            for (int j = 0; j < num_fin; j++) {
//                add_to_cache(pool, send_addrs[j]);
//            }
        }

        free(pool);
        delete[](send_addrs);
        delete[](dummy_physical_ring);
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
        constexpr int size = sizeof(desc) * VQ_ENYRY_NUM * 2 + sizeof(buf) * POOL_ENTRY_NUM + sizeof(volatile bool);
#ifdef PRINT
        std::cout << "size: " << size << std::endl;
        std::cout << "packet size: " << sizeof(packet) << ", " << SIZE_PACKET << std::endl;
        std::cout << "packet buffer size: " << sizeof(buf) << std::endl;
        std::cout << "desc size: " << sizeof(desc) << std::endl;
#endif
        static_assert(size <= SIZE_SHM, "over packet size");
    }

    // 初期設定
    int bfd = open_shmfile(SHM_FILE, SIZE_SHM, false);
    auto *pool = (buffer_pool *) mmap(nullptr, SIZE_SHM, PROT_READ | PROT_WRITE, SHM_FLAG, bfd, 0);
    auto *descs_rx = (desc *) (pool + 1);
    auto *descs_tx = (desc *) (descs_rx + VQ_ENYRY_NUM);

    vq vq_rx{};
    init_ring(&vq_rx, descs_rx);

    info_opt opt = get_opt(argc, argv);

    volatile bool *flag = (volatile bool *) (descs_tx + VQ_ENYRY_NUM);
    *flag = false;

    init_resource();

    while (!*flag) {
    }

    // 送受信開始
    send_packet(&vq_rx, pool, opt);

#ifdef HUGE_PAGE
    close_shmfile(bfd);
#else
    shm_unlink(SHM_FILE);
#endif

    return 0;
}

