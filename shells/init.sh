#!/bin/bash

mkdir results

mkdir results/individual

mkdir results/individual/programmatic
mkdir results/individual/programmatic/structure
mkdir results/individual/programmatic/structure/virtqueue
mkdir results/individual/programmatic/structure/virtqueue/queue
mkdir results/individual/programmatic/structure/virtqueue/queue/type
mkdir results/individual/programmatic/structure/virtqueue/queue/type/idx_64B

mkdir results/individual/programmatic/structure/virtqueue/queue/alignment
mkdir results/individual/programmatic/structure/virtqueue/queue/alignment/avail_idx
mkdir results/individual/programmatic/structure/virtqueue/queue/alignment/used_idx
mkdir results/individual/programmatic/structure/virtqueue/queue/alignment/desc

mkdir results/individual/programmatic/structure/virtqueue/entry
mkdir results/individual/programmatic/structure/virtqueue/entry/size
mkdir results/individual/programmatic/structure/virtqueue/entry/size/8B
mkdir results/individual/programmatic/structure/virtqueue/entry/size/64B
mkdir results/individual/programmatic/structure/virtqueue/entry/alignment
mkdir results/individual/programmatic/structure/virtqueue/entry/alignment/entry_index

mkdir results/individual/programmatic/structure/packet_buffer
mkdir results/individual/programmatic/structure/packet_buffer/buffer_size
mkdir results/individual/programmatic/structure/packet_buffer/buffer_size/320B
mkdir results/individual/programmatic/structure/packet_buffer/header_size
mkdir results/individual/programmatic/structure/packet_buffer/header_size/0B
mkdir results/individual/programmatic/structure/packet_buffer/header_size/1024B
mkdir results/individual/programmatic/structure/packet_buffer/offset_size
mkdir results/individual/programmatic/structure/packet_buffer/offset_size/0B

mkdir results/individual/programmatic/access
mkdir results/individual/programmatic/access/read
mkdir results/individual/programmatic/access/read/rx
mkdir results/individual/programmatic/access/read/rx/header
mkdir results/individual/programmatic/access/read/rx/header/first_4B
mkdir results/individual/programmatic/access/read/nf
mkdir results/individual/programmatic/access/read/nf/header
mkdir results/individual/programmatic/access/read/nf/header/first_4B
mkdir results/individual/programmatic/access/read/nf/body
mkdir results/individual/programmatic/access/read/nf/body/first_4B
mkdir results/individual/programmatic/access/read/tx
mkdir results/individual/programmatic/access/read/tx/header
mkdir results/individual/programmatic/access/read/tx/header/first_4B

mkdir results/individual/programmatic/access/write
mkdir results/individual/programmatic/access/write/rx
mkdir results/individual/programmatic/access/write/rx/header
mkdir results/individual/programmatic/access/write/rx/header/first_4B
mkdir results/individual/programmatic/access/write/nf
mkdir results/individual/programmatic/access/write/nf/header
mkdir results/individual/programmatic/access/write/nf/header/first_4B
mkdir results/individual/programmatic/access/write/nf/body
mkdir results/individual/programmatic/access/write/nf/body/first_4B
mkdir results/individual/programmatic/access/write/tx
mkdir results/individual/programmatic/access/write/tx/header
mkdir results/individual/programmatic/access/write/tx/header/first_4B


mkdir results/individual/programmatic/api
mkdir results/individual/programmatic/api/memcpy
mkdir results/individual/programmatic/api/memcpy/vector
mkdir results/individual/programmatic/api/memcpy/vector/rx
mkdir results/individual/programmatic/api/memcpy/vector/rx/movl
mkdir results/individual/programmatic/api/memcpy/vector/tx
mkdir results/individual/programmatic/api/memcpy/vector/tx/movl
mkdir results/individual/programmatic/api/memcpy/non-temporal
mkdir results/individual/programmatic/api/memcpy/non-temporal/rx
mkdir results/individual/programmatic/api/memcpy/non-temporal/rx/vmovntdqa
mkdir results/individual/programmatic/api/memcpy/non-temporal/tx
mkdir results/individual/programmatic/api/memcpy/non-temporal/tx/vmovntdqa

mkdir results/individual/programmatic/api/cache_line
mkdir results/individual/programmatic/api/cache_line/prefetch
mkdir results/individual/programmatic/api/cache_line/prefetch/nf
mkdir results/individual/programmatic/api/cache_line/prefetch/nf/prefetch
mkdir results/individual/programmatic/api/cache_line/prefetch/tx
mkdir results/individual/programmatic/api/cache_line/prefetch/tx/prefetch

mkdir results/individual/programmatic/api/cache_line/demote
mkdir results/individual/programmatic/api/cache_line/demote/nf
mkdir results/individual/programmatic/api/cache_line/demote/nf/demote
mkdir results/individual/programmatic/api/cache_line/demote/rx
mkdir results/individual/programmatic/api/cache_line/demote/rx/demote

mkdir results/individual/programmatic/api/cache_line/flush
mkdir results/individual/programmatic/api/cache_line/flush/nf
mkdir results/individual/programmatic/api/cache_line/flush/nf/flush
mkdir results/individual/programmatic/api/cache_line/flush/rx
mkdir results/individual/programmatic/api/cache_line/flush/rx/flush

mkdir results/individual/programmatic/other
mkdir results/individual/programmatic/other/zero_copy
mkdir results/individual/programmatic/other/zero_copy/rx
mkdir results/individual/programmatic/other/zero_copy/rx/zerocopy
mkdir results/individual/programmatic/other/zero_copy/tx
mkdir results/individual/programmatic/other/zero_copy/tx/zerocopy


mkdir results/individual/configuration
mkdir results/individual/configuration/runtime
mkdir results/individual/configuration/runtime/virtqueue
mkdir results/individual/configuration/runtime/virtqueue/num_entries
mkdir results/individual/configuration/runtime/virtqueue/num_entries/64
mkdir results/individual/configuration/runtime/virtqueue/num_entries/65536

mkdir results/individual/configuration/runtime/packet_pool
mkdir results/individual/configuration/runtime/packet_pool/buffer
mkdir results/individual/configuration/runtime/packet_pool/buffer/num_entries
mkdir results/individual/configuration/runtime/packet_pool/buffer/num_entries/128
mkdir results/individual/configuration/runtime/packet_pool/buffer/num_entries/1048576

mkdir results/individual/configuration/runtime/packet_pool/cache
mkdir results/individual/configuration/runtime/packet_pool/cache/num_entries
mkdir results/individual/configuration/runtime/packet_pool/cache/num_entries/0
mkdir results/individual/configuration/runtime/packet_pool/cache/num_entries/4096


mkdir results/individual/configuration/runtime/other
mkdir results/individual/configuration/runtime/other/batching
mkdir results/individual/configuration/runtime/other/batching/rx
mkdir results/individual/configuration/runtime/other/batching/rx/1
mkdir results/individual/configuration/runtime/other/batching/rx/256
mkdir results/individual/configuration/runtime/other/batching/nf
mkdir results/individual/configuration/runtime/other/batching/nf/1
mkdir results/individual/configuration/runtime/other/batching/nf/256
mkdir results/individual/configuration/runtime/other/batching/tx
mkdir results/individual/configuration/runtime/other/batching/tx/1
mkdir results/individual/configuration/runtime/other/batching/tx/256

mkdir results/individual/configuration/runtime/other/buffer_allocation
mkdir results/individual/configuration/runtime/other/buffer_allocation/rx
mkdir results/individual/configuration/runtime/other/buffer_allocation/rx/random
mkdir results/individual/configuration/runtime/other/buffer_allocation/nf
mkdir results/individual/configuration/runtime/other/buffer_allocation/nf/random
mkdir results/individual/configuration/runtime/other/buffer_allocation/tx
mkdir results/individual/configuration/runtime/other/buffer_allocation/tx/random

mkdir results/individual/configuration/bios
mkdir results/individual/configuration/bios/cache
mkdir results/individual/configuration/bios/cache/hw_prefetch
mkdir results/individual/configuration/bios/cache/hw_prefetch/off
mkdir results/individual/configuration/bios/cache/adjacent_cache
mkdir results/individual/configuration/bios/cache/adjacent_cache/off

