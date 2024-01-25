#include "vioqueue.hpp"
#include "shm.hpp"

namespace {
    void recv_packet(vq *vq_tx, guest_buffer_pool *pool_guest, info_opt opt) {
#ifdef CPU_BIND
        bind_core(2);
#endif

        int32_t num_fin = BATCH_SIZE_TX;
        bool is_stream = opt.stream == ON;

        auto *pool = new host_buffer_pool();
        init(pool);
        buf **recv_addrs = new buf *[num_fin];

        int last_pring_idx = 0;
        int last_pring_inflight_idx = 0;

        uint64_t first; //パケット生成時のクロック数
        uint64_t end; //パケットをPRINGに転送したときのクロック数
        uint64_t *result = new uint64_t[NUM_PACKET]; //上2つのクロックの差分をパケットの数だけ保存しておく配列
        int32_t result_idx = 0; //上の配列のインデックス
        uint64_t currentfreq; //現在のCPUクロック周波数（コア2）
        double latency; //クロック数の差分と周波数から計算したレイテンシー

        constexpr uint16_t RING_SIZE_MASK = PRING_SIZE_TX - 1;

        buf **dummy_physical_ring = new buf *[PRING_SIZE_TX];
        memset(dummy_physical_ring, 0, sizeof(buf *) * PRING_SIZE_TX);

        try {
            for (int i = NUM_PACKET; 0 < i; i -= num_fin) {
                if (i < num_fin) {
                    num_fin = i;
                }

                for (int j = 0; j < num_fin; j++) {
#ifdef RANDOM_TX
                    recv_addrs[j] = &pool->buffers[pool->last_pool_idx / 32 * 32 + (int) ids[j]];
                    pool->last_pool_idx = (pool->last_pool_idx + 1) % HOST_POOL_ENTRY_NUM;
#else
                    recv_addrs[j] = get_buffer(pool);
#endif
                }

                // Receive packets from the NF process
                send_guest_to_tx(vq_tx, recv_addrs, pool_guest, num_fin, is_stream);

                /* Flushing the all inflight buffers in the pring */
                {
                    int n_free = last_pring_idx - last_pring_inflight_idx;
                    if (n_free == 0) {

                    } else if (n_free < 0) {
                        n_free += PRING_SIZE_TX;
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
                    //PRINGにパケットを転送するタイミングでクロック数取得、差分を配列に記録
                    first = ((packet *) (recv_addrs[j]->addr))->rx_time;
                    end = __rdtsc();
                    result[result_idx] = (end - first);
                    result_idx++;
                }

#ifdef PRINT
                for(int j = 0; j < num_fin; j++) {
                    if (((i - j) & 8388607) == 0) {
                        print((packet *) &recv_addrs[j]->addr);
                    }
                }
#endif
            }
        } catch (std::exception &e) {
            std::cerr << "[tx] ERROR: " << e.what() << " terminating..." << std::endl;
        }

        // ＊前回の計測で各コアの周波数を記録したところ、各コアの周波数はほとんど変化していなかった
        // ＊したがって周波数の取得は一度のみで良いと判断した（各パケットの転送のタイミングで毎回ファイルを読みに行くとその部分の処理が重くなりすぎてしまうと考えたから）
        // ＊また、前回の計測でコア間の周波数にも差がほとんど見られなかった（最大周波数を制限していないコア同士の話,最大周波数を制限したコア同士も差がほとんどない）
        // ＊コア間の周波数にもほとんど差がないことから、コア0コア1コア2の周波数は同一という前提で計算
        // ＊もちろんrdtsc命令で取得できるサイクル数はコア間で異なるので、今回の計測でレイテンシーは正確には取れないが、ジッタの悪化を観測する分には問題ないという前提
        // 現在のcpu周波数取得（コア2）
        std::ifstream curFreqFile("/sys/devices/system/cpu/cpu2/cpufreq/scaling_cur_freq");
        if (curFreqFile.is_open()) {
            std::string curFreq;
            curFreqFile >> curFreq;
            currentfreq = std::stoi(curFreq);
        } else {
        }

        //記録用ファイルへの書き出し
        const char* filename = "output.log";
        
        // ファイルを開く
        std::ofstream outputFile(filename);

        // ファイルが正しく開けたか確認
        if (!outputFile.is_open()) {
            std::cerr << "Failed to open the file." << std::endl;
        }

        // 配列の内容をファイルに書き込む（レイテンシーの計算）
        for (int i = 0; i < NUM_PACKET; ++i) {
            latency = static_cast<double>(result[i]) / static_cast<double>(currentfreq) * (10^6);
            outputFile << result[i] << std::endl;
        }

        // ファイルを閉じる
        outputFile.close();

        //　記録用配列を削除
        delete result;

        delete pool;
        delete[](recv_addrs);
        delete[](dummy_physical_ring);
    }
}

int main(int argc, char **argv) {
    static_assert(BATCH_SIZE_TX <= VQ_ENTRY_NUM);

    // Initialize
    int bfd = open_shmfile(SHM_FILE, SIZE_SHM, false);
    auto *pool = (guest_buffer_pool *) mmap(nullptr, SIZE_SHM, PROT_READ | PROT_WRITE, SHM_FLAG, bfd, 0);
    auto *descs_rx = (desc *) (pool + 1);
    auto *descs_tx = (desc *) (descs_rx + VQ_ENTRY_NUM);

    vq vq_tx{};
    init_ring(&vq_tx, descs_tx);

    info_opt opt = get_opt(argc, argv);

    bool *flag = (bool *) (descs_tx + VQ_ENTRY_NUM);

    *flag = true;

    // Start measuring throughput
    std::chrono::system_clock::time_point start, end;
    start = std::chrono::system_clock::now();

    recv_packet(&vq_tx, pool, opt);

    // Stop the measuring
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
