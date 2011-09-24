#ifndef COMMON_H
#define COMMON_H


typedef struct catalog {

   unsigned char hash[65];
   char filename[255];

} catalog;

typedef struct hash_queue {

  char hash[65];
  //char host[255];

} hash_queue;

catalog *cat0;

#endif
