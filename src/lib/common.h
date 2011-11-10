#ifndef COMMON_H
#define COMMON_H

#define STR_BIG_S 4096
#define STR_REG_S 512
#define STR_SMALL_S 255
#define STR_TINY_S 128

#define EPOLL_TIMEOUT 100
#define MAX_EPOLL_EVENTS_PER_RUN 1
#define EPOLL_QUEUE_LEN 10
#define EPOLL_MAX_EVT 10


typedef struct catalog {

   unsigned char hash[65];
   char filename[STR_SMALL_S];

} catalog;

typedef struct hash_queue {

  char hash[65];
  //char host[STR_SMALL_S];

} hash_queue;

catalog *cat0;

#endif
