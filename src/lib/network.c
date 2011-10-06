#include "network.h"



int set_keepalive(int socknum) {

   extern dexpd_config conf0;

   int nbr = KEEPALIVE_NBRETRIES;
   int real_interval = conf0.keepalive_timeout / nbr;
   int ka = 1;

   
   if ( setsockopt(socknum,SOL_SOCKET,SO_KEEPALIVE,&ka,sizeof(int))  < 0 ) {
     return -1;
   }

   if ( setsockopt(socknum,SOL_TCP,TCP_KEEPCNT,&nbr,
   sizeof(KEEPALIVE_NBRETRIES))  < 0 ) {
     return -2;
   }

   if ( setsockopt(socknum,SOL_TCP,TCP_KEEPINTVL,&real_interval,
   sizeof(real_interval))  < 0 ) {
     return -2;
   }

   return 0;
   

}


int create_socket(char *addr,int port) {

    int sock;
    struct sockaddr_in sin;
    int val=1;
    
    if((sock=socket(AF_INET,SOCK_STREAM,0))<0) {

       fprintf(stderr,"ERROR: CANNOT CREATE SOCKET\n");
       exit(1);
    }
  
    memset(&sin,0,sizeof(sin));

    if (strcmp(addr,"0.0.0.0") == 0) {
       sin.sin_addr.s_addr=INADDR_ANY;
    }

    else {
        sin.sin_addr.s_addr = inet_addr(addr);
    }

    sin.sin_family=AF_INET;
    sin.sin_port=htons(port);
    setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,
      &val,sizeof(val));

    if (set_keepalive(sock) < 0) {
       fprintf(stderr,"ERROR: CANNOT ASSIGN KEEPALIVE OPTIONS TO SOCKET");
    }
    
    if(bind(sock,(struct sockaddr *)&sin,sizeof(sin))<0) {
        fprintf(stderr,"ERROR: CANNOT BIND SOCKET ON %s:%d\n",addr,port);
    }

    listen(sock,5);  

	return sock;

}



int create_socket_v6(char *addr,int port) {

    int sock;
    //struct sockaddr_in6 sin;
    int val=1;
    int i;
    uint8_t ip6addr[16];
    
   struct sockaddr_storage ss;
   struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) &ss;



    if((sock=socket(AF_INET6,SOCK_STREAM,0))<0) {

       fprintf(stderr,"ERROR: CANNOT CREATE SOCKET\n");
       exit(1);
    }
  
    //memset(&sin,0,sizeof(sin));

    if (strcmp(addr,"::") == 0) {

       for (i=0;i<16;i++) {
          sin6->sin6_addr.s6_addr[i] = 0x00;
       }
       
    }

    else {

        inet_pton(AF_INET6,addr,ip6addr);
        memcpy(sin6->sin6_addr.s6_addr,ip6addr,16*sizeof(uint8_t));

    }


    sin6->sin6_family=AF_INET6;
    sin6->sin6_port=htons(port);
    setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,
      &val,sizeof(val));


    if (set_keepalive(sock) < 0) {
       fprintf(stderr,"ERROR: CANNOT ASSIGN KEEPALIVE OPTIONS TO SOCKET");
    }

    
    if(bind(sock,(struct sockaddr *)sin6,sizeof(ss))<0) {

        fprintf(stderr,"ERROR: CANNOT BIND SOCKET ON %s:%d\n",addr,port);

    }

    listen(sock,5);  

	return sock;

}



int pconnect6(char* host,int portno) {


  int sockfd, n;
  struct sockaddr_storage ss;
  struct sockaddr_in6 *serv_addr = (struct sockaddr_in6 *) &ss;
  struct hostent *server;
  uint8_t ip6addr[16];

  sockfd = socket(AF_INET6, SOCK_STREAM, 0);
  if (sockfd < 0) { 
        return -1;
  }

  server = gethostbyname2(host,AF_INET6);
  if (server == NULL) { 
      return -2;
  }


  serv_addr->sin6_family = AF_INET6;
  memcpy(&serv_addr->sin6_addr,server->h_addr,server->h_length);

  serv_addr->sin6_port = htons(portno);

  if (set_keepalive(sockfd) < 0) {
       fprintf(stderr,"ERROR: CANNOT ASSIGN KEEPALIVE OPTIONS TO SOCKET");
  }

  if (connect(sockfd,(struct sockaddr *) serv_addr,sizeof(ss)) < 0) {
        return -3;
  }

  else {

      return sockfd; 

  }

}


int pconnect(char* host,int portno) {


  int sockfd, n;
  struct sockaddr_in serv_addr;
  struct hostent *server;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) { 
        return -1;
  }

  server = gethostbyname(host);
  if (server == NULL) { 
      return -2;
  }

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(portno);

  if (set_keepalive(sockfd) < 0) {
       fprintf(stderr,"ERROR: CANNOT ASSIGN KEEPALIVE OPTIONS TO SOCKET");
  }


  if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        return -3;
  }

  else {

      return sockfd; 

  }

}



int dexp_send(peer* cpeer,void* data,int datalen) {

	int len;
	
	if (cpeer->ssl != NULL) {

	len = SSL_write(cpeer->ssl,data,datalen);
		
	}
	

	else {
	
       len = send(cpeer->socknum,data,datalen,0);
		
	}

   //our socket is disconnected
   if (len <= 0) { 
      cpeer->socknum = -1;
      printf ("HOST %s Disconnected\n",cpeer->host);
   }

   return len;
	
}



int dexp_recv(peer* cpeer,void* dest,int destlen) {

	int len;
	
	if (cpeer->ssl != NULL) {

	len = SSL_read(cpeer->ssl,dest,destlen);
		
	}
	else {
       len = recv(cpeer->socknum,dest,destlen,0);
		
	}

   //our socket is disconnected
   if (len <= 0) { 
      cpeer->socknum = -1;
      printf ("HOST %s Disconnected\n",cpeer->host);
   }
   
   return len;
	
}





