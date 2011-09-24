#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "utils.h"
#include "peer.h"



#define ADEPD_CONFIG_FILE "/etc/obsidian.conf"
#define MAX_OPT_STR_LEN 4096


typedef struct dexpd_config {
   peer *peers;
   int nb_peers;
   void **filters;

   char data_dir[MAX_OPT_STR_LEN];
   char metadata_dir[MAX_OPT_STR_LEN];
   char tmp_dir[MAX_OPT_STR_LEN];
   char listening_addr[MAX_OPT_STR_LEN];
   int listening_port;

   char node_name[MAX_OPT_STR_LEN];
   char node_descr[MAX_OPT_STR_LEN*2];
   char node_location[255];
   int peer_timeout;

   int log_level;
   int log_stdout;
   char log_file[MAX_OPT_STR_LEN];

   
} dexpd_config;


//definition of global variables to handle config 
//(to avoid passing them by reference each time we need them)
int nb_cat;
dexpd_config conf0;
//

int init_config();

#endif




