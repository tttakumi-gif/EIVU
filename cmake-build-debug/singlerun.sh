#!/bin/bash

#opt_perf="-e l1d.replacement,l1d_pend_miss.fb_full,l1d_pend_miss.pending,l2_lines_in.all,L1-dcache-loads,L1-dcache-load-misses,L1-dcache-stores,L1-dcache-store-misses,LLC-loads,LLC-load-misses,LLC-stores,LLC-store-misses,l2_rqsts.all_rfo,l2_rqsts.rfo_hit,l2_rqsts.rfo_miss,offcore_response.demand_rfo.l3_hit.hit_other_core_no_fwd,offcore_requests_outstanding.cycles_with_demand_rfo"
opt_perf="-e cache-references,cache-misses,mem_load_uops_retired.l1_hit,mem_load_uops_retired.l1_miss,mem_load_uops_retired.hit_lfb,mem_load_uops_retired.l3_hit,mem_load_uops_retired.l3_miss,LLC-load,LLC-load-misses,LLC-store,LLC-store-misses"
#l2_rqsts.all_rfo,l2_rqsts.rfo_hit,l2_rqsts.rfo_miss,offcore_requests.demand_rfo,offcore_requests_outstanding.demand_rfo,mem_inst_retired.all_loads,mem_inst_retired.all_stores

#opt_perf="-e cache-misses,L1-dcache-load-misses,L1-dcache-loads,L1-dcache-stores,LLC-loads,LLC-stores,LLC-load-misses,LLC-store-misses,cpu/event=0xA3,umask=0x06,cmask=0x06,name=STALLS_L3_MISS/"
#opt_perf="-e cpu/event=0xA3,umask=0x06,cmask=0x06,name=STALLS_L3_MISS/,cycle_activity.stalls_l3_miss,cpu/event=0x60,umask=0x10,name=OUT_L3miss_Dem_RD/"

#i="-I 1000"
i=""
loop=1
batch=32
perf="perf stat $opt_perf -r $loop $i"
#perf="valgrind --tool=cachegrind"
#perf=""

#sudo perf stat $opt_perf -r $loop ~/new-b4-assignment/build/clt.out --stream=off --batch=$batch &
#sudo perf stat $opt_perf -r $loop ~/new-b4-assignment/build/clt.out --stream=on --batch=$batch &

#for (( i=1; i<=$loop; i++ ));do
#	sleep 0.1
#	sudo ~/new-b4-assignment/build/recv.out --batch=32 &
#	sleep 0.1
#	sudo ~/new-b4-assignment/srv.out --batch=32
#done

sudo ${perf} ./srv.out --stream=off --process=move --batch=32 &
#sudo ./srv.out --stream=off --process=move --batch=32 &
sleep 0.5
#sudo ${perf} ./clt.out --batch=$batch --process=copy --stream=$1 &
sudo ./clt.out --batch=$batch --process=copy --stream=$1 &
sleep 0.5
#sudo ${perf} ./recv.out --stream=$2 --process=copy --batch=32
sudo ./recv.out --stream=$2 --process=copy --batch=32

#sudo ~/new-b4-assignment/build/clt.out --batch=$batch --process=copy --stream=$@ &
#sleep 0.1
#sudo ~/new-b4-assignment/build/recv.out --batch=32 &
#sleep 0.1
#sudo ${perf} ~/new-b4-assignment/srv.out --batch=32

