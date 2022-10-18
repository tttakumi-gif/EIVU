#include "virtqueue.hpp"
#include "shm.hpp"

namespace {
    void recv_packet(vq *vq_tx, buffer_pool *pool_guest, info_opt opt) {
#ifdef CPU_BIND
        bind_core(2);
#endif

        int32_t num_fin = opt.size_batch;
        bool is_stream = opt.stream == ON;

        auto *pool = new buffer_pool();
        init(pool);
        buf **recv_addrs = new buf *[num_fin];

        int last_pring_idx = 0;
        int last_pring_inflight_idx = 0;

        constexpr int pring_size = 128;
	constexpr uint16_t RING_SIZE_MASK = pring_size - 1;

        buf **dummy_physical_ring = new buf *[pring_size];
	memset(dummy_physical_ring, 0, sizeof(buf*) * pring_size);

	try {
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
                    recv_addrs[j] = get_buffer(pool);
#endif
                }

                send_guest_to_tx(vq_tx, recv_addrs, pool_guest, num_fin, is_stream);

		/* Flushing the all inflight buffers in the pring */
		{
		    int n_free = last_pring_idx - last_pring_inflight_idx;
		    if (n_free == 0) {

		    } else if (n_free < 0) {
                        n_free += pring_size;
		    }

		    for (int j = 0; j < n_free; j++) {
			    assert(dummy_physical_ring[last_pring_inflight_idx]);
                        add_to_cache(pool, dummy_physical_ring[last_pring_inflight_idx]);
			dummy_physical_ring[last_pring_inflight_idx] = nullptr;

		        last_pring_inflight_idx++;
                        last_pring_inflight_idx &= RING_SIZE_MASK;
		    }
		}

		/* Transmit the packets to the pring */
                for (int j = 0; j < num_fin; j++) {
		    dummy_physical_ring[last_pring_idx] = recv_addrs[j];
		    last_pring_idx++;
		    last_pring_idx &= RING_SIZE_MASK;
                }

#ifdef PRINT
                for(int j = 0; j < num_fin; j++) {
                    if (((i - j) & 8388607) == 0) {
                        print((packet *) &recv_addrs[j]->addr);
                    }
                }
#endif
            }
	} catch (std::exception& e) {
            std::cerr << "[tx] ERROR: " << e.what() << " terminating..." << std::endl;
	}

        delete pool;
        delete[](recv_addrs);
        delete[](dummy_physical_ring);
    }
}

int main(int argc, char **argv) {

    // 初期設定
    int bfd = open_shmfile(SHM_FILE, SIZE_SHM, false);
    auto *pool = (buffer_pool *) mmap(nullptr, SIZE_SHM, PROT_READ | PROT_WRITE, SHM_FLAG, bfd, 0);
    auto *descs_rx = (desc *) (pool + 1);
    auto *descs_tx = (desc *) (descs_rx + VQ_ENTRY_NUM);

    vq vq_rx{};
    init_ring(&vq_rx, descs_tx);

    info_opt opt = get_opt(argc, argv);

    bool *flag = (bool *) (descs_tx + VQ_ENTRY_NUM);

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

#ifdef HUGE_PAGE
    close_shmfile(bfd);
#else
    shm_unlink(SHM_FILE);
#endif

    return 0;
}
