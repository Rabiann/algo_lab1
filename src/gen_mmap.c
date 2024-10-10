#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <error.h>
#include <stdlib.h>


int main(int argc, char ** argv) {
	if ( argc < 3 ) {
		perror("invalid arguments");
		exit(EXIT_FAILURE);
	}

	char * filename = argv[1];
	int size = atoi(argv[2]);
	srand(time(NULL));

	int fd = open(filename, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if ( fd < 0 ) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	if ( ftruncate(fd, size) < 0 ) {
		perror("truncate");
		exit(EXIT_FAILURE);
	}

	int * map = (int *)mmap(NULL, size, PROT_WRITE, MAP_SHARED, fd, 0);
	
	if ( map == MAP_FAILED ) {
		perror("mmap");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < size / sizeof(int); i++) {
		map[i] = rand();
	}	

	if ( munmap(map, size) < 0 ) {
		perror("munmap");
		exit(EXIT_FAILURE);
	}

	if ( close(fd) < 0 ) {
		perror("close");
		exit(EXIT_FAILURE);
	}

	write(1, "Done!", 6);

	exit(EXIT_SUCCESS);
}
