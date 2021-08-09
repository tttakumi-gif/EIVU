#! /bin/bash

opt_perf="-e l1d.replacement,l1d_pend_miss.fb_full,l1d_pend_miss.pending,l2_lines_in.all,l2_lines_in.e,l2_lines_in.i,l2_lines_in.s,mem_load_uops_retired.l1_hit,mem_load_uops_retired.l2_hit,mem_load_uops_retired.l3_hit,mem_load_uops_retired.l1_miss,mem_load_uops_retired.l2_miss,mem_load_uops_retired.l3_miss,mem_load_uops_retired.hit_lfb,l2_rqsts.all_rfo,l2_rqsts.rfo_hit,l2_rqsts.rfo_miss"
sudo perf stat $opt_perf --cpu 5 -r 60 ./a0.out &
sleep 0.3                             
sudo perf stat $opt_perf --cpu 6 -r 60 ./a1.out &
sleep 0.3                           
sudo perf stat $opt_perf --cpu 7 -r 60 ./a2.out 
