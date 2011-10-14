#ifndef COMMON_H
#define COMMON_H

#define STR_BIG_S 4096
#define STR_REG_S 512
#define STR_SMALL_S 255
#define STR_TINY_S 128


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
