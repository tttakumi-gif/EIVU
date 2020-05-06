#include <iostream>
#include <string>
#include <thread>
#include <atomic>

class q {
public:
	std::atomic<int> qq;

	q();
};
