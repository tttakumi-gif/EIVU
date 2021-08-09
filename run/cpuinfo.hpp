#pragma once

#include "header.hpp"

clock_t lastCPU, lastSysCPU, lastUserCPU;
int numProcessors;

enum cusage {
	CLT_PS,
	CLT_PR,
	CLT_T,
	SRV_PS,
	SRV_PR,
	SRV_T,
};

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

void init_p(){
	FILE* file;
	struct tms timeSample;
	char line[128];

	lastCPU = times(&timeSample);
	lastSysCPU = timeSample.tms_stime;
	lastUserCPU = timeSample.tms_utime;

	file = fopen("/proc/cpuinfo", "r");
	numProcessors = 0;
	while(fgets(line, 128, file) != NULL){
		if (strncmp(line, "processor", 9) == 0) numProcessors++;
	}
	fclose(file);
}

double getCurrentValue_p(){
	struct tms timeSample;
	clock_t now;
	double percent;

	now = times(&timeSample);
	if (now <= lastCPU || timeSample.tms_stime < lastSysCPU || timeSample.tms_utime < lastUserCPU){
		//Overflow detection. Just skip this value.
		percent = -1.0;
	}
	else{
		percent = (timeSample.tms_stime - lastSysCPU) + (timeSample.tms_utime - lastUserCPU);
		percent /= (now - lastCPU);
		percent /= numProcessors;
		percent *= 100;
//		percent *= 4;
	}
	lastCPU = now;
	lastSysCPU = timeSample.tms_stime;
	lastUserCPU = timeSample.tms_utime;

	return percent;
}

//int main() {
//	init();
//	unsigned long long num = 10000000000;
//#pragma omp parallel for
//	for(unsigned long long i = 0; i < num; i++) {
//		if(i == num / 2) {
//			std::cout << omp_get_num_threads() << std::endl;
//			std::cout << getCurrentValue() << std::endl;
//		}
//	}
//	if(omp_get_thread_num() == 0) {
//			std::cout << getCurrentValue() << std::endl;
//	}
//}

static unsigned long long lastTotalUser, lastTotalUserLow, lastTotalSys, lastTotalIdle;

void init_t(){
	FILE* file = fopen("/proc/stat", "r");
	int temp = fscanf(file, "cpu %llu %llu %llu %llu", &lastTotalUser, &lastTotalUserLow, &lastTotalSys, &lastTotalIdle);
	assert(-1 < temp);
	fclose(file);
}

double getCurrentValue_t(){
	double percent;
	FILE* file;
	unsigned long long totalUser, totalUserLow, totalSys, totalIdle, total;

	file = fopen("/proc/stat", "r");
	int temp = fscanf(file, "cpu %llu %llu %llu %llu", &totalUser, &totalUserLow, &totalSys, &totalIdle);
	assert(-1 < temp);
	fclose(file);

	if (totalUser < lastTotalUser || totalUserLow < lastTotalUserLow || totalSys < lastTotalSys || totalIdle < lastTotalIdle){
		//Overflow detection. Just skip this value.
		percent = -1.0;
	}
	else{
		total = (totalUser - lastTotalUser) + (totalUserLow - lastTotalUserLow) + (totalSys - lastTotalSys);
		percent = total;
		total += (totalIdle - lastTotalIdle);
		percent /= total;
		percent *= 100;
	}

	lastTotalUser = totalUser;
	lastTotalUserLow = totalUserLow;
	lastTotalSys = totalSys;
	lastTotalIdle = totalIdle;

	return percent;
}
