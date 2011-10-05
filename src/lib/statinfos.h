#ifndef STATINFOS_H
#define STATINFOS_H

#define SINFO_MAX_STRLEN 4096

struct stat_infos {

  int size;
  char filename[SINFO_MAX_STRLEN];
  char hash[65];

} stat_infos;

#endif