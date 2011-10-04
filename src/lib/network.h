#ifndef NETWORK_H
#define NETWORK_H

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "crypto.h"

#define OBSIDIAN_DEFAULT_PORT 11337

int create_socket(char*,int);


#endif


