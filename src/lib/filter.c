#include "filter.h"

//filter line sample:
// filter deny from 127.0.0.1 size gt 30000 
//filter allow from 127.0.0.1 filename contains ".pdf" 
//filter allow from all hash equals "aaaaffffbbbbb"  


filter* parse_filter(int line,char* filter_line) {

   filter_line = trim(filter_line);

   filter* result = (filter*) malloc(sizeof(filter)) ;
   int i = 0;
   stringlist sl0;

   sl0 = explode(filter_line,' ');

   if (sl0.nb_strings < 4 ) {

     fprintf(stderr,"WARNING: Invalid Filter Rule line %d:\"%s\"\n",line,filter_line);
     return NULL;   
   }

   if (sl0.nb_strings > 4 &&  sl0.nb_strings < 7 ) {

     fprintf(stderr,"WARNING: Invalid Filter Rule line %d:\"%s\"\n",line,filter_line);
     return NULL;   
   }

   
   if (stricmp(sl0.strlist[1],"allow") == 0 ) {

      result->action = FILTER_ACT_ALLOW;

   }

   else if (stricmp(sl0.strlist[1],"deny") == 0 ) {

      result->action = FILTER_ACT_DENY;

   }
   
   else {

     fprintf(stderr,"WARNING: Invalid Filter Rule line %d:\"%s\", invalid action keyword \"%s\"\n",line,filter_line,sl0.strlist[1]);
     return NULL; 

   }


   if (stricmp(sl0.strlist[2],"from") != 0 ) {

     fprintf(stderr,"WARNING: Invalid Filter Rule line %d:\"%s\", invalid keyword \"%s\"\n",line,filter_line,sl0.strlist[2]);
     return NULL; 

   }

   strncpy((*result).from,sl0.strlist[3],MAX_FILTER_STRLEN * sizeof(char) -1);

   if (sl0.nb_strings == 4) {
   
      return result;
   }
 
   return NULL;




}


