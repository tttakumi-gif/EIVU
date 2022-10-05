#include "virtqueue.hpp"
#include "shm.hpp"

namespace {
    void attach_buffer_to_vq(vq *virtqueue, buffer_pool *pool) {
        assert(VQ_ENYRY_NUM <= POOL_ENTRY_NUM);
        for (int i = 0; i < VQ_ENYRY_NUM; i++) {
#ifdef RANDOM_NF
            int entry_index = i / 32 * 32 + ids[i % 32];
#else
            auto entry_index = static_cast<int64_t>(get_buffer_index(pool, get_buffer(pool)));
#endif
            virtqueue[i].descs->entry_index = entry_index;
        }
    }

    void rs_packet(vq *vq_rx_to_guest, vq *ring_guest_to_tx, buffer_pool *pool, info_opt opt) {
#ifdef CPU_BIND
        bind_core(1);
#endif
        newvq v1{}, v2{};
        init_ring(&v1, vq_rx_to_guest->descs);
        init_ring(&v2, ring_guest_to_tx->descs);

        if (opt.process == MOVE) {
            int num_fin = static_cast<int>(opt.size_batch);

            for (int i = NUM_PACKET; 0 < i; i -= num_fin) {
                if (i < num_fin) {
                    num_fin = i;
                }

                guest_recv_process(&v1, &v2, pool, num_fin);
            }
        } else {
            exit(1);
        }
    }
}

int main(int argc, char *argv[]) {
    // 初期設定
    int bfd = open_shmfile("shm_buf", SIZE_SHM, true);
//    buf *pool_guest_addr = (buf *) mmap(nullptr, SIZE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, bfd, 0);
//    memset(pool_guest_addr, 0, sizeof(buf) * POOL_ENTRY_NUM);
//    for (int i = 0; i < POOL_ENTRY_NUM; i++) {
//        set_len(&pool_guest_addr[i], -1);
//    }

    auto *pool = (buffer_pool *) mmap(nullptr, SIZE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, bfd, 0);
//    auto *pool = (buffer_pool *) (pool_guest_addr + POOL_ENTRY_NUM);
    init(pool);

    auto *vq_rx_to_guest = (vq *) (pool + 1);
    init_ring(vq_rx_to_guest);
    attach_buffer_to_vq(vq_rx_to_guest, pool);

    vq *vq_guest_to_tx = (vq *) (vq_rx_to_guest + 1);
    init_ring(vq_guest_to_tx);

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
    rs_packet(vq_rx_to_guest, vq_guest_to_tx, pool, opt);

    shm_unlink("shm_buf");

    return 0;
}

