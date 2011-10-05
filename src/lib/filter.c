#include "filter.h"
#include "config.h"

int take_decision(peer* cpeer,struct stat_infos* si) {

  extern dexpd_config conf0;
  int i = 0;
  ruleset rs_contextual;
  rs_contextual.nb_rules = 0;

  for (i=0;i<conf0.rs.nb_rules;i++) {

     if (strcmp(cpeer->host,conf0.rs.rules[i].from) == 0 ) {


       rs_contextual.nb_rules++;


     }

  }

  return 0;  


}


filter* parse_filter(int line,char* filter_line) {

   filter_line = trim(filter_line);

   filter* result = (filter*) malloc(sizeof(filter)) ;
   int i = 0;
   stringlist sl0;

   sl0 = explode(filter_line,' ');

   if (sl0.nb_strings < 4 ) {

     fprintf(stderr,"Invalid rule line %d:missing arguments\n",line);
     return NULL;   
   }

   if (sl0.nb_strings > 4 &&  sl0.nb_strings < 7 ) {

     fprintf(stderr,"Invalid rule line %d:invalid arguments number\n",line);
     return NULL;   
   }

   
   if (stricmp(sl0.strlist[1],"allow") == 0 ) {

      result->action = FILTER_ACT_ALLOW;

   }

   else if (stricmp(sl0.strlist[1],"deny") == 0 ) {

      result->action = FILTER_ACT_DENY;

   }
   
   else {

     fprintf(stderr,"Invalid rule line %d:invalid action keyword \"%s\"\n",line,sl0.strlist[1]);
     return NULL; 

   }


   if (stricmp(sl0.strlist[2],"from") != 0 ) {

     fprintf(stderr,"Invalid rule line %d:invalid keyword \"%s\"\n",line,sl0.strlist[2]);
     return NULL; 

   }

   strncpy((*result).from,sl0.strlist[3],MAX_FILTER_STRLEN * sizeof(char) -1);

   if (sl0.nb_strings == 4) {
   
      return result;
   }


   if (stricmp(sl0.strlist[4],"hash") == 0 ) {

      result->object = FILTER_OBJ_HASH;

   }

   else if (stricmp(sl0.strlist[4],"filename") == 0 ) {

      result->object = FILTER_OBJ_FILENAME;

   }


   else if (stricmp(sl0.strlist[4],"size") == 0 ) {

      result->object = FILTER_OBJ_SIZE;

   }
   
   else {

     fprintf(stderr,"Invalid rule line %d:invalid object keyword \"%s\"\n",line,sl0.strlist[4]);
     return NULL; 

   }
   
    if (stricmp(sl0.strlist[5],"contains") == 0 ) {

      result->op = FILTER_OP_CONTAINS;

   }
   
   else if (stricmp(sl0.strlist[5],"equals") == 0 ) {
      result->op = FILTER_OP_EQ;  
   }

   else if (stricmp(sl0.strlist[5],"gt") == 0 ) {
      result->op = FILTER_OP_GT;
   }

   else if (stricmp(sl0.strlist[5],"lt") == 0 ) {
      result->op = FILTER_OP_LT;
   }
   
   else if (stricmp(sl0.strlist[5],"gte") == 0 ) {
      result->op = FILTER_OP_GTE;
   }

   else if (stricmp(sl0.strlist[5],"lte") == 0 ) {
      result->op = FILTER_OP_LTE;
   }

   else {
     fprintf(stderr,"Invalid rule line %d:invalid operator keyword \"%s\"\n",line,sl0.strlist[5]);
     return NULL; 
   }

   //handle size specific operators
   if ((*result).object != FILTER_OBJ_SIZE && (*result).op > 3) {
     fprintf(stderr,"Invalid rule line %d:operator \"%s\" only works with size attribute\n",line,sl0.strlist[5]);
     return NULL; 
   }

   //
   strncpy((*result).operand,sl0.strlist[6],MAX_FILTER_STRLEN * sizeof(char) -1);
   
   return result;

}


