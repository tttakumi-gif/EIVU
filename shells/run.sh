#!/bin/bash

opt_perf="-e cache-references,cache-misses,mem_load_uops_retired.l1_hit,mem_load_uops_retired.l1_miss,mem_load_uops_retired.hit_lfb,mem_load_uops_retired.l3_hit,mem_load_uops_retired.l3_miss,LLC-load,LLC-load-misses,LLC-store,LLC-store-misses"

#opt_perf="-e cache-misses,L1-dcache-load-misses,L1-dcache-loads,L1-dcache-stores,LLC-loads,LLC-stores,LLC-load-misses,LLC-store-misses,cpu/event=0xA3,umask=0x06,cmask=0x06,name=STALLS_L3_MISS/"
#opt_perf="-e cpu/event=0xA3,umask=0x06,cmask=0x06,name=STALLS_L3_MISS/,cycle_activity.stalls_l3_miss,cpu/event=0x60,umask=0x10,name=OUT_L3miss_Dem_RD/"

#i="-I 1000"
i=""
loop=1
#perf="perf stat $opt_perf -r $loop $i"
perf=""

batch_rx=32
batch_nf=32
batch_tx=32

sudo ${perf} ./nf.out --stream=off --process=move --batch=$batch_nf &
sleep 0.5
sudo ./rx.out --batch=$batch_rx --process=copy --stream=$1 &
sleep 0.5 
sudo ./tx.out --stream=$2 --process=copy --batch=$batch_tx

