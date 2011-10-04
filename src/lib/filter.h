#ifndef FILTER_H
#define FILTER_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include <stdint.h>

#define MAX_FILTER_STRLEN 512


#define FILTER_ACT_ALLOW 0x01
#define FILTER_ACT_DENY 0x02

#define FILTER_OBJ_SIZE 0x01
#define FILTER_OBJ_FILENAME 0x02

#define FILTER_OP_CONTAINS 0x01
#define FILTER_OP_EQUALS 0x02
#define FILTER_OP_EQ 0x03
#define FILTER_OP_GT 0x04
#define FILTER_OP_LT 0x05
#define FILTER_OP_GTE 0x06
#define FILTER_OP_LTE 0x07



typedef struct filter {
 
  int id;
  int action;
  char from[MAX_FILTER_STRLEN];
  uint8_t object;
  uint8_t op;  
  
  char operand[MAX_FILTER_STRLEN];

} filter;



typedef struct ruleset {

  filter* rules;
  int nb_rules;

} ruleset;


filter* parse_filter(int,char*);

#endif

