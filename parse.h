#ifndef parse_h
#define parse_h

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#define INC_LOW_TEMP  	55
#define INC_HIGH_TEMP 	65
#define INC_MAX_TEMP  	82

#define BASE_LEVEL	1
#define INC_LOW_LEVEL	2
#define INC_HIGH_LEVEL	4
#define INC_MAX_LEVEL	7

#define DEC_LOW_LEVEL	4
#define DEC_HIGH_LEVEL	5
#define DEC_MAX_LEVEL	7

#define DEC_THRESH  	1

#define DEC_LOW_TEMP  	50
#define DEC_HIGH_TEMP 	60
#define DEC_MAX_TEMP  	77

#define POLL_INTERVAL 	10
#define BUFF_MAX	128
#define PATH_MAX	128

#define STREQ(x,y)	(strcmp((x),(y)) == 0)

struct config {
 	unsigned short inc_low_temp;
 	unsigned short inc_high_temp;
 	unsigned short inc_max_temp;

 	unsigned short inc_low_lvl;
 	unsigned short inc_high_lvl;
 	unsigned short inc_max_lvl;

 	unsigned short dec_low_temp;
 	unsigned short dec_high_temp;
 	unsigned short dec_max_temp;

 	unsigned short dec_low_lvl;
 	unsigned short dec_high_lvl;
 	unsigned short dec_max_lvl;

 	unsigned short base_lvl;
 	unsigned short poll_int;
 	unsigned short dec_thres;
 };

size_t strtrim(char *str);
int parse_config(struct config *cfg);
int config_path_exists(char *path, int pathlen);

#endif