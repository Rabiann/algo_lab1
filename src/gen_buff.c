#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <error.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 4 * 1024 

int main() {
	int* tmp_rand;
	srand(time(NULL));
	char * filename = "test_buff";

	int fd = open(filename, O_CREAT | O_RDWR, S_IWUSR | S_IRUSR);

	if ( fd < 0 ) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	if ( ftruncate(fd, SIZE) < 0 ) {
		perror("truncate");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < SIZE / sizeof(int); i++) {
		*tmp_rand = rand();
		write(fd, tmp_rand, 4);
	}	

	if ( close(fd) < 0 ) {
		perror("close");
		exit(EXIT_FAILURE);
	}

	write(1, "Done!\n", 7);

	exit(EXIT_SUCCESS);
} 
