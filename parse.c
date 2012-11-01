#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include "parse.h"

size_t strtrim(char *str)
{
        char *left = str, *right;

        if(!str || *str == '\0') {
                return 0;
        }
        while(isspace((unsigned char)*left)) {
                left++;
        }
        if(left != str) {
                memmove(str, left, (strlen(left) + 1));
        }
        if(*str == '\0') {
                return 0;
        }

        right = (char*)rawmemchr(str, '\0') - 1;
        while(isspace((unsigned char)*right)) {
                right--;
        }
        *++right = '\0';

        return right - left;
}

int config_path_exists(char *path, int pathlen)
{
        char *cfg_path = "/etc/conf.d";
        struct stat st;

        if (stat(cfg_path, &st) == 0) {
                snprintf(path, pathlen, "%s/simpfand", cfg_path);
                return 1;
        }
        return 0;
}

void parse_config(struct config *cfg)
{
        char line[BUFF_MAX];
        char conf_path[PATH_MAX];
        FILE *fp;

        if (!config_path_exists(conf_path, sizeof(conf_path))) {
                fprintf(stderr, "warning: could not find /etc/conf.d, "
                                "using defaults\n");
                return;
        }

        fp = fopen(conf_path, "r");
        if (!fp) {
                fprintf(stderr, "warning: no config file found "
                                "using defaults\n");
                return;
        }

        while (fgets(line, PATH_MAX, fp)) {
                char *key, *val, *key_cmp = NULL;
                size_t linelen;
                int key_len;

                linelen = strtrim(line);
                if (linelen == 0 || line[0] == '#') {
                        /* go back to top if no line or is a comment*/
                        continue;
                }

                if ((val = strchr(line, '#'))) {
                        *val = '\0';
                }

                key = val = line;
                strsep(&val, "=");
                strtrim(val);
                strtrim(key);
                key_len = strlen(key);

                if (val && !*val) {
                        val = NULL;
                }

                unsigned short read_val;
                char *cpy = val;
                errno = 0;
                read_val = (unsigned short)strtol(val, &cpy, 10);
                if (errno != 0 || cpy == val || *cpy != 0) {
                        fprintf(stderr, "simpfand: invalid entry in config for %s: \"%s\"", key, cpy);
                } else {
                        /* just look at the last 4 chars */
                        key_cmp = &key[key_len-4];

                        if (STR_STARTS_WITH(key_cmp, "_LVL") && read_val > 7) {
                                fprintf(stderr, "warning: \"%s\" set greater than max level (7), "
                                        "using default value\n", key);
                                continue;
                        } else if (STR_STARTS_WITH(key_cmp, "_LVL") && read_val <= 0) {
                                /* I refuse to let anyone turn off their fan */
                                fprintf(stderr, "warning: \"%s\" set less or equal to than zero, "
                                        "using default value\n", key);
                                continue;
                        } else if (STR_STARTS_WITH(key_cmp, "TEMP") && read_val > cfg->max_temp) {
                                fprintf(stderr, "warning: \"%s\" set greater than max temp (%d), "
                                        "using default value\n", key, cfg->max_temp);
                                continue;
                        } else if (STR_STARTS_WITH(key_cmp, "TEMP") && read_val <= 0) {
                                fprintf(stderr, "warning: \"%s\" set less than lowest temp (0), "
                                        "using default value\n", key);
                                continue;
                        } else if (STR_STARTS_WITH(key, "POLLING") && read_val <= 0) {
                                fprintf(stderr, "warning: \"%s\" set less or equal to than zero, "
                                        "using default value\n", key);
                                continue;
                        }

                        if (STR_STARTS_WITH(key, "POLLING")) {
                                cfg->poll_int = read_val;
                        } else if (STR_STARTS_WITH(key, "INC_LOW_TEMP")) {
                                cfg->inc_low_temp = read_val;
                        } else if (STR_STARTS_WITH(key, "INC_HIGH_TEMP")) {
                                cfg->inc_high_temp = read_val;
                        } else if (STR_STARTS_WITH(key, "INC_MAX_TEMP")) {
                                cfg->inc_max_temp = read_val;
                        } else if (STR_STARTS_WITH(key, "DEC_LOW_TEMP")) {
                                cfg->dec_low_temp = read_val;
                        } else if (STR_STARTS_WITH(key, "DEC_HIGH_TEMP")) {
                                cfg->dec_high_temp = read_val;
                        } else if (STR_STARTS_WITH(key, "DEC_MAX_TEMP")) {
                                cfg->dec_max_temp = read_val;
                        } else if (STR_STARTS_WITH(key, "BASE_LVL")) {
                                cfg->base_lvl = read_val;
                        } else if (STR_STARTS_WITH(key, "INC_LOW_LVL")) {
                                cfg->inc_low_lvl = read_val;
                        } else if (STR_STARTS_WITH(key, "INC_HIGH_LVL")) {
                                cfg->inc_high_lvl = read_val;
                        } else if (STR_STARTS_WITH(key, "INC_MAX_LVL")) {
                                cfg->inc_max_lvl = read_val;
                        } else if (STR_STARTS_WITH(key, "DEC_LOW_LVL")) {
                                cfg->dec_low_lvl = read_val;
                        } else if (STR_STARTS_WITH(key, "DEC_HIGH_LVL")) {
                                cfg->dec_high_lvl = read_val;
                        } else if (STR_STARTS_WITH(key, "DEC_MAX_LVL")) {
                                cfg->dec_max_lvl = read_val;
                        }
                }
        }
}

void set_defaults(struct config *cfg)
{
        cfg->inc_low_temp  = INC_LOW_TEMP;
        cfg->inc_high_temp = INC_HIGH_TEMP;
        cfg->inc_max_temp  = INC_MAX_TEMP;

        cfg->inc_low_lvl   = INC_LOW_LEVEL;
        cfg->inc_high_lvl  = INC_HIGH_LEVEL;
        cfg->inc_max_lvl   = INC_MAX_LEVEL;

        cfg->dec_low_lvl   = DEC_LOW_LEVEL;
        cfg->dec_high_lvl  = DEC_HIGH_LEVEL;
        cfg->dec_max_lvl   = DEC_MAX_LEVEL;

        cfg->dec_low_temp  = DEC_LOW_TEMP;
        cfg->dec_high_temp = DEC_HIGH_TEMP;
        cfg->dec_max_temp  = DEC_MAX_TEMP;

        cfg->poll_int      = POLL_INTERVAL;
        cfg->base_lvl      = BASE_LEVEL;
}
