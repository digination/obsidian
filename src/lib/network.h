#ifndef NETWORK_H
#define NETWORK_H

#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "crypto.h"
#include <netinet/tcp.h>
#include <signal.h>

#define OBSIDIAN_DEFAULT_PORT 11337
#define KEEPALIVE_NBRETRIES 2


int set_keepalive(int);
int create_socket(char*,int);


#endif


