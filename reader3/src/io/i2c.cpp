//
// Created by ac on 7/15/26.
//

#include "i2c.h"

extern "C" {
#include <fcntl.h>

#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <byteswap.h>
// Terrible portability hack between arm-linux-gnueabihf-gcc on Mac OS X and native gcc on raspbian.
#ifndef I2C_M_RD
#include <linux/i2c.h>
#endif



}

ZMETA(I2c) {
    ZACT(open);
    ZACT(close);
    ZCMD(read, ZFF_CMD_DEF, "read",
         ZPRM(int, addr, 0, "addr", ZFF_PARAM),
         ZPRM(int, reg, 0, "reg", ZFF_PARAM)
         );
    ZCMD(write, ZFF_CMD_DEF, "write",
         ZPRM(int, addr, 0, "addr", ZFF_PARAM),
         ZPRM(int, reg, 0, "reg", ZFF_PARAM),
         ZPRM(int, data, 0, "data", ZFF_PARAM)
         );
};




// Default RPi B device name for the I2C bus exposed on GPIO2,3 pins (GPIO2=SDA, GPIO3=SCL):
const char *i2c_fname = "/dev/i2c-1";

// Returns a new file descriptor for communicating with the I2C bus:
int i2c_init(void) {
    int i2c_fd = -1;

    if ((i2c_fd = open(i2c_fname, O_RDWR)) < 0) {
        char err[200];
        sprintf(err, "open('%s') in i2c_init", i2c_fname);
        perror(err);
        return -1;
    }

    // NOTE we do not call ioctl with I2C_SLAVE here because we always use the I2C_RDWR ioctl operation to do
    // writes, reads, and combined write-reads. I2C_SLAVE would be used to set the I2C slave address to communicate
    // with. With I2C_RDWR operation, you specify the slave address every time. There is no need to use normal write()
    // or read() syscalls with an I2C device which does not support SMBUS protocol. I2C_RDWR is much better especially
    // for reading device registers which requires a write first before reading the response.

    return i2c_fd;
}

void i2c_close(int i2c_fd) {
    close(i2c_fd);
}

// Write to an I2C slave device's register:
int i2c_write(int i2c_fd,U8 slave_addr, U8 reg,U8 size, U8 *data) {
    int retval;
    U8 outbuf[5];

    struct i2c_msg msgs[1];
    struct i2c_rdwr_ioctl_data msgset[1];

    outbuf[0] = reg;
    memcpy(outbuf+1,data,size);

    msgs[0].addr = slave_addr;
    msgs[0].flags = 0;
    msgs[0].len = size+1;
    msgs[0].buf = outbuf;

    msgset[0].msgs = msgs;
    msgset[0].nmsgs = 1;

    if (ioctl(i2c_fd, I2C_RDWR, &msgset) < 0) {
        perror("ioctl(I2C_RDWR) in i2c_write");
        return -1;
    }

    return 0;
}
// Read the given I2C slave device's register and return the read value in `*result`:
int i2c_write_word(int i2c_fd,U8 slave_addr, U8 reg, U16 data) {
    data=bswap_16(data);

    int ret= i2c_write(i2c_fd,slave_addr,reg,2,(U8*)&data);
    return ret;
}

// Read the given I2C slave device's register and return the read value in `*result`:
int i2c_read(int i2c_fd,U8 slave_addr, U8 reg,U8 size,U8 *result) {
    int retval;
    U8 outbuf[2], inbuf[2];
    struct i2c_msg msgs[2];
    struct i2c_rdwr_ioctl_data msgset[1];

    msgs[0].addr = slave_addr;
    msgs[0].flags = 0;
    msgs[0].len = 1;
    msgs[0].buf = outbuf;

    msgs[1].addr = slave_addr;
    msgs[1].flags = I2C_M_RD | I2C_M_NOSTART;
    msgs[1].len = size;
    msgs[1].buf = (U8*)result;

    msgset[0].msgs = msgs;
    msgset[0].nmsgs = 2;

    outbuf[0] = reg;

    inbuf[0] = 0;

    *result = 0;
    if (ioctl(i2c_fd, I2C_RDWR, &msgset) < 0) {
        perror("ioctl(I2C_RDWR) in i2c_read");
        return -1;
    }

    //*result = inbuf[0];
    return 0;
}
// Read the given I2C slave device's register and return the read value in `*result`:
int i2c_read_word(int i2c_fd,U8 slave_addr, U8 reg, U16 *result) {
    int ret= i2c_read(i2c_fd,slave_addr,reg,2,(U8*)result);
    *result=bswap_16(*result);
    return ret;
}

z_status I2c::open() {
    if (_i2c_fd>0)
        return zs_ok;
    _i2c_fd=i2c_init();
    if (_i2c_fd<1) {
        _i2c_fd=0;
        return zs_access_denied;

    }
    return zs_ok;
}

z_status I2c::close() {
    if (_i2c_fd) {
        i2c_close(_i2c_fd);
        _i2c_fd=0;
    }
    return zs_ok;

}

z_status I2c::read(int addr, int reg) {
    if (open())
        return zs_not_open;
    U16 word;
    if (i2c_read_word(_i2c_fd,addr,reg,&word))
        return zs_io_error;
    printf("word=%04x\n",word);
    return zs_ok;

}

z_status I2c::write(int addr, int reg,int data) {
    if (open())
        return zs_not_open;

    if (i2c_write_word(_i2c_fd,addr,reg,(U16)data))
        return zs_io_error;
    return zs_ok;

}
