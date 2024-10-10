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
#include <time.h>

#define KB * 1024
#define MB *1024 KB
#define GB *1024 MB

int main(int argc, char ** argv) {
    if ( argc < 2 ) {
        perror("Usage: ./basic <filename>");
        exit(EXIT_FAILURE);
    }

    char * filename = argv[1];

    fflush(stdin);
    int read_a, write_a, write_b, write_c, read_b, read_c;
    bool b_done, c_done;
    int n, prev, b_prev, c_prev;
    bool flag, series_b, series_c;
    bool many_series;
    int b_n, c_n, ens;

    int t;

    clock_t start, end;


    start = clock();

    while (true) {
        ens = open(filename, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

        if ( ens < 0 ) {
          perror("open");
          exit(EXIT_FAILURE);
        }

        t = INT_MIN;
        many_series = false;
        while ( read(ens, &n, sizeof(int)) > 0 ) {
            if (n < t) {
                many_series = true;
                break;
            }

          t = n;
        }
        close(ens);

        if ( !many_series ) {
            printf("Done\n");

            end = clock() - start;

            printf("Time taken %lfs", ((double)end) / CLOCKS_PER_SEC);
            exit(EXIT_SUCCESS);
        }

        read_a = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

        if ( read_a < 0 ) {
          perror("open");
          exit(EXIT_FAILURE);
        }

        write_b = open("B", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if ( write_b < 0 ) {
          perror("open");
          exit(EXIT_FAILURE);
        }

        write_c = open("C", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if ( write_c < 0 ) {
          perror("open");
          exit(EXIT_FAILURE);
        }

        prev = INT_MIN;
        flag = true;

        while ( read(read_a, &n, sizeof(int)) > 0 ) {
            if ( n < prev ) {
                flag = !flag;
                prev = INT_MIN;
            }

            if ( flag ) {
                write(write_b, &n, sizeof(int));
            } else {
                write(write_c, &n, sizeof(int));
            }

            prev = n;
        }

        close(write_b);
        close(write_c);
        close(read_a);


        if ( remove( filename ) != 0 ) {
            perror("remove");
            exit(EXIT_FAILURE);
        }

        write_a = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if ( write_a < 0 ) {
            perror("open");
            exit(EXIT_FAILURE);
        }

        read_b = open("B", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if ( read_b < 0 ) {
          perror("open");
          exit(EXIT_FAILURE);
        }

        read_c = open("C", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if ( read_c < 0 ) {
          perror("open");
          exit(EXIT_FAILURE);
        }

        b_done = read(read_b, &b_n, sizeof(int)) < sizeof(int);
        c_done = read(read_c, &c_n, sizeof(int)) < sizeof(int);

        b_prev = INT_MIN;
        c_prev = INT_MIN;
        series_b = true;
        series_c = true;

        while ( !( b_done & c_done ) )
        {
            series_b = b_prev <= b_n;
            series_c = c_prev <= c_n;
            if ( !b_done & ( series_b & ( (b_n <= c_n) | !series_c ) ) | c_done) {
                b_prev = b_n;
                write(write_a, &b_n, sizeof(int));
                b_done = read(read_b, &b_n, sizeof(int)) < sizeof(int);
            } else if ( !c_done & ( series_c & ( (c_n < b_n) | !series_b ) ) | b_done) {
                c_prev = c_n;
                write(write_a, &c_n, sizeof(int));
                c_done = read(read_c, &c_n, sizeof(int)) < sizeof(int);
            } else if ( !series_b & !series_c ) {
                b_prev = INT_MIN;
                c_prev = INT_MIN;
            }
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

