//
// Created by ac on 7/15/26.
//

#ifndef ZIPOSOFT_I2C_H
#define ZIPOSOFT_I2C_H

#include "pch.h"

int i2c_init(void);
void i2c_close(int i2c_fd);
int i2c_write_word(int i2c_fd,U8 slave_addr, U8 reg, U16 data);
int i2c_read_word(int i2c_fd,U8 slave_addr, U8 reg, U16 *result);

class I2c {
    // Global file descriptor used to talk to the I2C bus:
    int _i2c_fd =0;
public:
    z_status open();
    z_status close();
    z_status read(int addr,int reg);
    z_status write(int addr,int reg,int data);


};


#endif //ZIPOSOFT_I2C_H
