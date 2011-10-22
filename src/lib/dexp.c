#include "dexp.h"

//informations to handle multiple versions/retro-compat
#define DEXP_VERSIONS_TAG "V1"
const char* supported_proto[] = {"V1"};
int nb_proto = 1;


int parse_capacity(peer* cpeer,char* io_buffer) {

   stringlist str0;
   stringlist str1;
   int i,j,k;
   char *fv;
   int found_proto =0;

   extern const char* supported_proto[];
   extern int nb_proto;
 
   str0  = explode(io_buffer,'\n');

   for (i=0;i<str0.nb_strings;i++) {

     fv = str0.strlist[i];

     //parsing of version
     if (strstr(fv,"PROTOCOL_VER:") == fv ) {
     
       fv += (13 * sizeof(char));
       str1 = explode(fv,',');

       for (j=0;j<str1.nb_strings;j++) {

          for (k=nb_proto-1;k>=0;k--) {

            if (strcmp(str1.strlist[j],supported_proto[k]) == 0) {

               strncpy(cpeer->capacity.proto,supported_proto[k],4*sizeof(char));
               found_proto = 1;
               break;
            }

          }

          strlfree(&str1);
          if (! found_proto) {

             fprintf(stderr,"ERROR: Cannot negotiate session with peer %s, protocol versions mismatch !\n"
             ,cpeer->host);

          }

       }

     }
     
     //parsing of TLS VALUE
     else if (strstr(fv,"HAS_TLS:") == fv ) {

       fv += (8 * sizeof(char));

       if ( strcmp(fv,"TRUE") == 0  ) {

          cpeer->capacity.has_tls = 1;

       }

       else cpeer->capacity.has_tls = 0;
     }

   }


   strlfree(&str0);

}



int negotiate(peer* cpeer) {

    char io_buffer[STR_BIG_S];
    setZeroN(io_buffer,STR_BIG_S);
    sprintf(io_buffer,"%s %s\r\n",DEXP_NEGOTIATE,cpeer->capacity.proto);
    dexp_send(cpeer,io_buffer,strlen(io_buffer));

}


int receiveNego(peer *cpeer) {

   char io_buffer[STR_BIG_S];
   char* version_ptr;
   setZeroN(io_buffer,STR_BIG_S);
   int len = 0;

   if ( (len = dexp_recv(cpeer,io_buffer,STR_BIG_S*sizeof(char))) > 0 ) {

      version_ptr = trim(io_buffer);

      if (strstr(version_ptr,DEXP_NEGOTIATE) == io_buffer) {

         //warning, BoF right here if sent negotiation string doesn't 
         //exceed sizeof(DEXP_NEGO) +1
         version_ptr += (sizeof(DEXP_NEGOTIATE));
         strncpy(cpeer->capacity.proto,version_ptr,4*sizeof(char));
         
         //debug
         printf("%s Session negotiated with peer %s\n",cpeer->capacity.proto,cpeer->host);
         return 0;
      }

      else {

         fprintf(stderr,"ERROR: invalid Negotiation string\n");
         return -1;

      }

   }

}


int sendInfos (peer* cpeer) {

   extern dexpd_config conf0;
    
   char *infos = (char*) malloc ( (strlen(conf0.node_name) + strlen(conf0.node_descr) + strlen(conf0.node_location) + STR_REG_S ) * sizeof(char) );

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
    
   char *capacity = (char*) malloc ( STR_BIG_S* sizeof(char) );
   char tls_vv[6];
   setZeroN(capacity,STR_BIG_S);

   if (conf0.use_tls == 1) {
   
      strncpy(tls_vv,"TRUE",4*sizeof(char));
   }
   
   else strncpy(tls_vv,"FALSE",5*sizeof(char));

   sprintf(capacity,"Obsidian DEXP\nNODE_NAME:%s\nPROTOCOL_VER:%s\nHAS_TLS:%s",conf0.node_name,DEXP_VERSIONS_TAG,tls_vv);

   dexp_send(cpeer,capacity,strlen(capacity)) ;
   
   free(capacity);
   
}



int sendCatalog(peer *cpeer) {

   extern catalog* cat0;
   extern int nb_cat;

   int i ,j;
   char *cat_header = (char*) malloc (STR_BIG_S* sizeof(char));
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



int flushAnnounceQueue(peer* cpeer) {

  int qpos = cpeer->an_queuesize;
  char ** anqueue = cpeer->announce_queue;
  int i = 0;  
  char announce[81];
  

  while ( i < qpos && cpeer->lock == 0) {

     //debug
     printf("flushing announce queue for host %s\n",cpeer->host);

     setZeroN(announce,81);
     strncpy(announce,"ANNOUNCE ",sizeof(announce));
     strncat(announce,DEXP_AN_ADD_STR,sizeof(announce) - strlen(announce));
     strncat(announce," ",sizeof(announce) - strlen(announce));
     strncat(announce,anqueue[i],sizeof(announce) - strlen(announce));

     strncat(announce,"\r\n",sizeof(announce) - strlen(announce));
     
     dexp_send(cpeer,announce,strlen(announce));
     i++;

  }

  cpeer->announce_queue = unqueue(cpeer->announce_queue,qpos, i);
  cpeer->an_queuesize -= i;
  return 0;

}



int announce(char *hash,int mode) {

   extern dexpd_config conf0;
   int i;   
   char io_buffer[STR_BIG_S];
   int qpos;
   char mode_str[STR_SMALL_S];

   setZeroN(io_buffer,STR_BIG_S);
   setZeroN(mode_str,STR_SMALL_S);

   switch(mode) {

      case DEXP_AN_ADD:
      
         strcpy(mode_str,DEXP_AN_ADD_STR);
         break;

      case DEXP_AN_MOD:
         strcpy(mode_str,DEXP_AN_MOD_STR);
         break;

      case DEXP_AN_DEL:
         strcpy(mode_str,DEXP_AN_DEL_STR);
         break;

      default:
         break;

   }


   strcpy(io_buffer,"ANNOUNCE ");
   strncat(io_buffer,mode_str,STR_BIG_S*sizeof(char) - strlen(io_buffer));
   strncat(io_buffer," ",STR_BIG_S*sizeof(char) - strlen(io_buffer));
   strncat(io_buffer,hash,STR_BIG_S*sizeof(char) - strlen(io_buffer) );
   strncat(io_buffer,"\r\n",STR_BIG_S*sizeof(char) - strlen(io_buffer) );

   for (i=0;i<conf0.nb_peers;i++) {
     
      if (conf0.peers[i].socknum > 0 && conf0.peers[i].lock != 1 ) {

         dexp_send(&(conf0.peers[i]),io_buffer,strlen(io_buffer));

      }

      else if ( conf0.peers[i].lock == 1) {

         printf("Notice: Peer connection is busy, queuing Announce %s\n",hash);
      
         qpos = conf0.peers[i].an_queuesize;
         conf0.peers[i].announce_queue[qpos] = (char*) malloc(65*sizeof(char));
         setZeroN(conf0.peers[i].announce_queue[qpos],65);
         
         strncpy(conf0.peers[i].announce_queue[qpos],hash,64*sizeof(char));

         conf0.peers[i].an_queuesize++;
         
         conf0.peers[i].announce_queue = (char**) realloc(
         conf0.peers[i].announce_queue,
         (conf0.peers[i].an_queuesize +1)*sizeof(char*));
                        
      }

      


   }

   setZeroN(io_buffer,STR_BIG_S);

}

int sendDoc(peer* cpeer,char *hash) {

   FILE *fh;
   char file_buffer[1000];
   char send_buffer[STR_BIG_S];
   char file_path[STR_BIG_S];
   long file_length;
   char fl_str[STR_SMALL_S];
   int i,j;
   int found = 0;

   extern catalog* cat0;
   extern dexpd_config conf0;
   extern int nb_cat;


   setZeroN(send_buffer,STR_BIG_S);
   
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
   
   setZeroN(file_path,STR_BIG_S);
   strncpy(file_path,conf0.data_dir,STR_BIG_S*sizeof(char));
   strncat(file_path,"/",STR_BIG_S*sizeof(char) - strlen(file_path));
   strncat(file_path,cat0[i].filename,STR_BIG_S*sizeof(char) - strlen(file_path));

   printf(":%s\n",file_path);

   fh = fopen(file_path,"rb");
   setZeroN(file_path,STR_BIG_S);

   if (!fh) {

     strcat(send_buffer,"401 CANNOT OPEN FILE FOR READING\r\n");
     dexp_send(cpeer,send_buffer,strlen(send_buffer));
     return -2;


   }

   fseek(fh, 0L, SEEK_END);
   file_length = ftell(fh);
   fseek(fh,0L,0);
   sprintf(fl_str,"%ld",file_length);
   
   setZeroN(send_buffer,STR_BIG_S);
   strncpy(send_buffer,"DOCUMENT:",sizeof(send_buffer));
   strncat(send_buffer,cat0[i].filename,sizeof(send_buffer) - strlen(send_buffer));
   strncat(send_buffer,":",sizeof(send_buffer) - strlen(send_buffer));
   strncat(send_buffer,fl_str,sizeof(send_buffer) - strlen(send_buffer));
   strncat(send_buffer,"\r\n",sizeof(send_buffer) - strlen(send_buffer));

   dexp_send(cpeer,send_buffer,strlen(send_buffer));

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

      cpeer->lock= 1;
      fetch_doc(cpeer,hash);
      cpeer->lock = 0;
      return 0;

   } 

   return -1;


}




int take_action(int socknum,peer* cpeer,void* io_buffer) {

  char *input = (char*) io_buffer;

  stringlist str0;
  str0 = explode(input,' ');
  char* catalog_str;
  hash_queue *hq0;
  int nb_hq;

  //printf("NB_ARGS: %d\n",str0.nb_strings);
    
  if (str0.nb_strings > 0) {

    //debug
    //printf("%s",str0.strlist[0]);

    if (strstr( str0.strlist[0] , DEXP_GETINFOS ) == str0.strlist[0] ) {

       sendInfos(cpeer);

    }  

    else if (strstr( str0.strlist[0] , DEXP_GETCAPA ) == str0.strlist[0] ) {

       sendCapa(cpeer);

    }    

    else if (strstr( str0.strlist[0] , DEXP_GETCATALOG ) == str0.strlist[0] ) {

       sendCatalog(cpeer);

    }

    else if (strcmp( str0.strlist[0], DEXP_READY ) == 0 ) {
    
       //retrieve catalog from peer
       if (cpeer->has_catalog == 0 && cpeer->sync_mode == SYNC_NORMAL && cpeer->pub != 1) {

           
           dexp_send(cpeer,"GET_CATALOG\r\n",14);
           catalog_str = receive_catalog(cpeer);

           if (catalog_str != NULL) {
              nb_hq = 0;
              hq0 = (hash_queue*) malloc(1 * sizeof(hash_queue));
              hq0 = register_hashes(catalog_str,hq0,&nb_hq);
              free(catalog_str);
              if (nb_hq > 0 ) {
                fetch_docs(cpeer,hq0,&nb_hq);
              }
              cpeer->has_catalog = 1;

           }
          
       }

    }

    else if (strstr( str0.strlist[0] , DEXP_ANNOUNCE ) == str0.strlist[0] && cpeer->pub != 1) {

        if (str0.nb_strings < 3) {

            dexp_send(cpeer,"300 MISSING ARGUMENT\r\n",22);
            return -2;
       } 

       process_announce(cpeer,trim(str0.strlist[2]));
       
       
    }


    else if (strstr( str0.strlist[0] , DEXP_GETDOCUMENT ) == str0.strlist[0] ) {


       if (str0.nb_strings < 2) {

            dexp_send(cpeer,"300 MISSING ARGUMENT\r\n",22);
            return -2;
       } 

       cpeer->lock = 1;
       sendDoc(cpeer,trim(str0.strlist[1]));
   
    }

    else if (strstr( str0.strlist[0] , DEXP_STARTTLS ) == str0.strlist[0] ) {

       printf("Starting TLS communication...\n");
       cpeer->ssl = (SSL*) start_tls(socknum);

      
    }

     else if (strstr( str0.strlist[0] , DEXP_FIN ) == str0.strlist[0] ) {

       cpeer->lock = 0;
      
    }




  }

}



char* receive_catalog(peer* cpeer) {

  char io_buffer[STR_BIG_S];
  char cat_part[STR_BIG_S];
  int len;
  char *head_end_ptr;
  int header_len;
  int catpart_len;
  int cat_len;
  stringlist cat_params;
  char* catalog_str;
  int xfr_size = 0;
  int i;


  if ( (len = dexp_recv(cpeer,io_buffer,STR_BIG_S*sizeof(char))) > 0 ) {

        if (strstr(io_buffer,"CATALOG") == io_buffer ) {

          head_end_ptr = strstr(io_buffer,"\r\n");
          if (head_end_ptr != NULL) {
            header_len = (head_end_ptr - io_buffer) + 2;
            catpart_len = len - header_len;
          }

         //debug
         //printf("HEADER_LEN:%d | CAT_LEN:%d\n",header_len,cat_len);

         if (catpart_len > 0) {
            memcpy(cat_part,head_end_ptr+2,sizeof(char) * catpart_len);    
            for(i=header_len;i<STR_BIG_S;i++) {
              io_buffer[i] = '\0';
            }            
         }
 

         //debug
         //printf("IO_BUFFER: %s\n",io_buffer);
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

            setZeroN(io_buffer,STR_BIG_S);
            len = dexp_recv(cpeer,io_buffer,STR_BIG_S*sizeof(char));
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

  io_buffer = (void*) malloc(STR_BIG_S*sizeof(char));

  //initialize announce_queue
  current_peer->announce_queue = (char**) malloc(1*sizeof(char*));
  current_peer->an_queuesize = 0;

  sendCapa(current_peer);
  receiveNego(current_peer);

  while(current_peer->socknum != -1) {

     if ( dexp_recv(current_peer,io_buffer,STR_BIG_S*sizeof(char)) > 0 ) {

        //printf("%s\n",(char*) io_buffer);

        take_action(current_peer->socknum,current_peer,io_buffer);
        setZeroN((char*)io_buffer,STR_BIG_S);

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

   FILE* fh,*ftest;
   int mode = 0;
   char doc_query[80];
   char io_buffer[STR_BIG_S];   
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

   setZeroN(doc_query,80);

   strcpy(doc_query,"GET_DOCUMENT ");
   strcat(doc_query,hash);
   strcat(doc_query,"\r\n");
   //printf("QUERY: %s",doc_query);

   dexp_send(cpeer,doc_query,strlen(doc_query));
   
   file_part = (void*) malloc(STR_BIG_S*sizeof(char));
   setZeroN((char*)file_part,STR_BIG_S);


   setZeroN(io_buffer,STR_BIG_S);
   
   if ( (len = dexp_recv(cpeer,io_buffer,STR_BIG_S * sizeof(char))) > 0 ) {
     

      if (strstr(io_buffer,"DOCUMENT") == io_buffer ) {

          head_end_ptr = strstr(io_buffer,"\r\n");
          if (head_end_ptr != NULL) {
            header_len = (head_end_ptr - io_buffer) + 2;
            fpart_len = len - header_len;
          }

         //debug
         //printf("HEADER_LEN:%d | FPART_LEN:%d\n",header_len,fpart_len);

         if (fpart_len> 0) {
            memcpy(file_part,head_end_ptr+2,sizeof(char) * fpart_len);    
            for(i=header_len;i<STR_BIG_S;i++) {
              io_buffer[i] = '\0';
            }            

         }

         //debug
         //printf("IO_BUFFER: %s\n",io_buffer);

         
         doc_params = explode(io_buffer,':');

         if ( doc_params.nb_strings < 3 ) return -1;

         //security, to avoid passing doc headers containing cannonical paths and/or/ upper directory references.
         if ( strstr(doc_params.strlist[1],"/") == doc_params.strlist[1] ||\
              strstr(doc_params.strlist[1],"../") != NULL ) {
              
            return -2;
         }


         setZero(file_path);
         strncpy(file_path,conf0.tmp_dir,sizeof(file_path));
         strncat(file_path,"/",sizeof(file_path) - strlen(file_path));
         strncat(file_path,hash,sizeof(file_path) - strlen(file_path));        

   
         setZero(file_dest);
         strncpy(file_dest,conf0.data_dir,sizeof(file_dest));
         strncat(file_dest,"/",sizeof(file_dest) - strlen(file_dest));
         strncat(file_dest,doc_params.strlist[1],sizeof(file_dest) - strlen(file_dest));


         //dirty Hack to avoid downloading the same file from multiple sources
         //Solution: The downloads hashqueue must be raised one level up 
         //(not thread dependant but program dependant)
         if ( (ftest = fopen ( file_path, "r" ) ) != NULL ) return -3;
         
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

         setZeroN(io_buffer,STR_BIG_S);
         while(xfr_len < file_len) {


            if ( (len = dexp_recv(cpeer,io_buffer,STR_BIG_S*sizeof(char))) > 0 ) {

              fwrite(io_buffer,1,len * sizeof(char),fh);
              xfr_len +=len;

            }

            setZeroN(io_buffer,STR_BIG_S);

         }


         printf("Notice: file %s fetched succesfully\n",doc_params.strlist[1]);
         fclose(fh);
         rename(file_path,file_dest);
         dexp_send(cpeer,DEXP_FIN,sizeof(DEXP_FIN));

     
       }

    
   }

}


void fetch_docs(peer *cpeer,hash_queue* hq0,int* nb_hq) {

   int i;
   
   for (i=0;i<*nb_hq;i++) {

      cpeer->lock=1;
      fetch_doc(cpeer,hq0[i].hash);
      cpeer->lock=0;
   }

}


void *session_thread_cli(void * p_input) {

  peer* current_peer = (peer*) p_input;
  char io_buffer[STR_BIG_S];

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


  //retrieve server capacities
  dexp_recv(current_peer,io_buffer,STR_BIG_S*sizeof(char));
  parse_capacity(current_peer,io_buffer);
  setZeroN(io_buffer,STR_BIG_S);

  
  //negotiate sessions parameters
  negotiate(current_peer);

  

    
  if (current_peer->capacity.has_tls) {
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

  //debug
  //printf("MODE IDLE \n");
  mode = DEXPMODE_IDLE;
  dexp_send(current_peer,DEXP_READY,sizeof(DEXP_READY));

  while(current_peer->socknum != -1) {

     if ( (len = dexp_recv(current_peer,io_buffer,STR_BIG_S*sizeof(char))) > 0 ) {

         switch(mode) {

            case DEXPMODE_IDLE:
               take_action(current_peer->socknum,current_peer,io_buffer);
               break;
             
            default:
               take_action(current_peer->socknum,current_peer,io_buffer);
               break;           
         }


     }

     setZeroN(io_buffer,STR_BIG_S);
     
  }

}

int createPeer(char* host,int socknum,uint8_t isPublic) {

  extern dexpd_config conf0;
  int cpos = 0;
  
  conf0.nb_peers++;
  conf0.peers = (peer*) realloc(conf0.peers,conf0.nb_peers * sizeof(peer) +1);
  cpos = conf0.nb_peers-1;

  memcpy(conf0.peers[cpos].host,host,STR_REG_S*sizeof(char));
  
  conf0.peers[cpos].has_catalog=0;
  conf0.peers[cpos].pub = isPublic;
  conf0.peers[cpos].socknum = socknum;
  conf0.peers[cpos].ssl = NULL; 

 
}