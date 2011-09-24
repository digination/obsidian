#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NUL '\0'

typedef struct stringlist {

   char **strlist;
   int nb_strings;

} stringlist;


stringlist explode(char*,char);
int setZero(char *);
int setZeroN(char *,int);
char* trim(char*);
int strippable(char c);

#endif
