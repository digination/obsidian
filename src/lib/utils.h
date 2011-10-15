#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"

#define NUL '\0'

typedef struct stringlist {

   char **strlist;
   int nb_strings;

} stringlist;


stringlist explode(char*,char);
int strlfree(stringlist *);
int setZero(char *);
int setZeroN(char *,int);
char* trim(char*);
int strippable(char c);
int stricmp(const char* s1, const char* s2);
char ** unqueue(char**,int,int);


#endif
