#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <error.h>
#include <stdlib.h>
#include <string.h>

#define KB * 1024
#define MB *1024 KB
#define GB *1024 MB

void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

int partition(int arr[], int low, int high) {
    int p = arr[low];
    int i = low;
    int j = high;

    while (i < j) {
        while (arr[i] <= p && i <= high - 1) {
            i++;
        }

        while (arr[j] > p && j >= low + 1) {
            j--;
        }
        if (i < j) {
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[low], &arr[j]);
    return j;
}

void quickSort(int arr[], int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

bool many_series(int * arr, int n) {
    for (int i = 0; i < n-1; i++) {
        if (arr[i] > arr[i+1]) {
            return true;
        }
    }

    return false;
}

int open_and_get_size(const char * filename, const int oflag, const int sflag, int * size) {
    int fd = open(filename, oflag, sflag);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    *size = lseek(fd, 0, SEEK_END);

    return fd;
}

int main(int argc, char ** argv) {
    bool presort = false;
    if (argc < 2) {
      perror("Usage: ./improved <filename>");
      exit(EXIT_FAILURE);
    }

    char * filename = argv[1];

    if (argc > 2) {
      if ( strcmp(argv[2], "-presort") == 0 ) {
          presort = true;
      }
    }

    fflush(stdin);
    int read_a, write_a, write_b, write_c, read_b, read_c;
    bool b_done, c_done;
    int n, prev, b_prev, c_prev;
    bool flag, series_b, series_c;
    int * curr_stream;
    bool many_series;
    int b_n, c_n, ens;

    int * garbage = 0;
    int i;
    int t;
    int sz;
    int szb;
    int szc;

    int a = 0;
    int b = 0;
    int c = 0;
    int * map;

    clock_t start, end;


    start = clock();

    if ( presort ) {
        int low = 0, high;
        ens = open_and_get_size(filename, O_RDWR, S_IRUSR | S_IWUSR, &sz);
        map = (int*)mmap(NULL, sz, PROT_READ | PROT_WRITE, MAP_SHARED, ens, 0);
        if ( map == MAP_FAILED ) {
            perror("mmap");
            exit(EXIT_FAILURE);
        }

        while ( low < high ) {
            high = ( sz - low >=  4 MB ) ? 4 MB : sz - low;
            quickSort(map, low / sizeof(int), high / sizeof(int));
            low = high;
        }

        if ( munmap(map, sz) == -1 ) {
            perror("munmap");
            exit(EXIT_FAILURE);
        }

        if ( close(ens) < 0 ) {
            perror("close");
            exit(EXIT_FAILURE);
        }

        printf("Sorted\n");
    }

    while (true) {
        a = 0;
        b = 0;
        c = 0;
        ens = open_and_get_size(filename, O_CREAT | O_RDWR, S_IWUSR | S_IRUSR, &sz);
        map = (int*)mmap(NULL, sz, PROT_READ, MAP_SHARED, ens, 0);

        if (map == MAP_FAILED) {
            perror("mmap");
            exit(EXIT_FAILURE);
        }

        t = INT_MIN;
        many_series = false;
        for (i = 0; i < sz / sizeof(int); i++) {
            if (map[i] < t)	{
                many_series = true;
                break;
            }
            t = map[i];
        }

        if ( munmap(map, sz) ) {
            perror("munmap");
            exit(EXIT_FAILURE);
        }

        close(ens);

        if ( !many_series ) {
            printf("Done\n");

            end = clock() - start;

            printf("Time taken %lfs", ((double)end) / CLOCKS_PER_SEC);
            exit(EXIT_SUCCESS);
        }

        read_a = open_and_get_size(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR, &sz);
        int * map_a = (int*)mmap(NULL, sz, PROT_READ | PROT_WRITE, MAP_SHARED, read_a, 0);
        if ( map_a == MAP_FAILED ) {
            perror("mmap");
            exit(EXIT_FAILURE);
        }

        write_b = open_and_get_size("B", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR, &garbage);
        if ( ftruncate( write_b, sz ) < 0 ) {
            perror("ftruncate");
            exit(EXIT_FAILURE);
        }

        int * map_b = (int*)mmap(NULL, sz, PROT_READ | PROT_WRITE, MAP_SHARED, write_b, 0);
        if ( map_b == MAP_FAILED ) {
            perror("mmap");
            exit(EXIT_FAILURE);
        }

        write_c = open_and_get_size("C", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR, &garbage);
        if ( ftruncate( write_c, sz ) < 0 ) {
            perror("ftruncate");
            exit(EXIT_FAILURE);
        }

        int * map_c = (int*)mmap(NULL, sz, PROT_READ | PROT_WRITE, MAP_SHARED, write_c, 0);
        if ( map_c == MAP_FAILED ) {
            perror("mmap");
            exit(EXIT_FAILURE);
        }

        prev = INT_MIN;
        flag = true;
        for (int i = 0; i < sz / sizeof(int); i++) {
            if (map_a[i] < prev) {
                flag = !flag;
                prev = INT_MIN;
            }
            if (flag) {
                map_b[b] = map_a[i];
                b += 1;
            } else {
                map_c[c] = map_a[i];
                c += 1;
            }

            prev = map_a[i];
        }

        if ( munmap(map_a, sz) < 0 ) {
            perror("munmap");
            exit(EXIT_FAILURE);
        }

        if ( munmap(map_b, sz) < 0 ) {
            perror("munmap");
            exit(EXIT_FAILURE);
        }

        if ( munmap(map_c, sz) < 0 ) {
            perror("munmap");
            exit(EXIT_FAILURE);
        }

        if ( ftruncate( write_b, b * sizeof(int) ) < 0 ) {
            perror("ftruncate");
            exit(EXIT_FAILURE);
        }

        if ( ftruncate( write_c, c * sizeof(int) ) < 0 ) {
            perror("ftruncate");
            exit(EXIT_FAILURE);
        }

        close(write_b);
        close(write_c);
        close(read_a);

        if ( remove(filename) != 0 ) {
            perror("remove");
            exit(EXIT_FAILURE);
        }
        write_a = open_and_get_size(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR, &garbage);
        if ( ftruncate( write_a, sz ) < 0) {
            perror("ftruncate");
            exit(EXIT_FAILURE);
        }

        map_a = (int*)mmap(NULL, sz, PROT_READ | PROT_WRITE, MAP_SHARED, write_a, 0);
        if ( map_a == MAP_FAILED ) {
            perror("mmap");
            exit(EXIT_FAILURE);
        }

        read_b = open_and_get_size("B", O_RDWR, S_IRUSR | S_IWUSR, &szb);
        map_b = (int*)mmap(NULL, szb, PROT_READ | PROT_WRITE, MAP_SHARED, read_b, 0);
        if ( map_b == MAP_FAILED ) {
            perror("mmap");
            exit(EXIT_FAILURE);
        }

        read_c = open_and_get_size("C", O_RDWR, S_IRUSR | S_IWUSR, &szc);
        map_c = (int*)mmap(NULL, szc, PROT_READ | PROT_WRITE, MAP_SHARED, read_c, 0);
        if ( map_c == MAP_FAILED ) {
            perror("mmap");
            exit(EXIT_FAILURE);
        }

        a = 0;
        b = 0;
        c = 0;

        b_done = b >= szb / sizeof(int);
        c_done = c >= szc / sizeof(int);

        b_prev = INT_MIN;
        c_prev = INT_MIN;
        series_b = true;
        series_c = true;
        b_n = map_b[0]; // mb error
        c_n = map_c[0]; // mb error

        b += 1;
        c += 1;

        while ( !( b_done & c_done ) )
        {
            series_b = b_prev <= b_n;
            series_c = c_prev <= c_n;
            if ( !b_done & ( series_b & ( b_n <= c_n | !series_c ) ) | c_done) {
                b_prev = b_n;
                map_a[a] = b_n;
                b_n = map_b[b];
                b += 1;
                b_done = b > szb / sizeof(int);
                a += 1;
            } else if ( !c_done & ( series_c & ( c_n < b_n | !series_b ) ) | b_done) {
                c_prev = c_n;
                map_a[a] = c_n;
                c_n = map_c[c];
                c += 1;
                c_done = c > szc / sizeof(int);
                a += 1;
            } else if ( !series_b & !series_c ) {
                b_prev = INT_MIN;
                c_prev = INT_MIN;
            }
        }

        if ( munmap(map_a, sz) ) {
            perror("munmap");
            exit(EXIT_FAILURE);
        }

        if ( munmap(map_b, szb) ) {
            perror("munmap");
            exit(EXIT_FAILURE);
        }

        if ( munmap(map_c, szc) ) {
            perror("munmap");
            exit(EXIT_FAILURE);
        }

        close(write_a);
        close(read_b);
        close(read_c);

        if ( remove("B") < 0 ) {
            perror("remove B");
            exit(EXIT_FAILURE);
        }

        if ( remove("C") < 0) {
            perror("remove C");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}

