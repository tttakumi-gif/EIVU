#include "virtqueue.hpp"
#include "shm.hpp"

namespace {
    void send_packet(vq *vq_rx, guest_buffer_pool *pool_guest, info_opt opt) {
#ifdef CPU_BIND
        bind_core(0);
#endif
        assert((intptr_t(pool_guest) & 63) == 0);

        int32_t i = NUM_PACKET;
        int32_t num_fin = BATCH_SIZE_RX;
        bool is_stream = opt.stream == ON;

        auto *pool = new host_buffer_pool();
        init(pool);
        buf **send_addrs = new buf *[num_fin];

        int last_pring_idx = 0;
        buf **dummy_physical_ring = new buf *[PRING_SIZE_RX];
        for (int j = 0; j < PRING_SIZE_RX; j++) {
            dummy_physical_ring[j] = get_buffer(pool);
        }

	try {
            while (0 < i) {
                if (i < num_fin) {
                    num_fin = i;
                }

                for (int j = 0; j < num_fin; j++, i--) {
#ifdef RANDOM_RX
                    send_addrs[j] = &pool->buffers[pool->last_pool_idx / 32 * 32 + (int) ids[j]];
                    pool->last_pool_idx = (pool->last_pool_idx + 1) % HOST_POOL_ENTRY_NUM;
#else
                    send_addrs[j] = dummy_physical_ring[last_pring_idx];
                    dummy_physical_ring[last_pring_idx] = get_buffer(pool);
                    last_pring_idx = (last_pring_idx + 1) % PRING_SIZE_RX;
//                  send_addrs[j] = get_buffer(pool);
#endif
                    ((packet *) (send_addrs[j]->addr))->packet_id = i;
                    ((packet *) (send_addrs[j]->addr))->packet_len = SIZE_PACKET;
                }

                send_rx_to_guest(vq_rx, send_addrs, pool_guest, num_fin, is_stream); 
                for (int j = 0; j < num_fin; j++) {
                    add_to_cache(pool, send_addrs[j]);
                }
            }
	} catch (std::exception& e) {
            std::cerr << "[rx] ERROR: " << e.what() << " terminating..." << std::endl;
	}

        delete pool;
        delete[] send_addrs;
        delete[] dummy_physical_ring;
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
    static_assert(BATCH_SIZE_RX <= VQ_ENTRY_NUM);

    {
        constexpr int size = sizeof(desc) * VQ_ENTRY_NUM * 2 + sizeof(buf) * HOST_POOL_ENTRY_NUM + sizeof(volatile bool);
#ifdef PRINT
        std::cout << "size: " << size << std::endl;
        std::cout << "packet size: " << sizeof(packet) << ", " << SIZE_PACKET << std::endl;
        std::cout << "packet buffer size: " << sizeof(buf) << std::endl;
        std::cout << "desc size: " << sizeof(desc) << std::endl;
#endif
        static_assert(size <= SIZE_SHM, "over packet size");
    }

    // Initialize
    int bfd = open_shmfile(SHM_FILE, SIZE_SHM, false);
    auto *pool = (guest_buffer_pool *) mmap(nullptr, SIZE_SHM, PROT_READ | PROT_WRITE, SHM_FLAG, bfd, 0);
    auto *descs_rx = (desc *) (pool + 1);
    auto *descs_tx = (desc *) (descs_rx + VQ_ENTRY_NUM);

    vq vq_rx{};
    init_ring(&vq_rx, descs_rx);

    info_opt opt = get_opt(argc, argv);

    volatile bool *flag = (volatile bool *) (descs_tx + VQ_ENTRY_NUM);
    *flag = false;

    init_resource();

    while (!*flag) {
    }

    // Start the main processing
    send_packet(&vq_rx, pool, opt);

#ifdef HUGE_PAGE
    close_shmfile(bfd);
#else
    shm_unlink(SHM_FILE);
#endif

    return 0;
}

