#define _GNU_SOURCE
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "parse.h"
#include "options.h"

#define SET_TEMP 0
#define MAX_TEMP 1
#define LVL_LEN 8

void die(char *msg, int exit_code)
{
        /* only display msg if fan really got set to auto */
        if (system("echo level auto > /proc/acpi/ibm/fan") != 256)
                fprintf(stderr, "%s\nfan level set to auto, exiting\n", msg);
        else
                fprintf(stderr, "%s\nwarning: could not verify fan state\n", msg);
        exit(exit_code);
}

void signal_handler()
{
        die("caught SIGINT", 1);
}

void print_version(void)
{
#ifdef SIMPFAND_VERSION
        printf("simpfand: %s\n", SIMPFAND_VERSION);
#endif
}

void print_help(void)
{
        print_version();
        printf("Usage: simpfand <action>\n\n"

               " Actions:\n"
               "  -v, --version         display version\n"
               "  -h, --help            display help\n"
               "  -s, --start           starts simpfand\n"

               " NOTE: running --start manually is not recommended!\n");
}

unsigned short get_temp(int type)
{
        FILE *fp;
        unsigned int read_temp;

        if (type == SET_TEMP)
                fp = fopen("/sys/devices/platform/coretemp.0/temp1_input", "r");
        else
                fp = fopen("/sys/devices/platform/coretemp.0/temp1_max", "r");

        // my temperatures on my t420 are being read somewhere else now...
        if (!fp) {
                if (type == SET_TEMP)
                        fp = fopen("/sys/devices/platform/coretemp.0/hwmon/hwmon1/temp1_input", "r");
                else
                        fp = fopen("/sys/devices/platform/coretemp.0/hwmon/hwmon1/temp1_max", "r");
        }

        if (!fp) die("error: could not read temperature input", EXIT_FAILURE);
        fscanf(fp, "%u", &read_temp);
        fclose(fp);

        return (unsigned short)(read_temp / 1000.);
}

unsigned short get_level(char *level_cmd, unsigned short old_temp,
                         unsigned short new_temp, unsigned short prev_lvl,
                         struct config *cfg)
{
        short temp_diff = new_temp - old_temp;
        unsigned short level = prev_lvl;

        if (temp_diff > 0 || prev_lvl == 100) {
                if (new_temp <= cfg->inc_low_temp)
                        level = cfg->base_lvl;
                else if (new_temp <= cfg->inc_high_temp)
                        level = cfg->inc_low_lvl;
                else if (new_temp <= cfg->inc_max_temp)
                        level = cfg->inc_high_lvl;
                else
                        level = cfg->inc_max_lvl;
        } else if (temp_diff < 0) {
                if (new_temp > cfg->dec_max_temp)
                        level = cfg->dec_max_lvl;
                else if (new_temp > cfg->dec_high_temp)
                        level = cfg->dec_high_lvl;
                else if (new_temp > cfg->dec_low_temp)
                        level = cfg->dec_low_lvl;
                else
                        level = cfg->base_lvl;
        }

        snprintf(level_cmd, LVL_LEN, "level %d", level);
        return level;
}

void fan_control(const char *fan_path)
{
        unsigned short old_temp, new_temp;
        unsigned short curr_lvl, prev_lvl;
        struct config cfg;
        char lvl[LVL_LEN];
        int file;

        cfg.max_temp = get_temp(MAX_TEMP);
        set_defaults(&cfg);
        parse_config(&cfg);
        new_temp = get_temp(SET_TEMP);
        curr_lvl = 100; /* need to initialize it to something invalid to start it */

        struct sigaction sig_int_handler;
        sig_int_handler.sa_handler = signal_handler;
        sigemptyset(&sig_int_handler.sa_mask);
        sig_int_handler.sa_flags = 0;
        sigaction(SIGINT, &sig_int_handler, NULL);
        sigaction(SIGTERM, &sig_int_handler, NULL);

        while (1) {

                old_temp = new_temp;
                new_temp = get_temp(SET_TEMP);

                prev_lvl = curr_lvl;
                curr_lvl = get_level(lvl, old_temp, new_temp, prev_lvl, &cfg);
                fprintf(stderr, "level: %d -> %d\n", prev_lvl, curr_lvl);

                if (prev_lvl != curr_lvl || prev_lvl == 100) {
                        if ((file = open(fan_path, O_WRONLY)) == -1)
                                die("error: could not open fan file", EXIT_FAILURE);
                        if ((write(file, lvl, strlen(lvl))) == -1)
                                die("error: could not write to fan file", EXIT_FAILURE);

                        close(file);
                }


                sleep(cfg.poll_int);
         }
}

int main(int argc, char *argv[])
{
        char *fan_path = "/proc/acpi/ibm/fan";
        int action = 0;

        if (!arg_count(argc))
                return EXIT_FAILURE;

        if ((action = read_command(argc, argv)) != 0) {
                if (action == OPT_HELP) {
                        print_help();
                } else if (action == OPT_VERSION) {
                        print_version();
                } else if (action == OPT_START) {
                        if (module_enabled(fan_path, "r")) {
                                fprintf(stderr, "fan control started\n");
                                fan_control(fan_path);
                        } else {
                                return EXIT_FAILURE;
                        }
                }
        } else {
                fprintf(stderr, "unknown option: %s\n", argv[1]);
        }
        return EXIT_SUCCESS;
}
