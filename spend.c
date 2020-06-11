#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

void unix_error(char const *s)
{
    perror(s);
    exit(1);
}

int main()
{
    const char *name = "temp.txt";
    int len = getpagesize();

    /* CreateFile */
    int fd = shm_open(name, O_RDONLY, 0);
    if (fd == -1)
        unix_error("open");

    /* CreateFileMapping, MapViewOfFile */
    const char *addr = mmap(NULL, len, PROT_READ, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED)
        unix_error("mmap");

    /* write to shared memory */
    printf("Read message %s\n", addr);

    pause();

    munmap((void *)addr, len);
    shm_unlink(name);
    return 0;
}
