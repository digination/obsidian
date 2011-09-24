#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "config.h"

#define LL_CRITICAL 0x01
#define LL_WARNING 0x02
#define LL_NORMAL 0x03
#define LL_DEBUG 0x04

int logger_init();

int logger(int,char*format,...);


#endif
