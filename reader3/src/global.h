//
// Created by ac on 11/23/25.
//

#ifndef ZIPOSOFT_GLOBAL_H
#define ZIPOSOFT_GLOBAL_H

#include "pch.h"


extern z_void_obj* gRootObject;
#define SET_ROOT_OBJ(_X_) z_void_obj* gRootObject=(z_void_obj*)&_X_;


void process_quit_notify();


#endif //ZIPOSOFT_CONFIG_H