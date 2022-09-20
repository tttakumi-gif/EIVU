# mock-vhost-user

## Introduction

mock-vhost-user is an inter-process communication program that follows the process of DPDK(vhost-user).
Rx generates packets and copies them to NF.
Tx then copies the packets from NF.

This program uses virtqueue (`vq`) and packet buffer pool (array of `buf`).

## Run mock-vhost-user

### Build mock-vhost-user

Use cmake to build.
Execute the following command.
Then, three binaries are generated: "rx.out", "nf.out", and "tx.out".

```
$ mkdir build && cd build
$ cmake ../
$ make
```

### Run mock-vhost-user

"run.sh" is copied to the build directory when configuring cmake.
When this "run.sh" is executed, three programs ("rx.out", "nf.out", and "tx.out") run and the throughput is displayed when the process is complete.

```
$ ./run.sh
```

### Profiling mock-vhost-user

"statrun.sh" is also copied after configuring.
Executing this "statrun.sh" will output the profile results for each thread in "results/results.txt".
Profiling items can be specified in A opt_perf and can be changed to any event supported by the machine.

```
$ ./statrun.sh
```
