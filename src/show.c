#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

int main(int argc, char ** argv) {
	if (argc < 2) {
        perror("Usage: ./show <filename>");
        return 1;
	}


    char * filename = argv[1];

    int fd = open(filename, O_RDONLY, S_IRUSR);

    int sz = lseek(fd, 0, SEEK_END);

    if (fd == -1) {
    	perror("open");
      	return 1;
    }

    int * map = mmap(0, sz, PROT_READ, MAP_SHARED, fd, 0);

    printf("Filename: %s\nSize: %d\nRecords: %d\n", filename, sz, (int)(sz / sizeof(int)));

    for (int i = 0; i < sz / sizeof(int); i++) {
    	printf("%d\n", map[i]);
    }

    if ( munmap(map, sz) == -1 ) {
        perror("munmap");
        return 1;
    }

    if ( close( fd ) < 0 ) {
      	perror("close");
        return 1;
    }

	return 0;
}
