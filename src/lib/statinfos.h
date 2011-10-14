#ifndef STATINFOS_H
#define STATINFOS_H

#include "common.h"

#define SINFO_MAX_STRLEN STR_BIG_S

struct stat_infos {

  int size;
  char filename[SINFO_MAX_STRLEN];
  char hash[65];

} stat_infos;

#endif
