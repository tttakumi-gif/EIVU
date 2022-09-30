#pragma once

#include "../header.hpp"

pid_t gettid() {
	return syscall(SYS_gettid);
}

void bind_core(int coreid) {
	cpu_set_t cpu_set;
	CPU_ZERO(&cpu_set);
	CPU_SET(coreid, &cpu_set);
	int result = sched_setaffinity(gettid(), sizeof(cpu_set_t), &cpu_set);
	assert(result == 0);
}
