#ifndef PARSE_H
#define PARSE_H

#define INC_LOW_TEMP    55
#define INC_HIGH_TEMP   65
#define INC_MAX_TEMP    82

#define BASE_LEVEL      1
#define INC_LOW_LEVEL   2
#define INC_HIGH_LEVEL  4
#define INC_MAX_LEVEL   6

#define DEC_LOW_LEVEL   4
#define DEC_HIGH_LEVEL  5
#define DEC_MAX_LEVEL   6

#define DEC_LOW_TEMP    50
#define DEC_HIGH_TEMP   60
#define DEC_MAX_TEMP    77

#define MAX_FAN_LVL     7

#define POLL_INTERVAL   10
#define BUFF_MAX        128
#define PATH_MAX        128

#define STR_STARTS_WITH(x,y)  (strncmp((x),(y), strlen(y)) == 0)

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
  unsigned short max_temp;
 };

size_t strtrim(char *str);
void parse_config(struct config *cfg);
int config_path_exists(char *path, int pathlen);
void set_defaults();

#endif
