#include "config.h"


int parse_peers(char *peers_str) {

   extern dexpd_config conf0;

   int nb_peers = 0;
   int i = 0;
     
   char **peersarray = NULL;
   stringlist peer_args;

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

	  peer_args = explode(peersarray[i],' ');
      memcpy(conf0.peers[i].host,peer_args.strlist[0],STR_SMALL_S * sizeof(char));

	  if (peer_args.nb_strings > 1) {
         conf0.peers[i].port = atoi(peer_args.strlist[1]);
	  }

      else {

         conf0.peers[i].port = OBSIDIAN_DEFAULT_PORT;
		  
	  }


      conf0.peers[i].sync_mode = SYNC_NORMAL;
	   
      if (peer_args.nb_strings > 2) {
		  
         if ( strcmp(peer_args.strlist[2],"nosync") == 0 ) {

			 //printf("Notice: peer %s is in nosync mode\n",conf0.peers[i].host);
			 conf0.peers[i].sync_mode = SYNC_NOSYNC;
		 }
  
	  }
  
      //we put the SSL handler to NULL
	  conf0.peers[i].ssl = NULL;
	  conf0.peers[i].has_catalog = 0;
	  conf0.peers[i].pub = 0;
		  
     //free(hostandport);
      
   }

  
   //on libere l'espace
   for (i=0;i<nb_peers;i++) {
      free(peersarray[i]);
   }
   free(peersarray);

   //debug
   //for (i=0;i<nb_peers;i++) printf("%s:%d\n",conf0.peers[i].host,conf0.peers[i].port);

   return nb_peers;

}




int init_config() {

   extern dexpd_config conf0;

   FILE *fh;
   char buff[STR_BIG_S];
   char opt_name[STR_BIG_S];
   char opt_value[STR_BIG_S];

   char * eq_pos ;
   int line = 0;
   filter* f0;

   char dexp_config_path[STR_BIG_S];
   setZeroN(dexp_config_path,STR_BIG_S);

   #ifdef OBSIDIAN_PREFIX

      strncpy(dexp_config_path,
              OBSIDIAN_PREFIX,
              STR_BIG_S*sizeof(char));

   #endif

     strncat(dexp_config_path,
             DEXPD_CONFIG_FILE,
             STR_BIG_S*sizeof(char) - strlen(dexp_config_path));

   fh = fopen(dexp_config_path,"r");

   //init some default config variables
   conf0.use_tls= 0;
   conf0.pub = 0;
   
   //init ruleset
   conf0.rs.rules = (filter*) malloc(sizeof(filter));
   conf0.rs.nb_rules = 0;

   if (! fh ) { 

      fprintf(stderr,"CANNOT OPEN CONFIG FILE\n");
      exit(1);
   }

   while (! feof(fh) ) {

     fgets(buff,STR_BIG_S * sizeof(char) ,fh);
     line++;
     
     //We parse Filter rules
     if ( strstr(buff,"filter") ==  buff) {

        f0 = parse_filter(line,buff);
        
        if (f0 != NULL) {
           memcpy( &(conf0.rs.rules[conf0.rs.nb_rules]),f0,sizeof(filter));
           conf0.rs.nb_rules++;
           conf0.rs.rules = (filter*) realloc(conf0.rs.rules,(conf0.rs.nb_rules+1) * sizeof(filter));
        }
        
     }

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

           memcpy(conf0.node_location,opt_value,STR_SMALL_S * sizeof(char));           
        }

        else if (strcmp(opt_name,"use_tls") == 0 ) {
           if (strcmp(opt_value,"true") == 0 ) conf0.use_tls = 1;
         }


        else if (strcmp(opt_name,"public_node") == 0 ) {
           if (strcmp(opt_value,"true") == 0 ) conf0.pub = 1;
         }

        else if (strcmp(opt_name,"tls_server_cert") == 0 ) {

           memcpy(conf0.tls_server_cert,opt_value,MAX_OPT_STR_LEN * sizeof(char));           
        }

         /*
         else if (strcmp(opt_name,"tls_cli_cert") == 0 ) {

           memcpy(conf0.tls_cli_cert,opt_value,MAX_OPT_STR_LEN * sizeof(char));           
        }
        */

         else if (strcmp(opt_name,"tls_server_ca") == 0 ) {

           memcpy(conf0.tls_server_ca,opt_value,MAX_OPT_STR_LEN * sizeof(char));           
        }

        else if (strcmp(opt_name,"tls_server_dh") == 0 ) {

           memcpy(conf0.tls_server_dh,opt_value,MAX_OPT_STR_LEN * sizeof(char));           
        }




         else if (strcmp(opt_name,"use_ipv6") == 0 ) {
            if (strcmp(opt_value,"true") == 0 ) conf0.use_ipv6 = 1;
        }

        else if (strcmp(opt_name,"ipv6_listening_addr") == 0) {
           memcpy(conf0.ipv6_listening_addr,opt_value,MAX_OPT_STR_LEN * sizeof(char));
         }

            
     }

   } 

   return 0;

}
