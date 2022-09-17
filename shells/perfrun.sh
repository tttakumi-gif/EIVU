#!/bin/bash

#opt_perf="-e l1d.replacement,l1d_pend_miss.fb_full,l1d_pend_miss.pending,l2_lines_in.all,L1-dcache-loads,L1-dcache-load-misses,L1-dcache-stores,L1-dcache-store-misses,LLC-loads,LLC-load-misses,LLC-stores,LLC-store-misses,l2_rqsts.all_rfo,l2_rqsts.rfo_hit,l2_rqsts.rfo_miss,offcore_response.demand_rfo.l3_hit.hit_other_core_no_fwd,offcore_requests_outstanding.cycles_with_demand_rfo"
opt_perf="-M L1MPKI,L2MPKI,L3MPKI -e l1d.replacement,l2_lines_out.non_silent,l2_lines_out.silent,l2_lines_out.useless_hwpf,offcore_requests.demand_rfo,offcore_requests_outstanding.demand_rfo,offcore_requests_outstanding.cycles_with_demand_rfo,offcore_response.all_rfo.any_response,offcore_response.demand_rfo.any_response,offcore_response.all_pf_rfo.any_response,offcore_response.pf_l2_rfo.any_response,offcore_response.pf_l3_rfo.any_response,LLC-loads,LLC-stores,l1d_pend_miss.fb_full" 
#l2_rqsts.all_rfo,l2_rqsts.rfo_hit,l2_rqsts.rfo_miss,offcore_requests.demand_rfo,offcore_requests_outstanding.demand_rfo,mem_inst_retired.all_loads,mem_inst_retired.all_stores

#opt_perf="-e cache-misses,L1-dcache-load-misses,L1-dcache-loads,L1-dcache-stores,LLC-loads,LLC-stores,LLC-load-misses,LLC-store-misses,cpu/event=0xA3,umask=0x06,cmask=0x06,name=STALLS_L3_MISS/"
#opt_perf="-e cpu/event=0xA3,umask=0x06,cmask=0x06,name=STALLS_L3_MISS/,cycle_activity.stalls_l3_miss,cpu/event=0x60,umask=0x10,name=OUT_L3miss_Dem_RD/"

#i="-I 1000"
i=""
loop=1
batch=32
#perf="perf stat $opt_perf -r $loop $i"
perf="perf c2c record -c 1"
#perf=""

#run() {
#  #sudo ${perf} ../build/srv.out --stream=off --process=move --batch=$batch &
#  sudo ../build/srv.out --stream=off --process=move --batch=$batch &
#  sleep 1
#  sudo ${perf} ../build/clt.out --stream=off --batch=$batch --process=copy &
#  #sudo ../build/clt.out --batch=$batch --process=copy --stream=$1 &
#  sleep 1 
#  #sudo ${perf} ../build/recv.out --stream=$2 --process=copy --batch=$batch
#  sudo ../build/recv.out --stream=off --process=copy --batch=$batch
#}

run() {
  sudo $2 ../build/srv.out --stream=off --process=move --batch=$batch &
  sleep 1
  sudo $1 ../build/clt.out --stream=$4 --batch=$batch --process=copy &
  sleep 1 
  sudo $3 ../build/recv.out --stream=$5 --process=copy --batch=$batch
}

outputfile() {
  sudo perf mem -t load report --sort=mem --stdio > ./$1/perfmem
  sudo perf c2c report --stdio > ./$1/perfc2c
}

rm -rf results
mkdir results
mkdir results/clt results/srv results/recv

gap=1
rxstream=off
txstream=off

# perf clt
echo "start clt"
run "${perf}" "" "" ${rxstream} ${txstream} > ./results/clt/throughput
#run "${perf}" "" "" off off
sleep ${gap}
outputfile results/clt
mv perf.data ./results/clt/
sudo chown sdn results/clt/perf.data

# perf srv
echo "start srv"
run "" "${perf}" "" ${rxstream} ${txstream} > ./results/srv/throughput
sleep ${gap}
outputfile results/srv
mv perf.data ./results/srv/
sudo chown sdn results/srv/perf.data

# perf recv
echo "start recv"
run "" "" "${perf}" ${rxstream} ${txstream} > ./results/recv/throughput
sleep ${gap}
outputfile results/recv
mv perf.data ./results/recv/
sudo chown sdn results/recv/perf.data

