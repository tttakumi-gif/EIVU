#!/bin/bash

#opt_perf="-M L1MPKI,L2MPKI,L3MPKI -e mem_load_retired.l1_hit,mem_load_retired.l2_hit,mem_load_retired.l3_hit,l1d.replacement,l2_lines_out.non_silent,l2_lines_out.silent,l2_lines_out.useless_hwpf,offcore_requests.demand_rfo,offcore_requests_outstanding.demand_rfo,offcore_requests_outstanding.cycles_with_demand_rfo,offcore_response.all_rfo.any_response,offcore_response.demand_rfo.any_response,offcore_response.all_pf_rfo.any_response,offcore_response.pf_l2_rfo.any_response,offcore_response.pf_l3_rfo.any_response,LLC-loads,LLC-stores,l1d_pend_miss.fb_full,l1d_pend_miss.pending_cycles"
opt_perf=""


#i="-I 1000"
i=""
loop=1
perf="perf stat $opt_perf -r $loop $i"
#perf=""

batch_rx=32
batch_nf=32
batch_tx=32

run() {
  sudo $2 ./nf.out --stream=off --process=move --batch=$batch_nf  &
  sleep 1
  sudo $1 ./rx.out --stream=$4 --batch=$batch_rx --process=copy  &
  sleep 1 
  sudo $3 ./tx.out --stream=$5 --process=copy --batch=$batch_tx 
}

rm -rf results
mkdir results

output_file="results.txt"

gap=1
rxstream=off
txstream=off

# perf rx
echo "start profiling of rx"
echo "Rx Thread" > ./results/${output_file}
run "${perf}" "" "" ${rxstream} ${txstream} &>> ./results/${output_file}
#run "${perf}" "" "" off off

# perf nf
echo "start profiling of nf"
echo "NF Thread" >> ./results/${output_file}
run "" "${perf}" "" ${rxstream} ${txstream} &>> ./results/${output_file}

# perf tx
echo "start profiling of tx"
echo "Tx Thread" >> ./results/${output_file}
run "" "" "${perf}" ${rxstream} ${txstream} &>> ./results/${output_file}

