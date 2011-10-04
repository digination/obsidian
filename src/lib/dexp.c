#include "dexp.h"

/*
int has_document(char *hash ,char **hash_array,int nb_hash) {

   int i = 0;

   for (i=0;i<nb_hash;i++) {

       if strcmp(hash,hash_array[i]) == 0 {
          return 1;
       }
   } 
   return 0;
}



int parse_cmd(char* rcv,char **hash_array,int nb_hash) {

  if (strstr(rcv,GET_METADATA) == rcv ) {

    
     if (has_document() )    

  }

}
*/




int sendInfos (peer* cpeer) {

   extern dexpd_config conf0;
    
   char *infos = (char*) malloc ( (strlen(conf0.node_name) + strlen(conf0.node_descr) + strlen(conf0.node_location) + 512 ) * sizeof(char) );

   strcpy(infos,"Node Name: ");
   strcat(infos,conf0.node_name);
   strcat(infos,"\n");
   strcat(infos,"Node Descr: ");
   strcat(infos,conf0.node_descr);
   strcat(infos,"\n");
   strcat(infos,"Node_Location: ");
   strcat(infos,conf0.node_location);

   dexp_send(cpeer,infos,strlen(infos)) ;
   
     

}


int sendCapa (peer *cpeer) {

   extern dexpd_config conf0;
    
   char *capacity = (char*) malloc ( 4096* sizeof(char) );

   strcpy(capacity,"PROTOCOL:V1\nCRYPTO_LAYER:NONE\n");
  
   dexp_send(cpeer,capacity,strlen(capacity)) ;
   
     

}



int sendCatalog(peer *cpeer) {

   extern catalog* cat0;
   extern int nb_cat;

   int i ,j;
   char *cat_header = (char*) malloc (4096* sizeof(char));
   char cat_chunk[521];


   sprintf(cat_header,"CATALOG %d\r\n", (nb_cat * 64 ) + nb_cat );    
   dexp_send(cpeer,cat_header,strlen(cat_header)) ;
	
   setZero(cat_chunk);
   for (i=0;i<nb_cat;i=i+8) {

      for (j=0;j<8;j++) {

         if ((i+j) < nb_cat) {

           strncat(cat_chunk,cat0[i+j].hash,521 * sizeof(char));
           strcat(cat_chunk,"\n");

         } 
      }

      if(strlen(cat_chunk) > 0) {

          dexp_send(cpeer,cat_chunk,strlen(cat_chunk)) ;
          setZero(cat_chunk);
      }


   }   

}



int announce(char *hash) {

   extern dexpd_config conf0;
   int i;   
   char io_buffer[4096];

   int qpos;

   strcpy(io_buffer,"ANNOUNCE ");
   strncat(io_buffer,hash,4096*sizeof(char) - strlen(io_buffer) );
   strncat(io_buffer,"\r\n",4096*sizeof(char) - strlen(io_buffer) );

   for (i=0;i<conf0.nb_peers;i++) {
     
      if (conf0.peers[i].socknum > 0 && conf0.peers[i].mode != DEXPMODE_BUSY ) {

         dexp_send(conf0.peers[i],io_buffer,strlen(io_buffer));

      }

      else if ( conf0.peers[i].mode == DEXPMODE_BUSY) {

         printf("Notice: Peer connection is busy, queuing Announce %s\n",hash);
      
         qpos = conf0.peers[i].an_queuesize;
         conf0.peers[i].announce_queue[qpos] = (char*) malloc(65*sizeof(char));
         strncpy(conf0.peers[i].announce_queue[qpos],hash,64*sizeof(char));
         conf0.peers[i].an_queuesize++;
         conf0.peers[i].announce_queue = (char**) realloc(conf0.peers[i].announce_queue,(conf0.peers[i].an_queuesize +1)*sizeof(char*));
         
         
             
                 
      }


   }

   setZeroN(io_buffer,4096);

}



int sendDoc(peer* cpeer,char *hash) {

   FILE *fh;
   char file_buffer[1000];
   char send_buffer[4096];
   char file_path[4096];
   long file_length;
   char fl_str[200];
   int i,j;
   int found = 0;

   extern catalog* cat0;
   extern dexpd_config conf0;
   extern int nb_cat;

   for (i=0;i<nb_cat;i++) {

     if (strcmp(hash,cat0[i].hash) == 0) {

       found = 1;
       break;

     }

   }
     
   if (!found) {

     strcat(send_buffer,"400 FILE NOT FOUND\r\n");
     dexp_send(cpeer,send_buffer,strlen(send_buffer)+1);
     return -1; 

   }
   
   
   strncat(file_path,conf0.data_dir,4096*sizeof(char));
   strncat(file_path,"/",4096*sizeof(char) - strlen(file_path));
   strncat(file_path,cat0[i].filename,4096*sizeof(char) - strlen(file_path));

   printf("%s\n",file_path);

   fh = fopen(file_path,"rb");
   setZero(file_path);

   if (!fh) {

     strcat(send_buffer,"401 CANNOT OPEN FILE FOR READING\r\n");
     dexp_send(cpeer,send_buffer,strlen(send_buffer));
     //dexp_send(cpeer,file_path,strlen(file_path));
     return -2;


   }

   fseek(fh, 0L, SEEK_END);
   file_length = ftell(fh);
   fseek(fh,0L,0);
   sprintf(fl_str,"%ld",file_length);
   
   setZero(send_buffer);
   strncpy(send_buffer,"DOCUMENT:",sizeof(send_buffer));
   strncat(send_buffer,cat0[i].filename,sizeof(send_buffer) - strlen(send_buffer));
   strncat(send_buffer,":",sizeof(send_buffer) - strlen(send_buffer));
   strncat(send_buffer,fl_str,sizeof(send_buffer) - strlen(send_buffer));
   strncat(send_buffer,"\r\n",sizeof(send_buffer) - strlen(send_buffer));

   dexp_send(cpeer,send_buffer,strlen(send_buffer));


   //DIRTY HACK TO AVOID PACKET REFRAGMENTATION;
   //sleep(1);

   while( ( i = fread( file_buffer, 1, sizeof( file_buffer ), fh ) ) > 0 ) {

      dexp_send(cpeer,file_buffer,i);

      for (j = 0;j<1000;j++) file_buffer[j] = NUL;


   }

   fclose(fh);
   return 0;
   
}



int process_announce(peer *cpeer,char*hash) {

   extern catalog* cat0;
   extern int nb_cat;
   int i = 0;
   int found = 0;


   for (i=0;i<nb_cat;i++) {

      if (strcmp(cat0[i].hash,hash) == 0  ) {

         found = 1;
         break;

      }

   }
   
   if (! found ) {

      fetch_doc(cpeer,hash);
      return -1;

   } 

   return 0;


}




int take_action(int socknum,peer* cpeer,void* io_buffer) {

  char *input = (char*) io_buffer;

  stringlist str0;
  str0 = explode(input,' ');

  //printf("NB_ARGS: %d\n",str0.nb_strings);
    
  if (str0.nb_strings > 0) {
  
    printf("%s",str0.strlist[0]);

    if (strstr( str0.strlist[0] , DEXP_GETINFOS ) == str0.strlist[0] ) {

       sendInfos(cpeer);

    }  

    else if (strstr( str0.strlist[0] , DEXP_GETCAPA ) == str0.strlist[0] ) {

       sendCapa(cpeer);

    }    

     else if (strstr( str0.strlist[0] , DEXP_GETCATALOG ) == str0.strlist[0] ) {

       sendCatalog(cpeer);

    }


    else if (strstr( str0.strlist[0] , DEXP_ANNOUNCE ) == str0.strlist[0] ) {

        if (str0.nb_strings < 2) {

            dexp_send(cpeer,"300 MISSING ARGUMENT\r\n",22);
            return -2;
       } 

       process_announce(cpeer,trim(str0.strlist[1]));


    }



     else if (strstr( str0.strlist[0] , DEXP_GETDOCUMENT ) == str0.strlist[0] ) {

       if (str0.nb_strings < 2) {

            dexp_send(cpeer,"300 MISSING ARGUMENT\r\n",22);
            return -2;
       } 

       sendDoc(cpeer,trim(str0.strlist[1]));

    }

   
    else if (strstr( str0.strlist[0] , DEXP_PING ) == str0.strlist[0] ) {

       dexp_send(cpeer,DEXP_PONG,sizeof(DEXP_PONG),0);

    }


    else if (strstr( str0.strlist[0] , DEXP_STARTTLS ) == str0.strlist[0] ) {

       printf("Starting TLS communication...\n");
       cpeer->ssl = (SSL*) start_tls(socknum);

    }



  }

}



char* receive_catalog(peer* cpeer) {

  char io_buffer[4096];
  char cat_part[4096];
  int len;
  char *head_end_ptr;
  int header_len;
  int catpart_len;
  int cat_len;
  stringlist cat_params;
  char* catalog_str;
  int xfr_size = 0;
  int i;


  if ( (len = dexp_recv(cpeer,io_buffer,4096*sizeof(char))) > 0 ) {

        if (strstr(io_buffer,"CATALOG") == io_buffer ) {

          head_end_ptr = strstr(io_buffer,"\r\n");
          if (head_end_ptr != NULL) {
            header_len = (head_end_ptr - io_buffer) + 2;
            catpart_len = len - header_len;
          }

         printf("HEADER_LEN:%d | CAT_LEN:%d\n",header_len,cat_len);

         if (catpart_len > 0) {
            memcpy(cat_part,head_end_ptr+2,sizeof(char) * catpart_len);    
            for(i=header_len;i<4096;i++) {
              io_buffer[i] = '\0';
            }            
         }
 

         printf("IO_BUFFER: %s\n",io_buffer);
         cat_params = explode(io_buffer,' ');

         if (cat_params.nb_strings > 1) {
            cat_len = atoi(cat_params.strlist[1]);
            catalog_str = (char*) malloc(cat_len * sizeof(char) +1);
            setZero(catalog_str);  
         }         

         else {
            return NULL;
         }

         if (catpart_len > 0) {
           strncpy(catalog_str,cat_part,cat_len * sizeof(char));
         }
  
         xfr_size += catpart_len;

         while(xfr_size < cat_len) {

            setZeroN(io_buffer,4096);
            len = dexp_recv(cpeer,io_buffer,4096*sizeof(char));
            strncat(catalog_str,io_buffer,cat_len*sizeof(char) - strlen(catalog_str));         
            xfr_size += len;

         }  

  
      }
       
  }


  return catalog_str;


}



void *session_thread_serv(void * p_input) {

  peer* current_peer = (peer*) p_input;
  void* io_buffer;
  int i;
  pthread_detach(pthread_self());

  io_buffer = (void*) malloc(4096*sizeof(char));

  //initialize announce_queue
  current_peer->announce_queue = (char**) malloc(1*sizeof(char*));
  current_peer->an_queuesize = 0;



  while(1) {

     if ( dexp_recv(current_peer,io_buffer,4096*sizeof(char)) > 0 ) {

        //printf("%s\n",(char*) io_buffer);

        take_action(current_peer->socknum,current_peer,io_buffer);
        setZero((char*)io_buffer);

     }


	 /*
     if (current_peer->mode != DEXPMODE_BUSY && ! current_peer->has_catalog) {

        printf ("GETCAT!\n");

        current_peer->mode = DEXPMODE_BUSY;
        dexp_send(current_peer,DEXP_GETCATALOG,sizeof(DEXP_GETCATALOG));
        receive_catalog(current_peer);
        current_peer->mode = DEXPMODE_IDLE;

     }
	 */


  }




}



hash_queue* register_hashes(char *cat_str,hash_queue* hq0,int* nb_hq) {

   extern catalog* cat0;
   extern int nb_cat;

   int i,j,k;
   int found =0;
   stringlist rcvhashes;

   int nb_newhashes = 0;

   rcvhashes = explode(cat_str,'\n');
   
   for (i=0;i<rcvhashes.nb_strings;i++) {

      found = 0;

      for(j=0;j<nb_cat;j++) {

          if ( strcmp( cat0[j].hash, rcvhashes.strlist[i]) == 0 ) {

             found = 1;
             //printf("%d FOUND HASH: %s\n",i,rcvhashes.strlist[i]);
             break;

          }

      }


      if (!found) {

         //printf("ADDING HASH %s TO QUEUE\n",rcvhashes.strlist[i]);

         strncpy(hq0[*nb_hq].hash,(char*) rcvhashes.strlist[i],sizeof(hq0[*nb_hq].hash) );

         *nb_hq = *nb_hq + 1;
         nb_newhashes+=1;

         hq0 = (hash_queue*) realloc(hq0, (*nb_hq+1) * sizeof(hash_queue)) ;         

      }


   }

  
   //free all malloced data
   for (i=0;i<rcvhashes.nb_strings;i++) free(rcvhashes.strlist[i]);
   free(rcvhashes.strlist);

   if (nb_newhashes > 0) printf("Notice: %d new files added to dl queue\n",nb_newhashes);

   return hq0;

}                 




int fetch_doc(peer *cpeer,char* hash) {

   extern dexpd_config conf0;

   FILE* fh;
   int mode = 0;
   char doc_query[80];
   char io_buffer[4096];   
   void* file_part;
   char *head_end_ptr;

   int xfr_len;
   int len;
   int i;
   char file_path[6000];
   char file_dest[6000];

   int file_len;
   int header_len = 0;
   int fpart_len = 0;

   stringlist doc_params;

   strcpy(doc_query,"GET_DOCUMENT ");
   strcat(doc_query,hash);
   strcat(doc_query,"\r\n");
   //printf("QUERY: %s",doc_query);

   dexp_send(cpeer,doc_query,strlen(doc_query));
   setZeroN(io_buffer,4096);

   file_part = (void*) malloc(4096*sizeof(char));
   setZeroN((char*)file_part,4096);

   
   if ( (len = dexp_recv(cpeer,io_buffer,4096 * sizeof(char))) > 0 ) {
     

      if (strstr(io_buffer,"DOCUMENT") == io_buffer ) {

          head_end_ptr = strstr(io_buffer,"\r\n");
          if (head_end_ptr != NULL) {
            header_len = (head_end_ptr - io_buffer) + 2;
            fpart_len = len - header_len;
          }

         printf("HEADER_LEN:%d | FPART_LEN:%d\n",header_len,fpart_len);

         if (fpart_len> 0) {
            memcpy(file_part,head_end_ptr+2,sizeof(char) * fpart_len);    
            for(i=header_len;i<4096;i++) {
              io_buffer[i] = '\0';
            }            

         }

         printf("IO_BUFFER: %s\n",io_buffer);
         

         doc_params = explode(io_buffer,':');

         setZero(file_path);
         strncpy(file_path,conf0.tmp_dir,sizeof(file_path));
         strncat(file_path,"/",sizeof(file_path) - strlen(file_path));
         strncat(file_path,doc_params.strlist[1],sizeof(file_path) - strlen(file_path));        

   
         setZero(file_dest);
         strncpy(file_dest,conf0.data_dir,sizeof(file_dest));
         strncat(file_dest,"/",sizeof(file_dest) - strlen(file_dest));
         strncat(file_dest,doc_params.strlist[1],sizeof(file_dest) - strlen(file_dest));

         fh = fopen(file_path,"wb");

         if (!fh) {

            fprintf(stderr,"ERROR: CANNOT OPEN %s FOR WRITING\n",file_path);
            return -1;

         }

         file_len = atoi(doc_params.strlist[2]);
         xfr_len = 0;

         if (fpart_len > 0) {

             fwrite(file_part,1,fpart_len * sizeof(char),fh);
             xfr_len = fpart_len;

         }

         setZeroN(io_buffer,4096);
         while(xfr_len < file_len) {


            if ( (len = dexp_recv(cpeer,io_buffer,4096*sizeof(char))) > 0 ) {

              fwrite(io_buffer,1,len * sizeof(char),fh);
              xfr_len +=len;

            }

            setZeroN(io_buffer,4096);

         }


         printf("Notice: file %s fetched succesfully\n",doc_params.strlist[1]);
         fclose(fh);
         rename(file_path,file_dest);
        
     
       }

    
   }

}


void fetch_docs(peer *cpeer,hash_queue* hq0,int* nb_hq) {

   int i;
   
   for (i=0;i<*nb_hq;i++) {

      fetch_doc(cpeer,hq0[i].hash);
   }

}


void *session_thread_cli(void * p_input) {

  peer* current_peer = (peer*) p_input;
  char io_buffer[4096];

  stringlist str0;
  int catalog_size = 0;
  char *catalog_str;
  int len = 0;
  int xfr_size = 0;

  hash_queue *hq0;
  int nb_hq;

  int k;
  int mode ;

  pthread_detach(pthread_self());

  hq0 = (hash_queue*) malloc(1 * sizeof(hash_queue));
  nb_hq = 0;

  

  //initialize announce_queue
  current_peer->announce_queue = (char**) malloc(1*sizeof(char*));
  current_peer->an_queuesize = 0;
  
  if (conf0.use_tls) {
      send(current_peer->socknum,DEXP_STARTTLS,sizeof(DEXP_STARTTLS),0);
      current_peer->ssl =  (SSL*) start_tls_cli(current_peer->socknum);
   }

  if (current_peer->sync_mode == SYNC_NORMAL) {
     dexp_send(current_peer,"GET_CATALOG\r\n",14);
     catalog_str = receive_catalog(current_peer);
  }

  else {

    printf("skipping initial synchronization with peer %s...\n",current_peer->host);
	  
  }
	  
  if (catalog_str != NULL) {

     hq0 = register_hashes(catalog_str,hq0,&nb_hq);
     free(catalog_str);

        if (nb_hq > 0 ) {

            fetch_docs(current_peer,hq0,&nb_hq);
        }
        current_peer->has_catalog = 1;

  }

  printf("MODE IDLE \n");

  mode = DEXPMODE_IDLE;

  while(1) {


     if ( (len = dexp_recv(current_peer,io_buffer,4096*sizeof(char))) > 0 ) {

         switch(mode) {

            case DEXPMODE_IDLE:
               take_action(current_peer->socknum,current_peer,io_buffer);
               break;
             
            default:
               take_action(current_peer->socknum,current_peer,io_buffer);
               break;           
         }


     }

     setZeroN(io_buffer,4096);
     
  }

}




void* keepalive_thread() {


  extern dexpd_config conf0;
  int i;
  char io_buffer[7];


  while(1) {

    for(i=0;i<conf0.nb_peers;i++) {


       if (conf0.peers[i].mode != DEXPMODE_BUSY) {

         dexp_send(conf0.peers[i],DEXP_PING,sizeof(DEXP_PING));

      }

     //setsockopt SO_KEEPALIVE ??
     if ( !recv(conf0.peers[i].socknum,io_buffer,sizeof(io_buffer),0) ) {

        conf0.peers[i].socknum = -1;
        pthread_kill(conf0.peers[i].thread);
        printf("Peer %s dosconnected\n", conf0.peers[i].host);

     }

    }

  }


  sleep(conf0.keepalive_timeout);


}



