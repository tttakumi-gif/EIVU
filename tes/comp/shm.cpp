#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include <iostream>
#include <cstring>

#define FILE_MODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)

struct tt {
	int qw;
	int *qa;
};

int main() {
	int *iptr;
	int *pool;

	int qfd, qfde, qret, qrete;
	qfd = shm_open("qshared_memory",  O_CREAT | O_RDWR, FILE_MODE);
	qfde = errno;
	if(qfd == -1) {
		fprintf (stderr, "shm_open failed. err=%s\n\n", std::strerror (qfde));
		exit (EXIT_FAILURE);
	}

	qret = ftruncate (qfd, 4096);
	qrete = errno;

	int (*nums)[16] = (int (*)[16])mmap (NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_SHARED, qfd, 0);
	
	/*for(int i = 0; i < 16; i++) {
		nums[i] = (int*)malloc(sizeof(int));
	}

	for(int i = 0; i < 16; i++) {
		pool[i] = i;
		&nums[i] = &pool[i];
	}*/

	if(nums == MAP_FAILED) {
		fprintf (stderr, "mmap failed\n");
		exit (EXIT_FAILURE);
	}

	int wfd, wfde, wret, wrete;
	wfd = shm_open("wshared_memory", O_CREAT | O_RDWR, FILE_MODE);
	wfde = errno;
	if(wfd == -1) {
		fprintf (stderr, "shm_open failed. err=%s\n\n", std::strerror (wfde));
		exit (EXIT_FAILURE);
	}

	wret = ftruncate (wfd, 4096);

	//pool = (int*)mmap(NULL, sizeof(int) * 16, PROT_READ | PROT_WRITE, MAP_SHARED, wfd, 0);
	pool = (int*)malloc(sizeof(int) * 16);

	int fd, fde, ret, rete;
	fd = shm_open("shared_memory",  O_CREAT | O_RDWR, FILE_MODE);
	fde = errno;
	if(fd == -1) {
		fprintf (stderr, "shm_open failed. err=%s\n\n", std::strerror (fde));
		exit (EXIT_FAILURE);
	}

	ret = ftruncate (fd, sizeof(int));
	rete = errno;

	iptr = (int*)mmap (NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(iptr == MAP_FAILED) {
		fprintf (stderr, "mmap failed\n");
		exit (EXIT_FAILURE);
	}

	puts("fin");
	*iptr = 0;
	while(*iptr == 0) {
		;
	}

	std::cout << "num: " << *iptr << std::endl;

	for(int i = 0; i < 16; i++) {
		std::cout << *(nums[i]) << std::endl;
	}

	if(ret = shm_unlink("qshared_memory")) {
		fprintf(stderr, "shm_unlink(shm_struct) failed\n");
		exit(EXIT_FAILURE);
	}

	if(ret = shm_unlink("shared_memory")) {
		fprintf(stderr, "shm_unlink(shm_struct) failed\n");
		exit(EXIT_FAILURE);
	}

	return 0;
}
