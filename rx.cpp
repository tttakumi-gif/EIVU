#include "buffer.hpp"
#include "shm.hpp"

namespace {
    void send_packet(vq *vq_rx_to_guest, buf *pool_guest_addr, info_opt opt) {
        bind_core(0);

        int32_t i = NUM_PACKET;
        int32_t num_fin = opt.size_batch;
        bool is_stream = (opt.stream == ON) ? true : false;

        int local_pool_index = 0;
        buf *pool_rx_addr = new(std::align_val_t{64}) buf[POOL_ENTRY_NUM];
        buf **send_addrs = new buf *[opt.size_batch];
        void **pool_copy_dest = new void *[opt.size_batch];

        while (0 < i) {
            // 受信パケット数の決定
            if (unlikely(i < num_fin)) {
                num_fin = i;
            }

            for (int j = 0; j < num_fin; j++, i--) {
                send_addrs[j] = &pool_rx_addr[local_pool_index + (int) ids[j]];
                ((packet *) (send_addrs[j]->addr))->packet_id = i;
                ((packet *) (send_addrs[j]->addr))->packet_len = SIZE_PACKET;

                int buffer_total_len = 128 + 2176;
                int pool_target_index = vq_rx_to_guest->last_pool_idx + ids[j];
                int mbuf_size = 128;
                int padding_size = 128;
                int offset = buffer_total_len * pool_target_index + mbuf_size + padding_size;

                pool_copy_dest[j] = (void*)((char*)pool_guest_addr + offset);
            }

            // update last used buffer pool index
            vq_rx_to_guest->last_pool_idx = (vq_rx_to_guest->last_pool_idx + num_fin) % POOL_ENTRY_NUM;

            send_rx_to_guest(vq_rx_to_guest, send_addrs, pool_copy_dest, num_fin, is_stream);

            local_pool_index += num_fin;
            if (POOL_ENTRY_NUM <= local_pool_index) {
                local_pool_index = 0;
            }
        }

        delete[](send_addrs);
        delete[](pool_rx_addr);
    }

}

int main(int argc, char **argv) {
    // 初期設定
    int bfd = open_shmfile("shm_buf", SIZE_SHM, false);
    buf *pool_guest_addr = (buf *) mmap(NULL, SIZE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, bfd, 0);
    vq *vq_rx_to_guest = (vq *) (pool_guest_addr + POOL_ENTRY_NUM);
    vq *vq_guest_to_tx = (vq *) (vq_rx_to_guest + 1);

    info_opt opt = get_opt(argc, argv);

    volatile bool *flag = (volatile bool *) (vq_guest_to_tx + 1);
    *flag = false;

    while (!*flag) {
    }

    // 送受信開始
    send_packet(vq_rx_to_guest, pool_guest_addr, opt);

#ifdef SKIP_CLT
    *flag = false;
#endif

    shm_unlink("shm_buf");

    return 0;
}

