#!/bin/bash

mkdir results
touch results/base.txt

mkdir results/individual

mkdir results/individual/programmatic
mkdir results/individual/programmatic/structure
mkdir results/individual/programmatic/structure/virtqueue
mkdir results/individual/programmatic/structure/virtqueue/queue
mkdir results/individual/programmatic/structure/virtqueue/queue/type
touch results/individual/programmatic/structure/virtqueue/queue/type/idx_64B.txt

mkdir results/individual/programmatic/structure/virtqueue/queue/alignment
touch results/individual/programmatic/structure/virtqueue/queue/alignment/avail_idx.txt
touch results/individual/programmatic/structure/virtqueue/queue/alignment/used_idx.txt
touch results/individual/programmatic/structure/virtqueue/queue/alignment/desc.txt

mkdir results/individual/programmatic/structure/virtqueue/entry
mkdir results/individual/programmatic/structure/virtqueue/entry/size
touch results/individual/programmatic/structure/virtqueue/entry/size/8B.txt
touch results/individual/programmatic/structure/virtqueue/entry/size/64B.txt
mkdir results/individual/programmatic/structure/virtqueue/entry/alignment
touch results/individual/programmatic/structure/virtqueue/entry/alignment/entry_index.txt

mkdir results/individual/programmatic/structure/packet_buffer
mkdir results/individual/programmatic/structure/packet_buffer/buffer_size
touch results/individual/programmatic/structure/packet_buffer/buffer_size/320B.txt
mkdir results/individual/programmatic/structure/packet_buffer/header_size
touch results/individual/programmatic/structure/packet_buffer/header_size/0B.txt
touch results/individual/programmatic/structure/packet_buffer/header_size/0B.txt
mkdir results/individual/programmatic/structure/packet_buffer/offset_size
touch results/individual/programmatic/structure/packet_buffer/offset_size/320B.txt

mkdir results/individual/programmatic/access
mkdir results/individual/programmatic/access/read
mkdir results/individual/programmatic/access/read/rx
mkdir results/individual/programmatic/access/read/rx/header
touch results/individual/programmatic/access/read/rx/header/first_4B.txt
mkdir results/individual/programmatic/access/read/nf
mkdir results/individual/programmatic/access/read/nf/header
touch results/individual/programmatic/access/read/nf/header/first_4B.txt
mkdir results/individual/programmatic/access/read/nf/body
touch results/individual/programmatic/access/read/nf/body/first_4B.txt
mkdir results/individual/programmatic/access/read/tx
mkdir results/individual/programmatic/access/read/tx/header
touch results/individual/programmatic/access/read/tx/header/first_4B.txt

mkdir results/individual/programmatic/access/write
mkdir results/individual/programmatic/access/write/rx
mkdir results/individual/programmatic/access/write/rx/header
touch results/individual/programmatic/access/write/rx/header/first_4B.txt
mkdir results/individual/programmatic/access/write/nf
mkdir results/individual/programmatic/access/write/nf/header
touch results/individual/programmatic/access/write/nf/header/first_4B.txt
mkdir results/individual/programmatic/access/write/nf/body
touch results/individual/programmatic/access/write/nf/body/first_4B.txt
mkdir results/individual/programmatic/access/write/tx
mkdir results/individual/programmatic/access/write/tx/header
touch results/individual/programmatic/access/write/tx/header/first_4B.txt


mkdir results/individual/programmatic/api
mkdir results/individual/programmatic/api/memcpy
mkdir results/individual/programmatic/api/memcpy/vector
mkdir results/individual/programmatic/api/memcpy/vector/rx
touch results/individual/programmatic/api/memcpy/vector/rx/movl.txt
mkdir results/individual/programmatic/api/memcpy/vector/tx
touch results/individual/programmatic/api/memcpy/vector/tx/movl.txt
mkdir results/individual/programmatic/api/memcpy/non-temporal
mkdir results/individual/programmatic/api/memcpy/non-temporal/rx
touch results/individual/programmatic/api/memcpy/non-temporal/rx/vmovntdqa.txt
mkdir results/individual/programmatic/api/memcpy/non-temporal/tx
touch results/individual/programmatic/api/memcpy/non-temporal/tx/vmovntdqa.txt

mkdir results/individual/programmatic/api/cache_line
mkdir results/individual/programmatic/api/cache_line/prefetch
mkdir results/individual/programmatic/api/cache_line/prefetch/nf
touch results/individual/programmatic/api/cache_line/prefetch/nf/prefetch.txt
mkdir results/individual/programmatic/api/cache_line/prefetch/tx
touch results/individual/programmatic/api/cache_line/prefetch/tx/prefetch.txt

mkdir results/individual/programmatic/api/cache_line/demote
mkdir results/individual/programmatic/api/cache_line/demote/nf
touch results/individual/programmatic/api/cache_line/demote/nf/demote.txt
mkdir results/individual/programmatic/api/cache_line/demote/rx
touch results/individual/programmatic/api/cache_line/demote/rx/demote.txt

mkdir results/individual/programmatic/api/cache_line/flush
mkdir results/individual/programmatic/api/cache_line/flush/nf
touch results/individual/programmatic/api/cache_line/flush/nf/flush.txt
mkdir results/individual/programmatic/api/cache_line/flush/rx
touch results/individual/programmatic/api/cache_line/flush/rx/flush.txt

mkdir results/individual/programmatic/other
mkdir results/individual/programmatic/other/zero_copy
mkdir results/individual/programmatic/other/zero_copy/rx
touch results/individual/programmatic/other/zero_copy/rx/zerocopy.txt
mkdir results/individual/programmatic/other/zero_copy/tx
touch results/individual/programmatic/other/zero_copy/tx/zerocopy.txt


mkdir results/individual/configuration
mkdir results/individual/configuration/runtime
mkdir results/individual/configuration/runtime/virtqueue
mkdir results/individual/configuration/runtime/virtqueue/num_entries
touch results/individual/configuration/runtime/virtqueue/num_entries/64.txt
touch results/individual/configuration/runtime/virtqueue/num_entries/65536.txt

mkdir results/individual/configuration/runtime/packet_pool
mkdir results/individual/configuration/runtime/packet_pool/buffer
mkdir results/individual/configuration/runtime/packet_pool/buffer/num_entries
touch results/individual/configuration/runtime/packet_pool/buffer/num_entries/128.txt
touch results/individual/configuration/runtime/packet_pool/buffer/num_entries/1048576.txt

mkdir results/individual/configuration/runtime/packet_pool/cache
mkdir results/individual/configuration/runtime/packet_pool/cache/num_entries
touch results/individual/configuration/runtime/packet_pool/cache/num_entries/0.txt
touch results/individual/configuration/runtime/packet_pool/cache/num_entries/4096.txt


mkdir results/individual/configuration/runtime/other
mkdir results/individual/configuration/runtime/other/batching
mkdir results/individual/configuration/runtime/other/batching/rx
touch results/individual/configuration/runtime/other/batching/rx/1.txt
touch results/individual/configuration/runtime/other/batching/rx/256.txt
mkdir results/individual/configuration/runtime/other/batching/nf
touch results/individual/configuration/runtime/other/batching/nf/1.txt
touch results/individual/configuration/runtime/other/batching/nf/256.txt
mkdir results/individual/configuration/runtime/other/batching/tx
touch results/individual/configuration/runtime/other/batching/tx/1.txt
touch results/individual/configuration/runtime/other/batching/tx/256.txt

mkdir results/individual/configuration/runtime/other/buffer_allocation
mkdir results/individual/configuration/runtime/other/buffer_allocation/rx
touch results/individual/configuration/runtime/other/buffer_allocation/rx/random.txt
mkdir results/individual/configuration/runtime/other/buffer_allocation/nf
touch results/individual/configuration/runtime/other/buffer_allocation/nf/random.txt
mkdir results/individual/configuration/runtime/other/buffer_allocation/tx
touch results/individual/configuration/runtime/other/buffer_allocation/tx/random.txt

mkdir results/individual/configuration/bios
mkdir results/individual/configuration/bios/cache
mkdir results/individual/configuration/bios/cache/hw_prefetch
touch results/individual/configuration/bios/cache/hw_prefetch/off.txt
mkdir results/individual/configuration/bios/cache/adjacent_cache
touch results/individual/configuration/bios/cache/adjacent_cache/off.txt

