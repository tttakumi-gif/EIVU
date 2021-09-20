#!/usr/bin/env python

# This script is written referring to
#   Basic Performance Measurements for AMD Athlon 64,
#   AMD Opteron and AMD Phenom Processors
#   http://developer.amd.com/Assets/intro_to_ca_v3_final.pdf


import sys
import re
import subprocess
import types

# Modify this list as you like.
# The format of each element is:
#   select: `NNN' for "perf stat -e rNNN"
#   name:   Event name used in output
events = [
    {"select": "c0", "name": "Retired Instructions"},
    {"select": "40", "name": "Data Cache Accesses"},
    {"select": "1e42", "name": "Data Cache Refills from L2"},
    {"select": "1e43", "name": "Data Cache Refills from System (Northbridge)"},
    {"select": "c80", "name": "Instruction Cache Fetches"},
    {"select": "c82", "name": "Instruction Cache Refills from L2"},
    {"select": "c83", "name": "Instruction Cache Refills from System (Northbridge)"},
    {"select": "c47d", "name": "Requests to L2 Cache [TLB fill]"},
    {"select": "c47e", "name": "L2 Cache Misses [TLB fill]"},
    {"select": "cf74e0", "name": "Read Requests to L3 Cache"},
    {"select": "cf74e1", "name": "L3 Cache Misses"}
    ]

def get_event_with_name(name):
    return [event for event in events if event["name"] == name][0]

def event_list():
    ret = ""
    for event in events:
        ret += "-e r" + event["select"] + "   "
    return ret

def store_count_to_events(perf_output_lines):
    # Sample perf-stat output line
    #        1565650  raw 0xc0                 #      0.000 M/sec    ( +-   1.457% )  (scaled from 75.04%)
    #  <not counted>  raw 0xc0                 #      0.000 M/sec
    count_pat = re.compile('^ <span style="font-weight:bold;">([0-9]+|<not counted>).</span>')
    variance_pat = re.compile("\+- *([0-9]+\.[0-9]+)%")

    lines = perf_output_lines
    for event in events:
        search_str = "raw 0x" + event["select"]
        event_line = "".join([line for line in lines if line.find(search_str) != -1])

        count_mat = count_pat.match(event_line)
        variance_mat = variance_pat.search(event_line)

        if count_mat is None:
            print("Unexpected event format:\n  %s"
                  % event_line)
            exit(1)
        if count_mat.group(1) != "<not counted>":
            event["count"] = int(count_mat.group(1))
            if variance_mat is not None:
                event["variance"] = float(variance_mat.group(1))
            else:
                event["variance"] = -1.0
        else:
            event["count"] = -1
            event["variance"] = -1.0

def print_running_env(perf_output_lines):
    search_str = "Performance counter stats for"
    print("".join([line for line in perf_output_lines if line.find(search_str) != -1]))

def print_elapsed_time(perf_output_lines):
    search_str = "seconds time elapsed"
    print("".join([line for line in perf_output_lines if line.find(search_str) != -1]))

def print_counted_events():
    for event in events:
        if event["count"] >= 0 and event["variance"] >= 0.0:
            print("%15s   %55s   ( +- %6.2f%%)"
                  % (str(event["count"]), event["name"], event["variance"]))
        elif event["count"] >= 0 and event["variance"] < 0.0:
            print("%15s   %55s"
                  % (str(event["count"]), event["name"]))
        else:
            print("%15s   %55s"
                  % ("<not counted>", event["name"]))

def print_calculated_events():
    def _clever_calc(rhs, terms):
        """
        @returns:
        if `terms' does not have any value less than 0:
            The result of rhs
        else:
            Value less than 0
        """
        if len([term for term in terms if term < 0]) > 0:
            return -1
        else:
            return rhs

    def _clever_print(val, ev_name):
        """
        @parameters:
        val: Value to print. Must be int or float.
             Value less than 0 means that the event is <not counted>.
        """
        is_valid_val = (val >= 0)
        if is_valid_val and type(val) == types.IntType:
            print("%15d   %s" % (val, ev_name))
        elif is_valid_val and type(val) == types.FloatType:
            print("%14.3f%%   %s" % (100.0 * val, ev_name))
        elif not is_valid_val:
            print("%15s   %s" % ("<not counted>", ev_name))
        else:
            print("Unexpected calculated value: " + str(val))

    retired_instructions = get_event_with_name("Retired Instructions")["count"]
    _clever_print(retired_instructions, "Retired Instructions")

    # Data Cache
    data_cache_accesses = get_event_with_name("Data Cache Accesses")["count"]
    _clever_print(data_cache_accesses, "Data Cache Accesses")

    data_cache_request_rate = _clever_calc(float(data_cache_accesses) / float(retired_instructions),
                                           [data_cache_accesses, retired_instructions])
    _clever_print(data_cache_request_rate, "Data Cache Request Rate")

    data_cache_refills_from_L2 = get_event_with_name("Data Cache Refills from L2")["count"]
    data_cache_refills_from_system = get_event_with_name("Data Cache Refills from System (Northbridge)")["count"]
    data_cache_misses = _clever_calc(data_cache_refills_from_L2 + data_cache_refills_from_system,
                                     [data_cache_refills_from_L2, data_cache_refills_from_system])
    _clever_print(data_cache_misses, "Data Cache Misses")

    data_cache_miss_ratio = _clever_calc(float(data_cache_misses) / float(data_cache_accesses),
                                         [data_cache_misses, data_cache_accesses])
    _clever_print(data_cache_miss_ratio, "Data Cache Miss Ratio")

    # Instruction Cache
    instruction_cache_fetches = get_event_with_name("Instruction Cache Fetches")["count"]
    _clever_print(instruction_cache_fetches, "Instruction Cache Fetches")

    instruction_cache_request_rate = _clever_calc(float(instruction_cache_fetches) / float(retired_instructions),
                                                  [instruction_cache_fetches, retired_instructions])
    _clever_print(instruction_cache_request_rate, "Instruction cache Request Rate")

    instruction_cache_refills_from_L2 = get_event_with_name("Instruction Cache Refills from L2")["count"]
    instruction_cache_refills_from_system = get_event_with_name("Instruction Cache Refills from System (Northbridge)")["count"]
    instruction_cache_misses = _clever_calc(instruction_cache_refills_from_L2 + instruction_cache_refills_from_system,
                                            [instruction_cache_refills_from_L2, instruction_cache_refills_from_system])
    _clever_print(instruction_cache_misses, "Instruction Cache Misses")

    instruction_cache_miss_ratio = _clever_calc(float(instruction_cache_misses) / float(instruction_cache_fetches),
                                                [instruction_cache_misses, instruction_cache_fetches])
    _clever_print(instruction_cache_miss_ratio, "Instruction Cache Miss Ratio")

    # L2 Cache
    L2_misses_TLB = get_event_with_name("Requests to L2 Cache [TLB fill]")["count"]
    L2_requests = _clever_calc(data_cache_misses + instruction_cache_misses + L2_misses_TLB,
                               [data_cache_misses, instruction_cache_misses, L2_misses_TLB])
    _clever_print(L2_requests, "L2 Requests")

    L2_request_rate = _clever_calc(float(L2_requests) / float(retired_instructions),
                                   [L2_requests, retired_instructions])
    _clever_print(L2_request_rate, "L2 Request Rate")

    L2_misses = _clever_calc(data_cache_refills_from_system + instruction_cache_refills_from_system + L2_misses_TLB,
                             [data_cache_refills_from_system, instruction_cache_refills_from_system, L2_misses_TLB])
    _clever_print(L2_misses, "L2 Misses")

    L2_miss_ratio = _clever_calc(float(L2_misses) / float(L2_requests),
                                 [L2_misses, L2_requests])
    _clever_print(L2_miss_ratio, "L2 Miss Ratio")

    # L3 Cache
    L3_requests = get_event_with_name("Read Requests to L3 Cache")["count"]
    _clever_print(L3_requests, "Read Requests to L3 Cache")

    L3_request_rate = _clever_calc(float(L3_requests) / float(retired_instructions),
                                   [L3_requests, retired_instructions])
    _clever_print(L3_request_rate, "L3 Request Rate")

    L3_misses = get_event_with_name("L3 Cache Misses")["count"]
    _clever_print(L3_misses, "L3 Misses")

    L3_miss_ratio = _clever_calc(float(L3_misses) / float(L3_requests),
                                 [L3_misses, L3_requests])
    _clever_print(L3_miss_ratio, "L3 Miss Ratio")


def main():
    command = "perf stat " + event_list() + " ".join(sys.argv[1:])
    p = subprocess.Popen(command, shell=True,
                         stderr=subprocess.PIPE)
    perf_output_lines = p.stderr.readlines()
    store_count_to_events(perf_output_lines)

    print("=== Running Environment ===")
    print_running_env(perf_output_lines)
    print("=== Counted Events ===")
    print_counted_events()
    print("\n=== Calculated Events ===")
    print_calculated_events()
    print("\n=== Elapsed Time ===")
    print_elapsed_time(perf_output_lines)

if __name__ == "__main__":
    main()
