#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "parse.h"
#include "options.h"

#define CMD_MAX 36
#define SET_TEMP 0
#define MAX_TEMP 1

void die(char *msg)
{
        fprintf(stderr, "%s\nfan level set to auto, exiting", msg);
        system("echo level auto > /proc/acpi/ibm/fan");
        exit(EXIT_FAILURE);
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

        if (type == MAX_TEMP)
                fp = fopen("/sys/devices/platform/coretemp.0/temp1_max", "r");
        else
                fp = fopen("/sys/devices/platform/coretemp.0/temp1_input", "r");

        if (!fp) die("error: could not read temperature input");
        fscanf(fp, "%u", &read_temp);
        fclose(fp);

        return (unsigned short)(read_temp / 1000.);
}

int get_level(char* cmd, unsigned short old_temp, unsigned short new_temp,
               struct config *cfg)
{
        unsigned short temp_diff = new_temp - old_temp;
        if (temp_diff == 0)
                return 0;

        if (temp_diff <= 0)
                if (new_temp > cfg->dec_max_temp)
                        sprintf(cmd, "echo level %d > /proc/acpi/ibm/fan", cfg->dec_max_lvl);
                else if (new_temp > cfg->dec_high_temp)
                        sprintf(cmd, "echo level %d > /proc/acpi/ibm/fan", cfg->dec_high_lvl);
                else if (new_temp > cfg->dec_low_temp)
                        sprintf(cmd, "echo level %d > /proc/acpi/ibm/fan", cfg->dec_low_lvl);
                else
                        sprintf(cmd, "echo level %d > /proc/acpi/ibm/fan", cfg->base_lvl);
        else
                if (new_temp <= cfg->inc_low_temp)
                        sprintf(cmd, "echo level %d > /proc/acpi/ibm/fan", cfg->base_lvl);
                else if (new_temp <= cfg->inc_high_temp)
                        sprintf(cmd, "echo level %d > /proc/acpi/ibm/fan", cfg->inc_low_lvl);
                else if (new_temp <= cfg->inc_max_temp)
                        sprintf(cmd, "echo level %d > /proc/acpi/ibm/fan", cfg->inc_high_lvl);
                else
                        sprintf(cmd, "echo level %d > /proc/acpi/ibm/fan", cfg->inc_max_lvl);
        return 1;
}

int main(int argc, char *argv[])
{
        unsigned short old_temp, new_temp;
        char cmd[CMD_MAX];
        struct config cfg;
        int action;

        if (!module_enabled("/proc/acpi/ibm/fan", "r") || !arg_count(argc))
                return 1;

        if ((action = read_command(argc, argv)) != 0) {
                if (action == OPT_HELP) {
                        print_help();
                } else if (action == OPT_VERSION) {
                        print_version();
                } else if (action == OPT_STOP) {
                        die("stopping simpfand");
                } else if (action == OPT_START) {
                        cfg.max_temp = (unsigned short)get_temp(MAX_TEMP);
                        set_defaults(&cfg);
                        parse_config(&cfg);
                        new_temp = get_temp(SET_TEMP);

                        /*
                         * initial call because the first call
                         * in the loop will never happen
                         */
                        get_level(cmd, new_temp-1, new_temp, &cfg);
                        system(cmd);

                        while (1) {
                                old_temp = new_temp;
                                new_temp = get_temp(SET_TEMP);
                                if (get_level(cmd, old_temp, new_temp, &cfg)) system(cmd);
                                sleep(cfg.poll_int);
                         }
                }
        }
        return 0;
}
