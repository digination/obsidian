#include "obsidian.h"


int isPeer(char *peer_str,int sock_fd) {

   int i =0;
   extern dexpd_config conf0;
  
   struct hostent* pname;

   for (i=0;i<conf0.nb_peers;i++) {

     //skip ipv6-coded peers if available
     if (strstr(conf0.peers[i].host,':') == NULL ) {

        pname = gethostbyname(conf0.peers[i].host);

        if ( strcmp(pname->h_name,peer_str) == 0 ) {

           conf0.peers[i].socknum = sock_fd;
           return i;
        }

     }
     
   }

   return -1;

}


int isPeerV6(char *peer_str,int sock_fd) {

   int i =0;
   extern dexpd_config conf0;
  
   struct hostent* pname;

   for (i=0;i<conf0.nb_peers;i++) {

     pname = gethostbyname2(conf0.peers[i].host,AF_INET6);

     if ( strcmp(pname->h_name,peer_str) == 0 ) {

     conf0.peers[i].socknum = sock_fd;
     return i;
     }

     
   }

   return -1;

}


int connectAll() {

  extern dexpd_config conf0;
  int i = 0;
  int socknum;
  
  if (conf0.peers == NULL) fprintf(stderr,"ERROR: NO PEERS IN CONFIGURATION\n");

  for (i=0;i<conf0.nb_peers;i++) {
          
      conf0.peers[i].socknum = -1;
	  conf0.peers[i].ssl = NULL;

      if (conf0.use_ipv6) {

         if ( (socknum = pconnect6(conf0.peers[i].host,conf0.peers[i].port)) < 0 ) {


         }
      

         else {

            printf("link established with peer %s !\n" , conf0.peers[i].host); 
            conf0.peers[i].socknum = socknum;
            pthread_create(&conf0.peers[i].thread,NULL,session_thread_cli,(void*)&conf0.peers[i]);
         }


      }


      if ( conf0.peers[i].socknum == -1 && (socknum = pconnect(conf0.peers[i].host,conf0.peers[i].port)) < 0 ) {

          fprintf(stderr,"Notice: cannot connect to peer %s\n", conf0.peers[i].host );

      }
      

      else if (conf0.peers[i].socknum == -1) {

         printf("link established with peer %s !\n" , conf0.peers[i].host); 
         conf0.peers[i].socknum = socknum;
         pthread_create(&conf0.peers[i].thread,NULL,session_thread_cli,(void*)&conf0.peers[i]);
      }

  }  

  return 0;


}


int try_reconnect() {

  extern dexpd_config conf0;
  int i = 0;
  int socknum;

  for (i=0;i<conf0.nb_peers;i++) {
          
      if (conf0.peers[i].socknum == -1) {

         printf("trying to reconnect to peer %s...\n", conf0.peers[i].host );

         if ( (socknum = pconnect(conf0.peers[i].host,conf0.peers[i].port)) < 0 ) {

          fprintf(stderr,"Notice: cannot connect to peer %s\n", conf0.peers[i].host );

         }
      
         else { 
            printf("link established with peer %s !\n" , conf0.peers[i].host);
            conf0.peers[i].socknum = socknum;
            pthread_create(&conf0.peers[i].thread,NULL,session_thread_cli,(void*)&conf0.peers[i]);
         }

     }
  
   }

   return 0;

}

void* reconnect_loop() {

  pthread_detach(pthread_self());

  while(1) {

     sleep(90);

     try_reconnect();


  }


}


int load_catalog(char* data_dir_str) {

   extern catalog* cat0;
   extern int nb_cat;

   DIR *data_dir;
   struct dirent *DirEntry;
   SHA256_CTX context;
   unsigned char md[SHA256_DIGEST_LENGTH];
   FILE *fh;
   char fto[4096];
   char buf[256];
   char hexpart[3];
   int i =0;
   int j = 0;


   //we start allocating 10K entries on catalog then realloc() if necesary
   cat0 = (catalog*) malloc(10000 * sizeof(catalog) );

   nb_cat = 0;
   data_dir = opendir(data_dir_str);

   if (!data_dir) { 

     fprintf(stderr,"ERROR: CANNOT OPEN DATA DIRECTORY\n");
     exit(1);
   }

   while ( (DirEntry = readdir(data_dir)) != NULL ) {

     if (strstr(DirEntry->d_name,".") != DirEntry->d_name ) {

        //printf( "%s\n",DirEntry->d_name);

        strncpy(cat0[nb_cat].filename,DirEntry->d_name,254* sizeof(char));

        strncpy(fto,data_dir_str,4096 * sizeof(char));
        strncat(fto,"/",4096 * sizeof(char) - strlen(fto) );
        strncat(fto,DirEntry->d_name,4096 * sizeof(char) - strlen(fto) );

        SHA256_Init(&context);

        if ( (fh = fopen(fto,"rb")) == NULL ) {

          fprintf(stderr,"ERROR: CANNOT OPEN FILE %s\n",fto);
          exit(1);

        } 
       
        while( ( i = fread( buf, 1, sizeof( buf ), fh ) ) > 0 )
        {
            SHA256_Update( &context, buf, i );
        }
        

        SHA256_Final(md, &context);

        for (i=0;i<SHA256_DIGEST_LENGTH;i++){

           sprintf(hexpart,"%02x",md[i]);
           strncat(cat0[nb_cat].hash,hexpart,2*sizeof(char));

        }
  
        nb_cat++;


   }

   }


    //printf( "======\n");

   //debug 
   /*  
   for (i=0;i<nb_cat;i++) {

      printf( "%s:%s\n", cat0[i].hash,cat0[i].filename );
     
   }
   */  


}


int main(int argc, char** argv) {


   extern dexpd_config conf0;


   int socknum = 0;
   int sock_id = 0;

   int wd = 0;
   int notify_fd = 0;

   char *peer_str;
   int peer_num;

   pthread_t notify_th;
   pthread_t reconnect_th;
   pthread_t listenv6;

   struct sockaddr_in peer_addr;
   socklen_t addr_len = sizeof(peer_addr);

  
   printf ("\n");

   printf ("        /\\\n");
   printf ("       /01\\\n");
   printf ("      /0011\\\n");
   printf ("     /000111\\     [[ Obsidian 0.1 == Clement Game 2011 ]]\n");
   printf ("    /00001111\\\n");
   printf ("   /0000011000\\\n");
   printf ("   \\0000000000/\n\n");


   init_config();

   
   if (conf0.use_tls) {
      printf("initializing TLS config...\n");
      conf0.ctx=(SSL_CTX*)initialize_ctx(conf0.tls_server_cert,"password");
      load_dh_params(conf0.ctx,conf0.tls_server_dh);
   }

   printf("Loading catalog from %s...\n",conf0.data_dir);

   load_catalog(conf0.data_dir);
   printf("starting inotify events listener...\n");

   //inotify init functions on datadir;
   notify_fd = inotify_init();
   wd = inotify_add_watch(notify_fd, conf0.data_dir, IN_CLOSE_WRITE|IN_MOVED_TO);

   //starting a new thread for inotify events read
   pthread_create(&notify_th,NULL,notify_thread,&notify_fd);


   printf("initating connections with peers...\n");
   connectAll();
   pthread_create(&reconnect_th,NULL,reconnect_loop,NULL);
   
   printf("listening on %s:%d...\n",conf0.listening_addr,conf0.listening_port);

   socknum  = create_socket(conf0.listening_addr,conf0.listening_port);

   if (conf0.use_ipv6) {

      printf("listening on %s:%d...\n",conf0.ipv6_listening_addr,conf0.listening_port);
      pthread_create(&listenv6,NULL,listen_v6,NULL);

   }

  

   while(1) {

      if((sock_id = accept(socknum,(struct sockaddr*)&peer_addr,&addr_len))<0) {

         fprintf(stderr,"ERROR: CANNOT ACCEPT NEW CONNECTION ON %s:%d",conf0.listening_addr,conf0.listening_port);

      }

      else {

         peer_str = inet_ntoa(peer_addr.sin_addr);
         printf("New Connection From %s\n",peer_str);

         //gerer la notion de public ici
         if ( (peer_num = isPeer(peer_str,sock_id)) < 0 ) {

             fprintf(stderr,"ERROR: REMOTE HOST NOT In PEERS LIST\n");
             close(sock_id);

         }  

         else {
			 
             conf0.peers[peer_num].ssl = NULL;
             pthread_create(&conf0.peers[peer_num].thread,NULL,session_thread_serv,(void*)&conf0.peers[peer_num]);
  

         }

 
     
      }


		
   }


}




void* listen_v6() {

   extern dexpd_config conf0;
   int socknum = create_socket_v6(conf0.ipv6_listening_addr,conf0.listening_port);
   int sock_id;
   char peer_buf[40];
   char *peer_str;
   int peer_num;
   
   struct sockaddr_storage ss;
   struct sockaddr_in6 *peer_addr = (struct sockaddr_in6 *) &ss;
   
   socklen_t addr_len = sizeof(peer_addr);


   if (socknum < 0) {

      fprintf(stderr,"ERROR: CANNOT CREATE SOCKET IPv6 ON %s:%d\n",conf0.ipv6_listening_addr,conf0.listening_port);
      return;
   }

    while(1) {

      if((sock_id = accept(socknum,(struct sockaddr*)peer_addr,&addr_len))<0) {

         fprintf(stderr,"ERROR: CANNOT ACCEPT NEW CONNECTION ON %s:%d",conf0.ipv6_listening_addr,conf0.listening_port);

      }

      else {

         inet_ntop(AF_INET6,(void*)&peer_addr->sin6_addr, peer_buf,39*sizeof(char));
         printf("New Connection From %s\n",peer_str);

         //gerer la notion de public ici
         if ( (peer_num = isPeerV6(peer_buf,sock_id)) < 0 ) {

             fprintf(stderr,"ERROR: REMOTE HOST NOT In PEERS LIST\n");
             close(sock_id);

         }  

         else {
			 
             conf0.peers[peer_num].ssl = NULL;
             pthread_create(&conf0.peers[peer_num].thread,NULL,session_thread_serv,(void*)&conf0.peers[peer_num]);
  

         }
     
      }
		
   }

 
}

