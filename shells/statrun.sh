#!/bin/bash

opt_perf="-M L1MPKI,L2MPKI,L3MPKI -e l1d.replacement,l2_lines_out.non_silent,l2_lines_out.silent,l2_lines_out.useless_hwpf,offcore_requests.demand_rfo,offcore_requests_outstanding.demand_rfo,offcore_requests_outstanding.cycles_with_demand_rfo,offcore_response.all_rfo.any_response,offcore_response.demand_rfo.any_response,offcore_response.all_pf_rfo.any_response,offcore_response.pf_l2_rfo.any_response,offcore_response.pf_l3_rfo.any_response,LLC-loads,LLC-stores,l1d_pend_miss.fb_full" 


#i="-I 1000"
i=""
loop=1
batch=32
perf="perf stat $opt_perf -r $loop $i"
#perf="perf c2c record -c 1"
#perf=""

run() {
  sudo $2 ../build/srv.out --stream=off --process=move --batch=$batch  &
  sleep 1
  sudo $1 ../build/clt.out --stream=$4 --batch=$batch --process=copy  &
  sleep 1 
  sudo $3 ../build/recv.out --stream=$5 --process=copy --batch=$batch 
}

rm -rf results
mkdir results

gap=1
rxstream=off
txstream=off

# perf clt
echo "start clt"
echo "Rx Thread" >> ./results/result.txt
run "${perf}" "" "" ${rxstream} ${txstream} &>> ./results/result.txt
#run "${perf}" "" "" off off

# perf srv
echo "start srv"
echo "Guest Thread" >> ./results/result.txt
run "" "${perf}" "" ${rxstream} ${txstream} &>> ./results/result.txt

# perf recv
echo "start recv"
echo "Tx Thread" >> ./results/result.txt
run "" "" "${perf}" ${rxstream} ${txstream} &>> ./results/result.txt

