#ifndef PEER_H

   #define PEER_H
   #include <openssl/ssl.h>

   typedef struct peer {
      char host[512];
      int port;
      int socknum;
      SSL* ssl;
      pthread_t thread;
      int mode;
      char **announce_queue;
      int an_queuesize;

   } peer;

#endif
