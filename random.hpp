#pragma once

#include <functional>
#include <random>

class random_generator {
private:
	std::random_device rnd;
	std::mt19937 mt;
	std::uniform_int_distribution<> rand;

public:
	random_generator();
	void set_range(int, int);
	int get_random();
};

random_generator::random_generator() {
}

void random_generator::set_range(int begin, int end) {
	//mt = std::mt19937(rnd());
	mt = std::mt19937(6);
	rand = std::uniform_int_distribution<>(begin, end);
}

int random_generator::get_random() {
	return rand(mt);
}

