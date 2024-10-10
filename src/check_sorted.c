#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>

int main(int argc, char ** argv) {
	if ( argc < 2 ) {
		perror("Filename should be provided");
		exit(EXIT_FAILURE);
	}

	char * filename = argv[1];
	
	int fd = open(filename, O_RDONLY, S_IRUSR);

	if ( fd < 0 ) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	off_t filesize = lseek(fd, 0, SEEK_END);	

	if ( filesize < 0 ) {
		perror("lseek");
		exit(EXIT_FAILURE);
	}	

	int * map = (int*)mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, fd, 0);

	if ( map == MAP_FAILED ) {
		perror("mmap");
		exit(EXIT_FAILURE);
	}

	int t = -1;

    for (int i = 0; i < filesize / sizeof(int); i++) {
    	if ( map[i] < t ) {
    		printf("Records are unsorted!");
    		exit(EXIT_SUCCESS);
    	}
    	t = map[i];
    }

    write(1, "Sorted successfully!\n", 21);

	if ( munmap(map, filesize) < 0 ) {
		perror("munmap");
		exit(EXIT_FAILURE);
	}

	if ( close(fd < 0 ) ) {
		perror("close");
		exit(EXIT_FAILURE);
	}
	
	exit(EXIT_SUCCESS);
}
