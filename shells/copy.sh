#!/bin/bash

cp ./run.sh ./base/

cp ./run.sh ./individual/programmatic/structure/virtqueue/queue/type/idx_64B/

cp ./run.sh ./individual/programmatic/structure/virtqueue/queue/alignment/avail_idx/
cp ./run.sh ./individual/programmatic/structure/virtqueue/queue/alignment/used_idx/
cp ./run.sh ./individual/programmatic/structure/virtqueue/queue/alignment/desc/

cp ./run.sh ./individual/programmatic/structure/virtqueue/entry/size/8B/
cp ./run.sh ./individual/programmatic/structure/virtqueue/entry/size/64B/
cp ./run.sh ./individual/programmatic/structure/virtqueue/entry/alignment/entry_index/

cp ./run.sh ./individual/programmatic/structure/packet_buffer/buffer_size/320B/
cp ./run.sh ./individual/programmatic/structure/packet_buffer/header_size/0B/
cp ./run.sh ./individual/programmatic/structure/packet_buffer/header_size/1024B/
cp ./run.sh ./individual/programmatic/structure/packet_buffer/offset_size/0B/

cp ./run.sh ./individual/programmatic/access/read/rx/header/first_4B/
cp ./run.sh ./individual/programmatic/access/read/nf/header/first_4B/
cp ./run.sh ./individual/programmatic/access/read/nf/body/first_4B/
cp ./run.sh ./individual/programmatic/access/read/tx/header/first_4B/

cp ./run.sh ./individual/programmatic/access/write/rx/header/first_4B/
cp ./run.sh ./individual/programmatic/access/write/nf/header/first_4B/
cp ./run.sh ./individual/programmatic/access/write/nf/body/first_4B/
cp ./run.sh ./individual/programmatic/access/write/tx/header/first_4B/


cp ./run.sh ./individual/programmatic/api/memcpy/vector/rx/vmovdqa/
cp ./run.sh ./individual/programmatic/api/memcpy/vector/tx/vmovdqa/
cp ./run.sh ./individual/programmatic/api/memcpy/non-temporal/rx/vmovntdqa/
cp ./run.sh ./individual/programmatic/api/memcpy/non-temporal/tx/vmovntdqa/

cp ./run.sh ./individual/programmatic/api/cache_line/prefetch/rx/prefetch/
cp ./run.sh ./individual/programmatic/api/cache_line/prefetch/tx/prefetch/

cp ./run.sh ./individual/programmatic/api/cache_line/demote/tx/demote/
cp ./run.sh ./individual/programmatic/api/cache_line/demote/rx/demote/

cp ./run.sh ./individual/programmatic/api/cache_line/flush/tx/flush/
cp ./run.sh ./individual/programmatic/api/cache_line/flush/rx/flush/

cp ./run.sh ./individual/programmatic/other/zero_copy/rx/zerocopy/
cp ./run.sh ./individual/programmatic/other/zero_copy/tx/zerocopy/


cp ./run.sh ./individual/configuration/runtime/virtqueue/num_entries/64/
cp ./run.sh ./individual/configuration/runtime/virtqueue/num_entries/65536/

cp ./run.sh ./individual/configuration/runtime/packet_pool/buffer/num_entries/128/
cp ./run.sh ./individual/configuration/runtime/packet_pool/buffer/num_entries/1048576/

cp ./run.sh ./individual/configuration/runtime/packet_pool/cache/num_entries/0/
cp ./run.sh ./individual/configuration/runtime/packet_pool/cache/num_entries/4096/


cp ./run.sh ./individual/configuration/runtime/other/batching/rx/16/
cp ./run.sh ./individual/configuration/runtime/other/batching/rx/256/
cp ./run.sh ./individual/configuration/runtime/other/batching/nf/16/
cp ./run.sh ./individual/configuration/runtime/other/batching/nf/256/
cp ./run.sh ./individual/configuration/runtime/other/batching/tx/16/
cp ./run.sh ./individual/configuration/runtime/other/batching/tx/256/

cp ./run.sh ./individual/configuration/runtime/other/buffer_allocation/rx/random/
cp ./run.sh ./individual/configuration/runtime/other/buffer_allocation/nf/random/
cp ./run.sh ./individual/configuration/runtime/other/buffer_allocation/tx/random/

cp ./run.sh ./individual/configuration/bios/cache/hw_prefetch/off/
cp ./run.sh ./individual/configuration/bios/cache/adjacent_cache/off/
