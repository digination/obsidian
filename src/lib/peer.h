#ifndef PEER_H

   #define PEER_H
   #include <openssl/ssl.h>
   #include <stdint.h>
   #include "common.h"

   #define SYNC_NORMAL 0x01
   #define SYNC_NOSYNC 0x02



   typedef struct dexp_queue {

       int dexp_cmd;
       char arg1[STR_REG_S];
       char arg2[STR_REG_S];
       char *data;
       int data_len;
       
   } dexp_queue;


   typedef struct peer_capacity {
   
     char proto[5];
     uint8_t has_tls;

   } peer_capacity;


   typedef struct peer {
      char host[STR_REG_S];
      int port;
      int socknum;
      SSL* ssl;
      uint8_t has_catalog;
      uint8_t sync_mode;
      pthread_t ioth;
      pthread_t worker;
      dexp_queue *in_queue;
      dexp_queue *out_queue;
      int in_queue_n;
      int out_queue_n;
      char **announce_queue;
      int an_queuesize;
      peer_capacity capacity;
      uint8_t pub;

   } peer;

#endif
