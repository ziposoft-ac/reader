//
// Created by ac on 11/23/25.
//

#ifndef ZIPOSOFT_GLOBAL_H
#define ZIPOSOFT_GLOBAL_H

#include "pch.h"


extern z_console gConsole;
extern bool g_process_shutting_down ;

void process_quit_notify();
void process_wait_for_quit();
extern const  char* BUILD_TIME_STAMP;


#endif //ZIPOSOFT_CONFIG_H