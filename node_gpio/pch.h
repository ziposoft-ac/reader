//
// Created by ac on 7/30/21.
//

#ifndef ZIPOSOFT_PCH_H
#define ZIPOSOFT_PCH_H
#include <cstdio>
#include <thread>
#include <limits>
#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include <exception>
#include <stdexcept>
#include <iostream>

#ifdef DEBUG
#define Z_TRACE_ENABLE
#endif
#include "zipolib/zipolib.h"
#include "zipolib/z_factory.h"
#include "zipolib/z_console.h"
#include "zipolib/z_safe_queue.h"
#include "zipolib/z_safe_map.h"

#endif //ZIPOSOFT_PCH_H
