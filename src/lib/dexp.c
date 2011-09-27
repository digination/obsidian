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




int sendInfos (int socknum) {

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

   send(socknum,infos,strlen(infos)+1,0) ;
   
     

}


int sendCapa (int socknum) {

   extern dexpd_config conf0;
    
   char *capacity = (char*) malloc ( 4096* sizeof(char) );

   strcpy(capacity,"PROTOCOL:V1\nCRYPTO_LAYER:NONE\n");
  
   send(socknum,capacity,strlen(capacity)+1,0) ;
   
     

}



int sendCatalog(int socknum) {

   extern catalog* cat0;
   extern int nb_cat;

   int i ,j;
   char *cat_header = (char*) malloc (4096* sizeof(char));
   char cat_chunk[521];


   sprintf(cat_header,"CATALOG %d\r\n", (nb_cat * 64 ) + nb_cat );
    
   send(socknum,cat_header,strlen(cat_header)+1,0) ;

   setZero(cat_chunk);
   for (i=0;i<nb_cat;i=i+8) {

      for (j=0;j<8;j++) {

         if ((i+j) < nb_cat) {

           strncat(cat_chunk,cat0[i+j].hash,521 * sizeof(char));
           strcat(cat_chunk,"\n");

         } 
      }

      if(strlen(cat_chunk) > 0) {

          send(socknum,cat_chunk,strlen(cat_chunk),0) ;
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

         send(conf0.peers[i].socknum,io_buffer,strlen(io_buffer),0);

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



int sendDoc(int socknum,char *hash) {

   FILE *fh;
   char file_buffer[1000];
   char send_buffer[4096];
   char file_path[4096];
   long file_length;
   char fl_str[200];
   int i,j;
   int found = 0;
   uint32_t dochead_len;

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
     send(socknum,send_buffer,strlen(send_buffer)+1,0);
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
     send(socknum,send_buffer,strlen(send_buffer),0);
     //send(socknum,file_path,strlen(file_path),0);
     return -2;

   }

   fseek(fh, 0L, SEEK_END);
   file_length = ftell(fh);
   fseek(fh,0L,0);
   sprintf(fl_str,"%ld",file_length);
   
   setZero(send_buffer);
   strncpy(send_buffer,"DOCUMENT ",sizeof(send_buffer));
   strncat(send_buffer,cat0[i].filename,sizeof(send_buffer) - strlen(send_buffer));
   strncat(send_buffer," ",sizeof(send_buffer) - strlen(send_buffer));
   strncat(send_buffer,fl_str,sizeof(send_buffer) - strlen(send_buffer));
   strncat(send_buffer,"\r\n",sizeof(send_buffer) - strlen(send_buffer));


   dochead_len = strlen(send_buffer);

   send(socknum,dochead_len,sizeof(dochead_len),0);
   send(socknum,send_buffer,strlen(send_buffer),0);


   

   //DIRTY HACK TO AVOID PACKET REFRAGMENTATION;
   //sleep(1);

   while( ( i = fread( file_buffer, 1, sizeof( file_buffer ), fh ) ) > 0 ) {

      send(socknum,file_buffer,i,0);

      for (j = 0;j<1000;j++) file_buffer[j] = NUL;


   }

   fclose(fh);
   return 0;
}



int process_announce(int socknum,char*hash) {

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

      fetch_doc(socknum,hash);
      return -1;

   } 

   return 0;


}




int take_action(int socknum,void* io_buffer) {

  char *input = (char*) io_buffer;

  stringlist str0;
  str0 = explode(input,' ');

  //printf("NB_ARGS: %d\n",str0.nb_strings);
    
  if (str0.nb_strings > 0) {
  
    printf("%s",str0.strlist[0]);

    if (strstr( str0.strlist[0] , DEXP_GETINFOS ) == str0.strlist[0] ) {

       sendInfos(socknum);

    }  

    else if (strstr( str0.strlist[0] , DEXP_GETCAPA ) == str0.strlist[0] ) {

       sendCapa(socknum);

    }    

     else if (strstr( str0.strlist[0] , DEXP_GETCATALOG ) == str0.strlist[0] ) {

       sendCatalog(socknum);

    }


    else if (strstr( str0.strlist[0] , DEXP_ANNOUNCE ) == str0.strlist[0] ) {

        if (str0.nb_strings < 2) {

            send(socknum,"300 MISSING ARGUMENT\r\n",22,0);
            return -2;
       } 

       process_announce(socknum,trim(str0.strlist[1]));


    }



     else if (strstr( str0.strlist[0] , DEXP_GETDOCUMENT ) == str0.strlist[0] ) {

       if (str0.nb_strings < 2) {

            send(socknum,"300 MISSING ARGUMENT\r\n",22,0);
            return -2;
       } 

       sendDoc(socknum,trim(str0.strlist[1]));

    }

   
    else if (strstr( str0.strlist[0] , DEXP_PING ) == str0.strlist[0] ) {

       send(socknum,DEXP_PONG,sizeof(DEXP_PONG),0);

    }



  }

}


void *session_thread_serv(void * p_input) {

  peer* current_peer = (peer*) p_input;
  void* io_buffer;

  pthread_detach(pthread_self());

  io_buffer = (void*) malloc(4096*sizeof(char));

  //initialize announce_queue
  current_peer->announce_queue = (char**) malloc(1*sizeof(char*));
  current_peer->an_queuesize = 0;



  while(1) {

     if ( recv(current_peer->socknum,io_buffer,4096*sizeof(char),0) > 0 ) {

        //printf("%s\n",(char*) io_buffer);

        take_action(current_peer->socknum,io_buffer);
        setZero((char*)io_buffer);

     }
   
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




void fetch_doc(int socknum,char* hash) {

   extern dexpd_config conf0;

   FILE* fh;
   int mode = 0;
   int i,k;
   char doc_query[80];
   char io_buffer[4096];   

   int xfr_len;
   int len;
   char file_path[6000];
   char file_dest[6000];

   int file_len;
   uint32_t dochead_len;

   stringlist doc_params;

   strcpy(doc_query,"GET_DOCUMENT ");
   strcat(doc_query,hash);
   strcat(doc_query,"\r\n");
   printf("QUERY: %s",doc_query);

   send(socknum,doc_query,strlen(doc_query),0);
   setZero(io_buffer);


   if ( (len = recv(socknum,dochead_len,sizeof(uint32_t),0)) > 0 ) {

    if ( (len = recv(socknum,io_buffer,dochead_len * sizeof(char),0)) > 0 ) {
      //
      printf("%s\n",io_buffer);

      if (strstr(io_buffer,"DOCUMENT") == io_buffer ) {

         doc_params = explode(io_buffer,' ');

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

         }

         file_len = atoi(doc_params.strlist[2]);
         xfr_len = 0;

         while(xfr_len < file_len) {

            if ( (len = recv(socknum,io_buffer,4096*sizeof(char),0)) > 0 ) {

              fwrite(io_buffer,1,len * sizeof(char),fh);
              xfr_len +=len;

            }

            setZero(io_buffer);

         }


         printf("Notice: file %s fetched succesfully\n",doc_params.strlist[1]);
         fclose(fh);
         rename(file_path,file_dest);
        
     
       }

     }

   }

}











void fetch_docs(int socknum,hash_queue* hq0,int* nb_hq) {

   extern dexpd_config conf0;

   FILE* fh;
   int mode = 0;
   int i,k;
   char doc_query[80];
   char io_buffer[4096];   

   int xfr_len;
   int len;
   char file_path[6000];
   char file_dest[6000];

   int file_len;
   uint32_t dochead_len;


   stringlist doc_params;

   
   for (i=0;i<*nb_hq;i++) {

     strcpy(doc_query,"GET_DOCUMENT ");
     strcat(doc_query,hq0[i].hash);
     strcat(doc_query,"\r\n");

     printf("QUERY: %s",doc_query);

     send(socknum,doc_query,strlen(doc_query),0);
     setZero(io_buffer);

     if ( (len = recv(socknum,dochead_len,sizeof(uint32_t),0)) > 0 ) {
      //

      if ( (len = recv(socknum,io_buffer,dochead_len * sizeof(char),0)) > 0 ) {

      printf("%s\n",io_buffer);

      if (strstr(io_buffer,"DOCUMENT") == io_buffer ) {

         doc_params = explode(io_buffer,' ');

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

         }

         file_len = atoi(doc_params.strlist[2]);
         xfr_len = 0;

         while(xfr_len < file_len) {

            if ( (len = recv(socknum,io_buffer,4096*sizeof(char),0)) > 0 ) {

              fwrite(io_buffer,1,len * sizeof(char),fh);
              xfr_len +=len;

            }

            setZero(io_buffer);

         }


         printf("Notice: file %s fetched succesfully\n",doc_params.strlist[1]);
         fclose(fh);
         rename(file_path,file_dest);
      
        }
  
      }


     }

   }

}




void *session_thread_cli(void * p_input) {

  peer* current_peer = (peer*) p_input;
  char io_buffer[4096];

  int mode = DEXPMODE_IDLE;

  stringlist str0;
  int catalog_size = 0;
  char *catalog_str;
  int len = 0;
  int xfr_size = 0;

  hash_queue *hq0;
  int nb_hq;

  int k;

  pthread_detach(pthread_self());

  hq0 = (hash_queue*) malloc(1 * sizeof(hash_queue));
  nb_hq = 0;

  

  //initialize announce_queue
  current_peer->announce_queue = (char**) malloc(1*sizeof(char*));
  current_peer->an_queuesize = 0;
  
  //initialize peer_mode
  current_peer->mode = DEXPMODE_IDLE;

  send(current_peer->socknum,"GET_CATALOG\r\n",14,0);
  mode = DEXPMODE_WAIT_CATALOG_HEADER;
  //current_peer->mode = DEXPMODE_BUSY;
  
  while(1) {


     if ( (len = recv(current_peer->socknum,io_buffer,4096*sizeof(char),0)) > 0 ) {

         switch(mode) {

            case DEXPMODE_IDLE:
               take_action(current_peer->socknum,io_buffer);
               break;
            case DEXPMODE_WAIT_CATALOG_HEADER:

               str0 = explode(io_buffer,' ');
               if (strstr( str0.strlist[0] , DEXPHEAD_CATALOG ) == str0.strlist[0] ) {
                  
                   if (str0.nb_strings > 1) {
                      catalog_size = atoi(trim(str0.strlist[1]));
                      catalog_str = (char*) malloc(catalog_size * sizeof(char) +1);
                      setZero(catalog_str);
                      mode = DEXPMODE_CATALOG_XFR;
                   }

               }
   
               break;
 
             case DEXPMODE_CATALOG_XFR:

                xfr_size += len; 
                strncat(catalog_str,io_buffer,catalog_size * sizeof(char) - strlen(catalog_str) );
                //catalog xfer is finished
                if (xfr_size >= catalog_size) {

                  hq0 = register_hashes(catalog_str,hq0,&nb_hq);

                  free(catalog_str);

                  if (nb_hq > 0 ) {

                      mode = DEXPMODE_FETCHDOCS;
                      fetch_docs(current_peer->socknum,hq0,&nb_hq);
                  }

                  current_peer->mode = DEXPMODE_IDLE;
                  mode = DEXPMODE_IDLE;
                  printf("MODE IDLE \n");

                }

                break;
             
            default:
               take_action(current_peer->socknum,io_buffer);
               break;           
         }


     }

     setZero((char*)io_buffer);

     
  }

}




void* keepalive_thread() {


  extern dexpd_config conf0;
  int i;
  char io_buffer[7];


  while(1) {

    for(i=0;i<conf0.nb_peers;i++) {


       if (conf0.peers[i].mode != DEXPMODE_BUSY) {

         send(conf0.peers[i].socknum,DEXP_PING,sizeof(DEXP_PING));

      }

     //setsockopt SO_KEEPALIVE ??
     if ( !recv(conf0.peers[i].socknum,io_buffer,sizeof(io_buffer)) ) {

        conf0.peers[i].socknum = -1;
        pthread_kill(conf0.peers[i].thread);
        printf("Peer %s dosconnected\n", conf0.peers[i].host);

     }

    }

  }


  sleep(conf0.keepalive_timeout);


}



