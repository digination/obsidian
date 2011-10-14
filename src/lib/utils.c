#include "utils.h"


int setZero(char * str) {

   int i = 0;
   int len = strlen(str);
   for (i=0;i<len;i++) {

      str[i] = '\0';
   }

}


int setZeroN(char * str,int len) {

   int i = 0;
   for (i=0;i<len;i++) {
      str[i] = '\0';
   }
}




stringlist explode(char *str, char separator) {

  stringlist res;

  if (str == NULL) { 
 
     res.strlist = NULL;
     res.nb_strings = 0;
     return res;
  }
 
   int  nbstr = 1;
   int  len;
   int  from = 0;
   int  i;
   int  j;
   
   res.strlist = (char **) malloc(sizeof (char *));
     len = strlen(str);
     for (i = 0; i <= len; ++i)
     {
         if ((i == len) || (str[i] == separator)) {
            res.strlist = (char **) realloc(res.strlist, ++nbstr * sizeof (char *));
            res.strlist[nbstr - 2] = (char *) malloc((i - from + 1) * sizeof (char));
            for (j = 0; j < (i - from); ++j) res.strlist[nbstr - 2][j] = str[j + from];
            res.strlist[nbstr - 2][i - from] = '\0';
            from = i + 1;
            ++i;
            
        }
    }
    res.strlist[nbstr - 1] =  NULL;
    res.nb_strings = nbstr - 1;
    return res;
}


int strlfree(stringlist *str0) {

   int i = 0;
   for (i=0;i<str0->nb_strings;i++) {
   
      free(str0->strlist[i]);
   }

   free(str0->strlist);
}


int strippable(char c) {

   if (isspace(c) || c == '\r' || c == '\n') return 1;

   return 0;
   

}


char *trim(char *str)
{
      char *ibuf = str, *obuf = str;
      int i = 0, cnt = 0;

      if (str)
      {
         
            for (ibuf = str; *ibuf && strippable(*ibuf); ++ibuf)
                  ;
            if (str != ibuf)
                  memmove(str, ibuf, ibuf - str);

            while (*ibuf)
            {
                  if (strippable(*ibuf) && cnt)
                        ibuf++;
                  else
                  {
                        if (!strippable(*ibuf))
                              cnt = 0;
                        else
                        {
                              *ibuf = ' ';
                              cnt = 1;
                        }
                        obuf[i++] = *ibuf++;
                  }
            }
            obuf[i] = NUL;

            while (--i >= 0)
            {
                  if (!strippable(obuf[i]))
                        break;
            }
            obuf[++i] = NUL;
      }
      return str;
}


int stricmp(const char* s1, const char* s2)
{
    int    c1, c2;
    int    cmp = 0;

    if (s1 && s2)
    for (;;)
    {
        c1 = *s1++;
        c2 = *s2++;
        if (c1 && c2)
        {
            c1 = tolower(c1)&0xFF; // 8 bits
            c2 = tolower(c2)&0xFF; // only
            if (c1 < c2)
            {
                cmp = -1;
                break;
            }
            else if (c1 > c2)
            {
                cmp = 1;
                break;
            }
        }
        else
        {
            if (c1)
                cmp = 1;
            else if (c2)
                cmp = -1;
            break;
        }
    }
    return cmp;
}


char ** strshift(char** strlist,int shift) {

   

   return strlist;

}