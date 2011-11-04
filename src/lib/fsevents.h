#ifndef FSEVENTS_H
#define FSEVENTS_H

#include <sys/types.h>
#include <sys/inotify.h>
#include <sys/select.h>
#include <stdio.h>
#include <openssl/sha.h>
#include "config.h"
#include "common.h"
#include "dexp.h"


#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

void *notify_thread(void*);
char* add_hash(char*);

#endif
