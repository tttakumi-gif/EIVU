#pragma once

#include "header.hpp"

bool judge_process(std::string);

enum process_srv : int_fast8_t {
	COPY,
	MOVE,
};

class info_opt {
public:
	int_fast8_t size_batch;
	process_srv process;
	int_fast32_t num;

	info_opt() {
		size_batch = 32;
		process = MOVE;
		num = -1;
	}
};

struct option longopts[] = {
	{"batch", required_argument, nullptr, 'b'},
	{"process", required_argument, nullptr, 'p'},
	{"num", required_argument, nullptr, 'n'},
	{0, 0, 0, 0},
};

constexpr int SIZE_OPT = sizeof(longopts) / sizeof(option);

info_opt get_opt(int argc, char **argv) {
	int opt;
	info_opt result;
	std::string cstr;
	while((opt = getopt_long(argc, argv, "abc:d::", longopts, nullptr)) != -1) {
		switch(opt) {
			case 'b':
				result.size_batch = atoi(optarg);
				break;
			case 'p':
				if(judge_process(optarg)) {
					result.process = COPY;
				}
				break;
			case 'n':
				result.num = atoi(optarg);
				break;
		}
	}

#if 0
	for(int i = 0; i < SIZE_OPT; i++) {
		if(((pr.result >> i) & 1) == 1) {
			std::printf("mrszk: %c\n", longopts[i].val);
		}
	}
#endif
	return result;
}

bool judge_process(std::string optarg) {
	if(optarg == "copy" || optarg == "c") {
		return true;
	}
	else if(optarg != "move" && optarg != "m") {
		std::printf("invalid argument \"--process=%s\"\n", optarg.c_str());
	}

	return false;
}
