#include "virtqueue.hpp"
#include "shm.hpp"

namespace {
    void init_descs(desc *descs) {
        memset(descs, 0, sizeof(desc) * VQ_ENTRY_NUM);
        for (int i = 0; i < VQ_ENTRY_NUM; i++) {
            descs[i].entry_index = static_cast<int16_t>(i);
            descs[i].flags = static_cast<int16_t>(descs[i].flags | USED_FLAG);
        }
    }

    void attach_buffer_to_vq(vq *virtqueue, buffer_pool *pool) {
        assert(VQ_ENTRY_NUM <= POOL_ENTRY_NUM);
        for (int i = 0; i < VQ_ENTRY_NUM; i++) {
#ifdef RANDOM_NF
            int entry_index = i / 32 * 32 + ids[i % 32];
#else
            auto entry_index = static_cast<int64_t>(get_buffer_index(pool, get_buffer(pool)));
#endif
            virtqueue->descs->entry_index = entry_index;
        }
    }

    void rs_packet(vq *vq_rx, vq *vq_tx, buffer_pool *pool, info_opt opt) {
#ifdef CPU_BIND
        bind_core(1);
#endif
        attach_buffer_to_vq(vq_rx, pool);

        if (opt.process == MOVE) {
            int num_fin = static_cast<int>(opt.size_batch);

            for (int i = NUM_PACKET; 0 < i; i -= num_fin) {
                if (i < num_fin) {
                    num_fin = i;
                }

                guest_recv_process(vq_rx, vq_tx, pool, num_fin);
            }
        } else {
            exit(1);
        }
    }
}

int main(int argc, char *argv[]) {
    // 初期設定
    int bfd = open_shmfile(SHM_FILE, SIZE_SHM, true);

    auto *pool = (buffer_pool *) mmap(nullptr, SIZE_SHM, PROT_READ | PROT_WRITE, SHM_FLAG, bfd, 0);
    init(pool);

    auto *descs_rx = (desc *) (pool + 1);
    init_descs(descs_rx);
    vq vq_rx{};
    init_ring(&vq_rx, descs_rx);

    auto *descs_tx = (desc *) (descs_rx + VQ_ENTRY_NUM);
    init_descs(descs_tx);
    vq vq_tx{};
    init_ring(&vq_tx, descs_tx);

    info_opt opt = get_opt(argc, argv);

    volatile bool *flag = (volatile bool *) (descs_tx + VQ_ENTRY_NUM);
    *flag = false;

    while (!*flag) {
    }

    // 送受信開始
    rs_packet(&vq_rx, &vq_tx, pool, opt);

#ifdef HUGE_PAGE
    close_shmfile(bfd);
#else
    shm_unlink(SHM_FILE);
#endif

    return 0;
}

