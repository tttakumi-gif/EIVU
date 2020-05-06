#include "a.hpp"

q::q() {
	std::cout << "construct" << std::endl;
}

int main() {
	q *test;
	test = (q*)malloc(sizeof(q));
	*test = q();
	std::atomic<uint16_t> count{0};
	std::thread th1([&count] {
		for(int i = 0; i < 100000; i++) {
			count++;
		}
	});
	std::thread th2([&count] {
		for(int i = 0; i < 100000; i++) {
			count++;
		}
	});
	std::thread th3([&count] {
		for(int i = 0; i < 100000; i++) {
			count++;
		}
	});
	std::thread th4([&count] {
		for(int i = 0; i < 100000; i++) {
			count++;
		}
	});
	std::thread th5([&count] {
		for(int i = 0; i < 100000; i++) {
			count++;
		}
	});
	th1.join();
	th2.join();
	th3.join();
	th4.join();
	th5.join();
	std::cout << count << std::endl;
}
