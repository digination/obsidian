#ifndef PEER_H

   #define PEER_H
   #include <openssl/ssl.h>
   #include <stdint.h>

   typedef struct peer {
      char host[512];
      int port;
      int socknum;
      SSL* ssl;
      uint8_t has_catalog;
      pthread_t thread;
      int mode;
      char **announce_queue;
      int an_queuesize;

   } peer;

#endif
