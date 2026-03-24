


int open_serial_port(const char *devname)
{
    struct termios opts;

    int fd = open(devname, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0) {
        return CC_LOG_VERB("Could not open modem on port: %s\n", devname);
    }

    tcgetattr(fd, &opts);
    opts.c_cflag |= CREAD | CLOCAL;
    opts.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHONL | ISIG);
    opts.c_iflag |= (IXON | IXOFF | IXANY);
    opts.c_oflag &= ~OPOST;

    cfsetispeed(&opts, B115200);
    cfsetospeed(&opts, B115200);

    if (tcsetattr(fd, TCSANOW, &opts) != 0) {
        CC_LOG_ERROR("failed to configure port: %s\n", devname);
        close(fd);
        return -1;

    }
    fcntl(fd, F_SETFL, O_NONBLOCK);
    return fd;

}

int read_port(int fd)
{

    ret = check_fd( fd, 1 );
    if( ret != 0 )
        return( ret );

    FD_ZERO( &read_fds );
    FD_SET( fd, &read_fds );

    tv.tv_sec  = timeout / 1000;
    tv.tv_usec = ( timeout % 1000 ) * 1000;

    ret = select( fd + 1, &read_fds, NULL, NULL, timeout == 0 ? NULL : &tv );
}