#!/bin/bash
mkdir individual/programmatic/structure/virtqueue/queue/type/idx_64B/run.sh individual/programmatic/structure/virtqueue/queue/type/idx_64B/run.sh

mkdir individual/programmatic/structure/virtqueue/queue/alignment/avail_idx/run.sh individual/programmatic/structure/virtqueue/queue/alignment/avail_idx/run.sh
mkdir individual/programmatic/structure/virtqueue/queue/alignment/used_idx/run.sh individual/programmatic/structure/virtqueue/queue/alignment/used_idx/run.sh
mkdir individual/programmatic/structure/virtqueue/queue/alignment/desc/run.sh individual/programmatic/structure/virtqueue/queue/alignment/run.sh

mkdir individual/programmatic/structure/virtqueue/entry/size/8B/run.sh individual/programmatic/structure/virtqueue/entry/size/8B/run.sh
mkdir individual/programmatic/structure/virtqueue/entry/size/64B/run.sh individual/programmatic/structure/virtqueue/entry/size/64B/run.sh
mkdir individual/programmatic/structure/virtqueue/entry/alignment/entry_index/run.sh individual/programmatic/structure/virtqueue/entry/alignment/entry_index/run.sh

mkdir individual/programmatic/structure/packet_buffer/buffer_size/320B/run.sh individual/programmatic/structure/packet_buffer/buffer_size/320B/run.sh
mkdir individual/programmatic/structure/packet_buffer/header_size/0B/run.sh individual/programmatic/structure/packet_buffer/header_size/0B/run.sh
mkdir individual/programmatic/structure/packet_buffer/header_size/1024B/run.sh individual/programmatic/structure/packet_buffer/header_size/1024B/run.sh
mkdir individual/programmatic/structure/packet_buffer/offset_size/0B/run.sh individual/programmatic/structure/packet_buffer/offset_size/0B/run.sh

mkdir individual/programmatic/access
mkdir individual/programmatic/access/read
mkdir individual/programmatic/access/read/rx
mkdir individual/programmatic/access/read/rx/header
mkdir individual/programmatic/access/read/rx/header/first_4B
mkdir individual/programmatic/access/read/nf
mkdir individual/programmatic/access/read/nf/header
mkdir individual/programmatic/access/read/nf/header/first_4B
mkdir individual/programmatic/access/read/nf/body
mkdir individual/programmatic/access/read/nf/body/first_4B
mkdir individual/programmatic/access/read/tx
mkdir individual/programmatic/access/read/tx/header
mkdir individual/programmatic/access/read/tx/header/first_4B

mkdir individual/programmatic/access/write
mkdir individual/programmatic/access/write/rx
mkdir individual/programmatic/access/write/rx/header
mkdir individual/programmatic/access/write/rx/header/first_4B
mkdir individual/programmatic/access/write/nf
mkdir individual/programmatic/access/write/nf/header
mkdir individual/programmatic/access/write/nf/header/first_4B
mkdir individual/programmatic/access/write/nf/body
mkdir individual/programmatic/access/write/nf/body/first_4B
mkdir individual/programmatic/access/write/tx
mkdir individual/programmatic/access/write/tx/header
mkdir individual/programmatic/access/write/tx/header/first_4B


mkdir individual/programmatic/api
mkdir individual/programmatic/api/memcpy
mkdir individual/programmatic/api/memcpy/vector
mkdir individual/programmatic/api/memcpy/vector/rx
mkdir individual/programmatic/api/memcpy/vector/rx/movl
mkdir individual/programmatic/api/memcpy/vector/tx
mkdir individual/programmatic/api/memcpy/vector/tx/movl
mkdir individual/programmatic/api/memcpy/non-temporal
mkdir individual/programmatic/api/memcpy/non-temporal/rx
mkdir individual/programmatic/api/memcpy/non-temporal/rx/vmovntdqa
mkdir individual/programmatic/api/memcpy/non-temporal/tx
mkdir individual/programmatic/api/memcpy/non-temporal/tx/vmovntdqa

mkdir individual/programmatic/api/cache_line
mkdir individual/programmatic/api/cache_line/prefetch
mkdir individual/programmatic/api/cache_line/prefetch/nf
mkdir individual/programmatic/api/cache_line/prefetch/nf/prefetch
mkdir individual/programmatic/api/cache_line/prefetch/tx
mkdir individual/programmatic/api/cache_line/prefetch/tx/prefetch

mkdir individual/programmatic/api/cache_line/demote
mkdir individual/programmatic/api/cache_line/demote/nf
mkdir individual/programmatic/api/cache_line/demote/nf/demote
mkdir individual/programmatic/api/cache_line/demote/rx
mkdir individual/programmatic/api/cache_line/demote/rx/demote

mkdir individual/programmatic/api/cache_line/flush
mkdir individual/programmatic/api/cache_line/flush/nf
mkdir individual/programmatic/api/cache_line/flush/nf/flush
mkdir individual/programmatic/api/cache_line/flush/rx
mkdir individual/programmatic/api/cache_line/flush/rx/flush

mkdir individual/programmatic/other
mkdir individual/programmatic/other/zero_copy
mkdir individual/programmatic/other/zero_copy/rx
mkdir individual/programmatic/other/zero_copy/rx/zerocopy
mkdir individual/programmatic/other/zero_copy/tx
mkdir individual/programmatic/other/zero_copy/tx/zerocopy


mkdir individual/configuration
mkdir individual/configuration/runtime
mkdir individual/configuration/runtime/virtqueue
mkdir individual/configuration/runtime/virtqueue/num_entries
mkdir individual/configuration/runtime/virtqueue/num_entries/64
mkdir individual/configuration/runtime/virtqueue/num_entries/65536

mkdir individual/configuration/runtime/packet_pool
mkdir individual/configuration/runtime/packet_pool/buffer
mkdir individual/configuration/runtime/packet_pool/buffer/num_entries
mkdir individual/configuration/runtime/packet_pool/buffer/num_entries/128
mkdir individual/configuration/runtime/packet_pool/buffer/num_entries/1048576

mkdir individual/configuration/runtime/packet_pool/cache
mkdir individual/configuration/runtime/packet_pool/cache/num_entries
mkdir individual/configuration/runtime/packet_pool/cache/num_entries/0
mkdir individual/configuration/runtime/packet_pool/cache/num_entries/4096


mkdir individual/configuration/runtime/other
mkdir individual/configuration/runtime/other/batching
mkdir individual/configuration/runtime/other/batching/rx
mkdir individual/configuration/runtime/other/batching/rx/1
mkdir individual/configuration/runtime/other/batching/rx/256
mkdir individual/configuration/runtime/other/batching/nf
mkdir individual/configuration/runtime/other/batching/nf/1
mkdir individual/configuration/runtime/other/batching/nf/256
mkdir individual/configuration/runtime/other/batching/tx
mkdir individual/configuration/runtime/other/batching/tx/1
mkdir individual/configuration/runtime/other/batching/tx/256

mkdir individual/configuration/runtime/other/buffer_allocation
mkdir individual/configuration/runtime/other/buffer_allocation/rx
mkdir individual/configuration/runtime/other/buffer_allocation/rx/random
mkdir individual/configuration/runtime/other/buffer_allocation/nf
mkdir individual/configuration/runtime/other/buffer_allocation/nf/random
mkdir individual/configuration/runtime/other/buffer_allocation/tx
mkdir individual/configuration/runtime/other/buffer_allocation/tx/random

mkdir individual/configuration/bios
mkdir individual/configuration/bios/cache
mkdir individual/configuration/bios/cache/hw_prefetch
mkdir individual/configuration/bios/cache/hw_prefetch/off
mkdir individual/configuration/bios/cache/adjacent_cache
mkdir individual/configuration/bios/cache/adjacent_cache/off

