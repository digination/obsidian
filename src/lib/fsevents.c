#include "fsevents.h"


char* add_hash(char *filename) {

  extern catalog* cat0;
  extern int nb_cat;
  extern dexpd_config conf0;   
  int i;

  char* res;

  SHA256_CTX context;
  unsigned char md[SHA256_DIGEST_LENGTH];
  char buf[STR_REG_S];
  char hexpart[3];


  char *file_path;
  FILE* fh;


  file_path = (char*) malloc( (strlen(conf0.data_dir) + strlen(filename) + 2 ) * sizeof(char)  );

  setZeroN(file_path,strlen(conf0.data_dir) + strlen(filename) + 2);

  strncpy(file_path,conf0.data_dir,strlen(conf0.data_dir) * sizeof(char));
  strncat(file_path,"/",1 * sizeof(char));
  strncat(file_path,filename,strlen(filename) * sizeof(char));
  
  SHA256_Init(&context);

  if ( (fh = fopen(file_path,"rb")) == NULL ) {

    fprintf(stderr,"ERROR: CANNOT OPEN FILE %s\n",file_path);
    exit(1);

   } 
       
   while( ( i = fread( buf, 1, sizeof( buf ), fh ) ) > 0 ) {
      SHA256_Update( &context, buf, i );
   }
        
   SHA256_Final(md, &context);

   if (nb_cat > 99999) {
     cat0 = (catalog*) realloc(cat0,(nb_cat+1) * sizeof(catalog));
   }


   strncpy(cat0[nb_cat].filename,filename,sizeof(cat0[nb_cat].filename));

   for (i=0;i<SHA256_DIGEST_LENGTH;i++){

      sprintf(hexpart,"%02x",md[i]);
      strncat(cat0[nb_cat].hash,hexpart,2*sizeof(char));

   }

   res = cat0[nb_cat].hash;

   printf("NEW HASH ADDED:%s:%s\n",cat0[nb_cat].hash,cat0[nb_cat].filename);
   nb_cat++;

   return res;

}


void *notify_thread(void* notify_fd) {

  int fd = 0;
  char buffer[EVENT_BUF_LEN];
  int count = 0;
  int i = 0;
  int length = 0;
  char *result_hash;

  pthread_detach(pthread_self());

  fd = *(int*) notify_fd;


  while(1) {

        i = 0;
        length = read( fd, buffer, EVENT_BUF_LEN ); 
 
         while ( i < length ) {     
            struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ]; 
            printf( "New file %s Editted.\n", event->name );
            i += EVENT_SIZE + event->len;

            result_hash = add_hash(event->name);
            announce(result_hash,DEXP_AN_ADD);

          }

  }


}

