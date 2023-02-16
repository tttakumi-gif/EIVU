#include "vioqueue.hpp"
#include "shm.hpp"

namespace {
    void init_descs(desc *descs) {
        memset(descs, 0, sizeof(desc) * VQ_ENTRY_NUM);
        for (int i = 0; i < VQ_ENTRY_NUM; i++) {
            descs[i].entry_index = static_cast<int16_t>(i);
            descs[i].flags = static_cast<int16_t>(descs[i].flags | USED_FLAG);
        }
    }

    void attach_buffer_to_vq(vq *vioqueue, guest_buffer_pool *pool) {
        assert(VQ_ENTRY_NUM <= GUEST_POOL_ENTRY_NUM);
        for (int i = 0; i < VQ_ENTRY_NUM; i++) {
#ifdef RANDOM_NF
            int entry_index = i / 32 * 32 + ids[i % 32];
            pool->last_pool_idx = (pool->last_pool_idx + 1) % GUEST_POOL_ENTRY_NUM;
#else
            auto entry_index = static_cast<int64_t>(get_buffer_index(pool, get_buffer(pool)));
#endif
            vioqueue->descs[i].entry_index = entry_index;
            set_avail_flag(&vioqueue->descs[i]);
        }
    }

    void guest_process(guest_buffer_pool *pool __attribute__((unused)), buf **pkts __attribute__((unused)), int num_fin __attribute__((unused))) {
        // You can add arbitrary packet processing here
    }

    void rs_packet(vq *vq_rx, vq *vq_tx, guest_buffer_pool *pool, info_opt opt) {
#ifdef CPU_BIND
        bind_core(1);
#endif

        if (opt.process == MOVE) {
            int num_fin = BATCH_SIZE_NF;
            buf *pkts[num_fin];
            memset(pkts, 0, sizeof(buf *) * num_fin);

            try {
                for (int i = NUM_PACKET; 0 < i; i -= num_fin) {
                    if (i < num_fin) {
                        num_fin = i;
                    }

                    guest_recv_process(vq_rx, pool, pkts, num_fin);

                    guest_process(pool, pkts, num_fin);

                    guest_send_process(vq_tx, pool, pkts, num_fin);
                }
            } catch (std::exception &e) {
                std::cerr << "[nf] ERROR: " << e.what() << " terminating..." << std::endl;
            }
        } else {
            exit(1);
        }
    }
}

int main(int argc, char *argv[]) {
    static_assert(VIO_HEADER_SIZE <= PACKET_BUFFER_PADDING);
    static_assert(BATCH_SIZE_NF <= VQ_ENTRY_NUM);

    // Initialize
    int bfd = open_shmfile(SHM_FILE, SIZE_SHM, true);

    auto *pool = (guest_buffer_pool *) mmap(nullptr, SIZE_SHM, PROT_READ | PROT_WRITE, SHM_FLAG, bfd, 0);
    init(pool);

    auto *descs_rx = (desc *) (pool + 1);
    init_descs(descs_rx);
    vq vq_rx{};
    init_ring(&vq_rx, descs_rx);
    attach_buffer_to_vq(&vq_rx, pool);

    auto *descs_tx = (desc *) (descs_rx + VQ_ENTRY_NUM);
    init_descs(descs_tx);
    for (int i = 0; i < VQ_ENTRY_NUM; i++) {
        descs_tx[i].entry_index = -1;
        set_avail_flag(&descs_tx[i]);
    }
    vq vq_tx{};
    init_ring(&vq_tx, descs_tx);

    info_opt opt = get_opt(argc, argv);

    volatile bool *flag = (volatile bool *) (descs_tx + VQ_ENTRY_NUM);
    *flag = false;

    while (!*flag) {
    }

    // Start the main processing
    rs_packet(&vq_rx, &vq_tx, pool, opt);

#ifdef HUGE_PAGE
    close_shmfile(bfd);
#else
    shm_unlink(SHM_FILE);
#endif

    return 0;
}

