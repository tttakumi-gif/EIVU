#include "virtqueue.hpp"
#include "shm.hpp"

namespace {
    void attach_buffer_to_vq(newvq *virtqueue, buffer_pool *pool) {
        assert(VQ_ENYRY_NUM <= POOL_ENTRY_NUM);
        for (int i = 0; i < VQ_ENYRY_NUM; i++) {
#ifdef RANDOM_NF
            int entry_index = i / 32 * 32 + ids[i % 32];
#else
            auto entry_index = static_cast<int64_t>(get_buffer_index(pool, get_buffer(pool)));
#endif
            virtqueue->descs->entry_index = entry_index;
        }
    }

    void rs_packet(newvq *vq_rx_to_guest, newvq *vq_guest_to_tx, buffer_pool *pool, info_opt opt) {
#ifdef CPU_BIND
        bind_core(1);
#endif

        if (opt.process == MOVE) {
            int num_fin = static_cast<int>(opt.size_batch);

            for (int i = NUM_PACKET; 0 < i; i -= num_fin) {
                if (i < num_fin) {
                    num_fin = i;
                }

                guest_recv_process(vq_rx_to_guest, vq_guest_to_tx, pool, num_fin);
            }
        } else {
            exit(1);
        }
    }
}

int main(int argc, char *argv[]) {
    // 初期設定
    int bfd = open_shmfile("shm_buf", SIZE_SHM, true);

    auto *pool = (buffer_pool *) mmap(nullptr, SIZE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, bfd, 0);
    init(pool);

    auto *vq_rx_to_guest = (vq *) (pool + 1);
    init_ring(vq_rx_to_guest);
    newvq v1{};
    init_ring(&v1, vq_rx_to_guest->descs);
    attach_buffer_to_vq(&v1, pool);

    vq *vq_guest_to_tx = (vq *) (vq_rx_to_guest + 1);
    init_ring(vq_guest_to_tx);
    newvq v2{};
    init_ring(&v2, vq_guest_to_tx->descs);

    info_opt opt = get_opt(argc, argv);

    volatile bool *flag = (volatile bool *) (vq_guest_to_tx + 1);
    *flag = false;

#ifdef PRINT
    std::printf("srv: \n  - pool_guest_addr: %p\n  - RxRing: %p\n  - TxRing: %p\n  - end: %p\n", pool,
                vq_rx_to_guest, vq_guest_to_tx, flag);
#endif

    while (!*flag) {
    }

    // 送受信開始
    rs_packet(&v1, &v2, pool, opt);

    shm_unlink("shm_buf");

    return 0;
}

