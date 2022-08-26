#!/bin/bash

#opt_perf="-e l1d.replacement,l1d_pend_miss.fb_full,l1d_pend_miss.pending,l2_lines_in.all,L1-dcache-loads,L1-dcache-load-misses,L1-dcache-stores,L1-dcache-store-misses,LLC-loads,LLC-load-misses,LLC-stores,LLC-store-misses,l2_rqsts.all_rfo,l2_rqsts.rfo_hit,l2_rqsts.rfo_miss,offcore_response.demand_rfo.l3_hit.hit_other_core_no_fwd,offcore_requests_outstanding.cycles_with_demand_rfo"
opt_perf="-M L1MPKI,L2MPKI,L3MPKI -e mem_load_retired.l1_hit,mem_load_retired.l2_hit,mem_load_retired.l3_hit,cache-misses,cache-references,l1d.replacement,l2_lines_out.non_silent,l2_lines_out.silent,l2_lines_out.useless_hwpf,offcore_requests.demand_rfo,offcore_requests_outstanding.demand_rfo,offcore_requests_outstanding.cycles_with_demand_rfo,offcore_response.all_rfo.any_response,offcore_response.demand_rfo.any_response,offcore_response.all_pf_rfo.any_response,offcore_response.pf_l2_rfo.any_response,offcore_response.pf_l3_rfo.any_response,LLC-loads,LLC-stores,l1d_pend_miss.fb_full,l1d_pend_miss.pending_cycles"
#opt_perf="-M L1MPKI,L2MPKI,L3MPKI -e l1d_pend_miss.pending_cycles,mem_load_retired.l1_hit,mem_load_retired.l1_miss,mem_load_retired.l2_hit,mem_load_retired.l2_miss,l1d.replacement,l2_lines_out.non_silent,l2_lines_out.silent,l2_lines_out.useless_hwpf,l2_rqsts.all_rfo,l2_rqsts.rfo_hit,l2_rqsts.rfo_miss,LLC-loads,LLC-stores,offcore_requests.demand_rfo,offcore_requests_outstanding.cycles_with_demand_rfo,offcore_requests_outstanding.demand_rfo,ocr.demand_rfo.l3_miss,ocr.hwpf_l2_rfo.l3_miss,ocr.demand_rfo.any_response,ocr.demand_rfo.dram,ocr.demand_rfo.l3_hit.any,ocr.demand_rfo.l3_hit.snoop_hit_no_fwd,ocr.demand_rfo.l3_hit.snoop_hitm,ocr.demand_rfo.l3_hit.snoop_miss,ocr.demand_rfo.l3_hit.snoop_not_needed,ocr.demand_rfo.l3_hit.snoop_sent,ocr.demand_rfo.local_dram,ocr.demand_rfo.local_dram,ocr.hwpf_l2_rfo.any_response,ocr.hwpf_l2_rfo.dram,ocr.hwpf_l2_rfo.l3_hit.any,ocr.hwpf_l2_rfo.l3_hit.snoop_hit_no_fwd,ocr.hwpf_l2_rfo.l3_hit.snoop_hitm,ocr.hwpf_l2_rfo.l3_hit.snoop_miss,ocr.hwpf_l2_rfo.l3_hit.snoop_not_needed,ocr.hwpf_l2_rfo.l3_hit.snoop_sent,ocr.hwpf_l2_rfo.local_dram"
#l2_rqsts.all_rfo,l2_rqsts.rfo_hit,l2_rqsts.rfo_miss,offcore_requests.demand_rfo,offcore_requests_outstanding.demand_rfo,mem_inst_retired.all_loads,mem_inst_retired.all_stores

#opt_perf="-e cache-misses,L1-dcache-load-misses,L1-dcache-loads,L1-dcache-stores,LLC-loads,LLC-stores,LLC-load-misses,LLC-store-misses,cpu/event=0xA3,umask=0x06,cmask=0x06,name=STALLS_L3_MISS/"
#opt_perf="-e cpu/event=0xA3,umask=0x06,cmask=0x06,name=STALLS_L3_MISS/,cycle_activity.stalls_l3_miss,cpu/event=0x60,umask=0x10,name=OUT_L3miss_Dem_RD/"

#i="-I 1000"
i=""
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

sudo ${perf} ../build/srv.out --stream=off --process=move --batch=32 &
#sudo ../build/srv.out --stream=off --process=move --batch=32 &
sleep 0.5
#sudo ${perf} ../build/clt.out --batch=$batch --process=copy --stream=$1 &
sudo ../build/clt.out --batch=$batch --process=copy --stream=$1 &
sleep 0.5 
#sudo ${perf} ../build/recv.out --stream=$2 --process=copy --batch=32
sudo ../build/recv.out --stream=$2 --process=copy --batch=32

#sudo ~/new-b4-assignment/build/clt.out --batch=$batch --process=copy --stream=$@ &
#sleep 0.1
#sudo ~/new-b4-assignment/build/recv.out --batch=32 &
#sleep 0.1
#sudo ${perf} ~/new-b4-assignment/srv.out --batch=32

