#include <getopt.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#define MAX_SIZE 1024

int tee(const char* file, int mode) {
    int err = 0;
    int flags = mode == O_TRUNC ? O_TRUNC : mode;
    flags |= (O_CREAT | O_WRONLY);

    int permits = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

    int ofd = open(file, flags, permits);
    if (ofd == -1) {
        fprintf(stderr, "open %s error: %s\n", file, strerror(errno));
        return -1;
    }

    char buffer[MAX_SIZE] = {0};
    ssize_t n_read = 0, n_write = 0;
    while ((n_read = read(STDIN_FILENO, buffer, MAX_SIZE)) > 0) {
        n_write = write(STDOUT_FILENO, buffer, n_read);

        if (n_write == -1) {
            fprintf(stderr, "write to stdin error: %s\n", strerror(errno));
            err = -1;
            break;     
        }

        n_write = write(ofd, buffer, n_read);
        
        if (n_write == -1) {
            fprintf(stderr, "write to %s error: %s\n", file, strerror(errno));
            err = -1;
            break;
        }
    }

    if (n_read == -1) {
        fprintf(stderr, "read from stdin error: %s\n", strerror(errno));
        err = -1;        
    }

    if (close(ofd) == -1) {
        fprintf(stderr, "close %s error: %s\n", file, strerror(errno));
        err = -1;
    }

    return err;
}

void usage() {
    printf("usage: tee [-a] file\n");
}

int main(int argc, char** argv) {
    if (argc < 2) {
        usage();
        return -1;
    }

    int ch;
    const char options[] = "a:h";
    char* file = argv[1];
    int mode = O_TRUNC;

    while ((ch = getopt(argc, argv, options)) != -1) {
        switch (ch)
        {
        case 'a':
            file = optarg;
            mode = O_APPEND;
            break;
        
        case 'h':
            usage();
            return 0;

        default:
            usage();
            return -1;
        }
    }

    return tee(file, mode);
}