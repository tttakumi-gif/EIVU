#include <sys/mman.h>
#include <sys/stat.h>

int main() {
	shm_unlink("shm_buf");
	return 0;
}
