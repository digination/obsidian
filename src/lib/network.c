#include "network.h"


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
    
    if(bind(sock,(struct sockaddr *)&sin,sizeof(sin))<0) {

        fprintf(stderr,"ERROR: CANNOT BIND SOCKET ON %s:%d\n",addr,port);

    }

    listen(sock,5);  

	return sock;

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


  if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        return -3;
  }

  else {

      return sockfd; 

  }



}







