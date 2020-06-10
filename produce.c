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
    int fd = shm_open(name, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
    if (fd == -1)
        unix_error("open");

    if (ftruncate(fd, len) == -1)
        unix_error("ftruncate");

    /* CreateFileMapping, MapViewOfFile */
    char *addr = mmap(NULL, len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED)
        unix_error("mmap");

    /* write to shared memory */
    sprintf(addr, "Shared memory message");
    msync(addr, len, MS_SYNC);

    munmap(addr, len);
    shm_unlink(name);
    return 0;
}
