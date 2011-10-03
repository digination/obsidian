#include "lib/config.h"
#include "lib/fsevents.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <openssl/sha.h>
#include "lib/common.h"
#include "lib/dexp.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

int connectAll();
int try_reconnect();
int load_catalog(char*);
int isPeer(char*,int);
void* reconnect_loop();
void* listen_v6();



