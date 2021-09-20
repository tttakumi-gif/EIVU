#!/bin/bash

#opt_perf="-e l1d.replacement,l1d_pend_miss.fb_full,l1d_pend_miss.pending,l2_lines_in.all,l2_lines_in.e,l2_lines_in.i,l2_lines_in.s,mem_load_uops_retired.l1_hit,mem_load_uops_retired.l2_hit,mem_load_uops_retired.l3_hit,mem_load_uops_retired.l1_miss,mem_load_uops_retired.l2_miss,mem_load_uops_retired.l3_miss,mem_load_uops_retired.hit_lfb,mem_uops_retired.all_loads,mem_uops_retired.all_stores,L1-dcache-loads,L1-dcache-load-misses,L1-dcache-stores,L1-dcache-store-misses,LLC-loads,LLC-load-misses,LLC-stores,LLC-store-misses,l2_rqsts.all_rfo,l2_rqsts.rfo_hit,l2_rqsts.rfo_miss,offcore_requests.demand_rfo,offcore_requests_outstanding.cycles_with_demand_rfo"
opt_perf="-e L1-dcache-load-misses,L1-dcache-loads,L1-dcache-stores,LLC-load-misses,LLC-loads,LLC-store-misses,LLC-stores"

i="-I 1000"
#i=""
loop=1
batch=32
perf="perf stat $opt_perf -r $loop $i"
#perf=""

#sudo perf stat $opt_perf -r $loop ~/new-b4-assignment/build/clt.out --stream=off --batch=$batch &
#sudo perf stat $opt_perf -r $loop ~/new-b4-assignment/build/clt.out --stream=on --batch=$batch &

#for (( i=1; i<=$loop; i++ ));do
#	sleep 0.1
#	sudo ~/new-b4-assignment/build/recv.out --batch=32 &
#	sleep 0.1
#	sudo ~/new-b4-assignment/srv.out --batch=32
#done

sudo ~/new-b4-assignment/build/srv.out --stream=off --process=move --batch=32 &
sleep 0.5
sudo ${perf} ~/new-b4-assignment/build/clt.out --batch=$batch --process=copy --stream=$@ &
sleep 0.5
sudo ~/new-b4-assignment/build/recv.out --stream=off --process=copy --batch=32

#sudo ~/new-b4-assignment/build/clt.out --batch=$batch --process=copy --stream=$@ &
#sleep 0.1
#sudo ~/new-b4-assignment/build/recv.out --batch=32 &
#sleep 0.1
#sudo ${perf} ~/new-b4-assignment/srv.out --batch=32

