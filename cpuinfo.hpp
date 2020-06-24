#pragma once

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "sys/times.h"
#include "sys/vtimes.h"

//#include <omp.h>
#include <iostream>

static clock_t lastCPU, lastSysCPU, lastUserCPU;
static int numProcessors;

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
	if (now <= lastCPU || timeSample.tms_stime < lastSysCPU ||
		timeSample.tms_utime < lastUserCPU){
		//Overflow detection. Just skip this value.
		percent = -1.0;
	}
	else{
		percent = (timeSample.tms_stime - lastSysCPU) +
			(timeSample.tms_utime - lastUserCPU);
		percent /= (now - lastCPU);
		percent /= numProcessors;
		percent *= 100;
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
