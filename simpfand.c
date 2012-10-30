#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "parse.h"
#include "options.h"

#define SET_TEMP 0
#define MAX_TEMP 1
#define LVL_LEN 8

void die(char *msg, int exit_code)
{
        fprintf(stderr, "%s\nfan level set to auto, exiting", msg);
        system("echo level auto > /proc/acpi/ibm/fan");
        exit(exit_code);
}

void print_version(void)
{
        printf("simpfand: %s\n", SIMPFAND_VERSION);
}

void print_help(void)
{
        print_version();
        printf("Usage: simpfand <action>\n\n"

               " Actions:\n"
               "  -v, --version         display version\n"
               "  -h, --help            display help\n"
               "  -s, --start           starts daemon\n"
               "  -t, --stop            stops daemon\n\n"

               " NOTE: running --start and --stop manually is not recommended!\n");
}

unsigned short get_temp(int type)
{
        FILE *fp;
        unsigned int read_temp;

        if (type == SET_TEMP)
                fp = fopen("/sys/devices/platform/coretemp.0/temp1_input", "r");
        else
                fp = fopen("/sys/devices/platform/coretemp.0/temp1_max", "r");

        if (!fp) die("error: could not read temperature input", EXIT_FAILURE);
        fscanf(fp, "%u", &read_temp);
        fclose(fp);

        return (unsigned short)(read_temp / 1000.);
}

void get_level(char *level, unsigned short old_temp, unsigned short new_temp,
               struct config *cfg)
{
        unsigned short temp_diff = new_temp - old_temp;

        if (temp_diff <= 0)
                if (new_temp > cfg->dec_max_temp)
                        snprintf(level, LVL_LEN, "level %d", cfg->dec_max_lvl);
                else if (new_temp > cfg->dec_high_temp)
                        snprintf(level, LVL_LEN, "level %d", cfg->dec_high_lvl);
                else if (new_temp > cfg->dec_low_temp)
                        snprintf(level, LVL_LEN, "level %d", cfg->dec_low_lvl);
                else
                        snprintf(level, LVL_LEN, "level %d", cfg->base_lvl);
        else
                if (new_temp <= cfg->inc_low_temp)
                        snprintf(level, LVL_LEN, "level %d", cfg->base_lvl);
                else if (new_temp <= cfg->inc_high_temp)
                        snprintf(level, LVL_LEN, "level %d", cfg->inc_low_lvl);
                else if (new_temp <= cfg->inc_max_temp)
                        snprintf(level, LVL_LEN, "level %d", cfg->inc_high_lvl);
                else
                        snprintf(level, LVL_LEN, "level %d", cfg->inc_max_lvl); 
}

int main(int argc, char *argv[])
{
        unsigned short old_temp, new_temp;
        char *fan_path = "/proc/acpi/ibm/fan";
        char level[LVL_LEN];
        struct config cfg;
        int action;
        int file;

        if (!module_enabled(fan_path, "r") || !arg_count(argc))
                return EXIT_FAILURE;

        if ((action = read_command(argc, argv)) != 0) {
                if (action == OPT_HELP) {
                        print_help();
                } else if (action == OPT_VERSION) {
                        print_version();
                } else if (action == OPT_STOP) {       
                        die("stopping simpfand", EXIT_SUCCESS);
                } else if (action == OPT_START) {
                        cfg.max_temp = get_temp(MAX_TEMP);
                        set_defaults(&cfg);
                        parse_config(&cfg);
                        new_temp = get_temp(SET_TEMP);

                        while (1) {
                                old_temp = new_temp;
                                new_temp = get_temp(SET_TEMP);
                                get_level(level, old_temp, new_temp, &cfg);

                                if((file = open(fan_path, O_WRONLY)) == -1)
                                        die("could not open fan file", EXIT_FAILURE);
                                if((write(file, level, strlen(level))) == -1)
                                        die("could not write to fan file", EXIT_FAILURE);

                                close(file);
                                sleep(cfg.poll_int);
                         }
                }
        }
        return EXIT_SUCCESS;
}
