// This version of stream uses a shared UIO device to execute STREAM. This
// allows multiple independent systems to run a part of stream.

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>
#include <time.h>

#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>


int main() {
    // keep the size of the operation low. 16 pages.
    const int SIZE = 0x10000;

    // print the process id
    printf("Running stream with PID %d\n", getpid());

    // we'll use a uio device to test write and write responses. Replace this
    // with any device you want to test with.
    char* uio_mountpoint = "/dev/uio0";

    int uiofd = open(uio_mountpoint, O_RDWR);
    if (uiofd < 0) {
        printf("Error mounting! Make sure that the mount point %s is valid\n",
                uio_mountpoint);
        return EXIT_FAILURE;
    }

    // Allocate the huge page. the starting physical address will be same as
    // the uio device.
    char *ptr = mmap(
                0x0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, uiofd, 0);

    // Map should not fail if the device is setup correctly.
    if (ptr == MAP_FAILED) {
        perror("Failed to map a 1 GiB page!\n");
        return -1;
    }

    // Mapping worked. Inform the user where the staring address is located.
    volatile char* start_address = (volatile char *) ptr;
    printf("Mapping successful at address %#lx\n", (long) start_address);


    // start the timing debug here
    system("m5 --addr=0x10010000 exit;");

    // fill up the 16 addresses
    for (int i = 0 ; i < SIZE ; i++) {
        *((int *) (start_address + i * sizeof(int))) = i;
    }

    // do not close the file

    return 0;
}
