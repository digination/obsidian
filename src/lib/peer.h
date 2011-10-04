#ifndef PEER_H

   #define PEER_H
   #include <openssl/ssl.h>
   #include <stdint.h>

   #define SYNC_NORMAL 0x01
   #define SYNC_NOSYNC 0x02

   typedef struct peer {
      char host[512];
      int port;
      int socknum;
      SSL* ssl;
      uint8_t has_catalog;
      uint8_t sync_mode;
	   
      pthread_t thread;
      int mode;
      char **announce_queue;
      int an_queuesize;

   } peer;

#endif
