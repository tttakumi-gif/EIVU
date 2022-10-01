#pragma once

#include "../header.hpp"

enum process_srv : int_fast8_t {
    COPY,
    MOVE,
};

enum instruction_stream : int_fast8_t {
    ON,
    OFF,
};

class info_opt {
public:
    int size_batch;
    process_srv process;
    instruction_stream stream;
    int_fast32_t num;

    info_opt();

    void print_opt() const;
};

inline info_opt::info_opt() {
    size_batch = 32;
    process = MOVE;
    stream = OFF;
    num = -1;
}

inline void info_opt::print_opt() const {
    std::printf("*********** options ***********\n");
    std::printf("  --batch = %d\n", size_batch);
    std::printf("  --process = %s\n", (process == COPY) ? "copy" : "move");
    if (num != -1) {
        std::printf("  --num = %d\n", (int) num);
    }
    if (process == COPY) {
        std::printf("  --stream = %s\n", (stream == ON) ? "on" : "off");
    }
    std::printf("*******************************\n");
}

info_opt get_opt(int, char **);

bool judge_process(const std::string &);

bool judge_stream(const std::string &);

static struct option longopts[] = {
        {"batch",   required_argument, nullptr, 'b'},
        {"process", required_argument, nullptr, 'p'},
        {"num",     required_argument, nullptr, 'n'},
        {"stream",  required_argument, nullptr, 's'},
        {nullptr, 0,                   nullptr, 0},
};

constexpr int SIZE_OPT = sizeof(longopts) / sizeof(option);

info_opt get_opt(int argc, char **argv) {
    int opt;
    info_opt result;
    std::string cstr;
    while ((opt = getopt_long(argc, argv, "abc:d::", longopts, nullptr)) != -1) {
        switch (opt) {
            case 'b':
                result.size_batch = atoi(optarg);
                break;
            case 'p':
                if (judge_process(optarg)) {
                    result.process = COPY;
                } else {
                    result.process = MOVE;
                }
                break;
            case 's':
                if (judge_stream(optarg)) {
                    result.stream = ON;
                } else {
                    result.stream = OFF;
                }
                break;
            case 'n':
                result.num = atoi(optarg);
                break;
            default:
                std::printf("unknown argument: %s", reinterpret_cast<char *>(&opt));
        }
    }

#if 0
    for(int i = 0; i < SIZE_OPT; i++) {
        if(((pr.result >> i) & 1) == 1) {
            std::printf("mrszk: %c\n", longopts[i].val);
        }
    }
#endif
#ifdef PRINT
    if (result.num <= 0) {
        result.print_opt();
    }
#endif
    return result;
}

bool judge_process(const std::string &arg) {
    if (arg == "copy" || arg == "c") {
        return true;
    } else if (arg != "move" && arg != "m") {
        std::printf("invalid argument \"--process=%s\"\n", arg.c_str());
    }

    return false;
}

bool judge_stream(const std::string &arg) {
    if (arg == "on") {
        return true;
    } else if (arg != "off") {
#ifdef PRINT
        std::printf("invalid argument \"--process=%s\"\n", arg.c_str());
#endif
    }

    return false;
}
