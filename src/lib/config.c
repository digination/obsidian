#include "config.h"


int parse_peers(char *peers_str) {

   extern dexpd_config conf0;

   int nb_peers = 0;
   int i = 0;
     
   char **peersarray = NULL;
   char **hostandport = NULL;

   if (strlen(peers_str) >= 1 ) nb_peers=1;

   //count peers
   for (i=0;i<strlen(peers_str);i++) {

      if( peers_str[i] == ';'  ) nb_peers++;

   }

   conf0.peers = (peer*) malloc (nb_peers * sizeof(peer) +1);
   if ( (peersarray = explode(peers_str,';').strlist) == NULL ) {

     fprintf(stderr,"ERROR: CANNOT PARSE PEERS STRING");
     exit(1);

   }

   for (i=0;i<nb_peers;i++) {

      if ((hostandport = explode(peersarray[i],':').strlist)  != NULL) {
         memcpy(conf0.peers[i].host,hostandport[0],255 * sizeof(char));
         conf0.peers[i].port = atoi(hostandport[1]);
         free(hostandport);
      }

  
   }

  
   //on libere l'espace
   for (i=0;i<nb_peers;i++) {
      free(peersarray[i]);
   }
   free(peersarray);

   //debug
   for (i=0;i<nb_peers;i++) printf("%s:%d\n",conf0.peers[i].host,conf0.peers[i].port);

   return nb_peers;

}




int init_config() {

   extern dexpd_config conf0;

   FILE *fh;
   char buff[4096];
   char opt_name[4096];
   char opt_value[4096];

   char * eq_pos ;

   fh = fopen(ADEPD_CONFIG_FILE,"r");

   if (! fh ) { 

      fprintf(stderr,"CANNOT OPEN CONFIG FILE\n");
      exit(1);
   }

   while (! feof(fh) ) {

     fgets(buff,4096 * sizeof(char) ,fh);

     //we parse the option/value tuple in the config file.
     if ( strstr(buff,"=")  && strstr(buff,"#") != buff ) {

         eq_pos = strstr(buff,"=");
         eq_pos++;

         memcpy(opt_value,trim(eq_pos),MAX_OPT_STR_LEN * sizeof(char));
         sscanf(buff,"%s =",opt_name);

         if (strcmp(opt_name,"listening_addr") == 0 ) {
           memcpy(conf0.listening_addr,opt_value,MAX_OPT_STR_LEN * sizeof(char));
         }


         else if (strcmp(opt_name,"listening_port") == 0 ) {
            conf0.listening_port = atoi(opt_value);
         }


        else if (strcmp(opt_name,"data_dir") == 0 ) {

           memcpy(conf0.data_dir,opt_value,MAX_OPT_STR_LEN * sizeof(char));
           memcpy(conf0.metadata_dir,opt_value,MAX_OPT_STR_LEN * sizeof(char));
           strncat(conf0.metadata_dir,"/metadata",MAX_OPT_STR_LEN * sizeof(char));          
        }


        else if (strcmp(opt_name,"tmp_dir") == 0 ) {
           memcpy(conf0.tmp_dir,opt_value,MAX_OPT_STR_LEN * sizeof(char));
        }


        else if (strcmp(opt_name,"peers") == 0) {
          conf0.nb_peers = parse_peers(opt_value);

        }

        else if (strcmp(opt_name,"keepalive_timeout") == 0) {
          conf0.keepalive_timeout = atoi(opt_value);
        }


        else if (strcmp(opt_name,"node_name") == 0 ) {
           
           memcpy(conf0.node_name,opt_value,MAX_OPT_STR_LEN * sizeof(char));
        }


         else if (strcmp(opt_name,"node_descr") == 0 ) {

           memcpy(conf0.node_descr,opt_value,MAX_OPT_STR_LEN * sizeof(char));           
        }

        else if (strcmp(opt_name,"node_location") == 0 ) {

           memcpy(conf0.node_location,opt_value,255 * sizeof(char));           
        }

             
     }

   } 

   return 0;

}
