#ifndef ADEP_H
#define ADEP_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "peer.h"
#include "utils.h"
#include "config.h"
#include "common.h"
#include <stdint.h>


//Queries
#define DEXP_GETDOCUMENT "GET_DOCUMENT"
#define DEXP_GETMETADATA "GET_METADATA"
#define DEXP_GETINFOS "GET_INFOS\r\n"
#define DEXP_GETCAPA "GET_CAPA\r\n"
#define DEXP_GETCATALOG "GET_CATALOG\r\n"
#define DEXP_ANNOUNCE "ANNOUNCE"
#define DEXP_PING "PING\r\n"

//Responses
#define DEXP_PONG "PONG\r\n"


//Modes
#define DEXPMODE_IDLE 0x00
#define DEXPMODE_WAIT_CATALOG_HEADER 0x01
#define DEXPMODE_CATALOG_XFR 0x02
#define DEXPMODE_FILE_XFR 0x03
#define DEXPMODE_FETCHDOCS 0x04
#define DEXPMODE_BUSY 0x05



//Headers
#define DEXPHEAD_CATALOG  "CATALOG"




#define BEGIN_XFR "BEGIN_XFR"
#define END_XFR "END_XFR"
#define BEGIN_ANNOUNCE "BEGIN_ANNOUNCE"
#define END_ANNOUNCE "END_ANNOUNCE"

#define SOCK_IDLE 0x00
#define SOCK_ANNOUNCE_RECV_MODE 0x01
#define SOCK_XFR_RECV_MODE 0x02

#define SOCK_ANNOUNCE_SEND_MODE 0x03
#define SOCK_XFR_SEND_MODE 0x04

int has_document(char*,char**,int);
int parse_cmd(char*,char**,int) ;
void *session_thread_serv(void *);
void *session_thread_cli(void *);

void* keepalive_thread();

int announce(char*);
int process_announce(int,char*);
void fetch_doc(int,char*);


#endif

