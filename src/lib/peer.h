#ifndef PEER_H
#define PEER_H

   typedef struct peer {
      char host[512];
      int port;
      int socknum;
      pthread_t thread;
      int mode;
      char **announce_queue;
      int an_queuesize;

   } peer;

#endif
