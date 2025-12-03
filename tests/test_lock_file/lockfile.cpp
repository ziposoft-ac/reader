//
// Created by ac on 11/30/25.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <errno.h>
#include "zipolib/z_string.h"


class LockFile
{
    int _fd=0;
    z_string _filename;
public:
    LockFile(const char *filename="lockfile.tmp")
    {
        _filename=filename;
    }
    ~LockFile()
    {
        unlock();
    }
    bool lock()
    {
        _fd = open(_filename, O_RDWR | O_CREAT, 0666);
        if (_fd == -1) {
            //perror("open");
            _fd=0;
            return false;
        }
        // Attempt to acquire an exclusive lock (LOCK_EX).
        // LOCK_NB makes it non-blocking; if the lock is held, it will return EWOULDBLOCK.
        // Without LOCK_NB, flock() would block until the lock can be acquired.
        if (flock(_fd, LOCK_EX | LOCK_NB) == -1) {
            if (errno == EWOULDBLOCK) {
                //printf("Failed to acquire lock: Another process holds the lock.\n");
            } else {
            }
            close(_fd);
            _fd=0;
            return false;

        }
        return true;
    }
    bool unlock()
    {
        if (_fd)
        {
            if (flock(_fd, LOCK_UN) == -1) {
                //perror("flock (unlock)");
            }
            close(_fd); // Close the file descriptor, which also releases the lock if not explicitly done
            _fd=0;
        }
        return true;
    }
};
int main(){
    LockFile  lf;
    if (!lf.lock())
    {
            printf("Failed to acquire lock: Another process holds the lock.\n");
        return 1;
    }
    sleep(5);
};

#define LOCK_FILE "my_lock_file.lock"

int main1() {
    int fd;

    // Open the file to be locked. O_CREAT will create it if it doesn't exist.
    // O_RDWR is chosen here to allow both reading and writing if needed,
    // though for flock() the mode doesn't strictly matter for the lock itself.
    fd = open(LOCK_FILE, O_RDWR | O_CREAT, 0666);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    printf("Attempting to acquire exclusive lock on %s...\n", LOCK_FILE);

    // Attempt to acquire an exclusive lock (LOCK_EX).
    // LOCK_NB makes it non-blocking; if the lock is held, it will return EWOULDBLOCK.
    // Without LOCK_NB, flock() would block until the lock can be acquired.
    if (flock(fd, LOCK_EX | LOCK_NB) == -1) {
        if (errno == EWOULDBLOCK) {
            printf("Failed to acquire lock: Another process holds the lock.\n");
            close(fd);
            exit(EXIT_SUCCESS); // Exit gracefully as another process is running
        } else {
            perror("flock");
            close(fd);
            exit(EXIT_FAILURE);
        }
    }

    printf("Exclusive lock acquired. Performing critical section...\n");

    // Simulate work in the critical section
    sleep(5);

    printf("Critical section complete. Releasing lock...\n");

    // Release the lock (LOCK_UN)
    if (flock(fd, LOCK_UN) == -1) {
        perror("flock (unlock)");
        close(fd);
        exit(EXIT_FAILURE);
    }

    printf("Lock released. Exiting.\n");

    close(fd); // Close the file descriptor, which also releases the lock if not explicitly done
    return 0;
}