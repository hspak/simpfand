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

#define LVL_LEN 8
#define INIT_GARBAGE 100

#define INC 0
#define DEC 1

static char* temp_path;
static char* max_temp_path;

void die(char *msg, int exit_code)
{
        /* only display msg if fan really got set to auto */
        if (system("echo level auto > /proc/acpi/ibm/fan") != 256) {
                fprintf(stderr, "%s\nfan level set to auto, exiting\n", msg);
        } else {
                fprintf(stderr, "%s\nwarning: could not verify fan state\n", msg);
                exit_code = EXIT_FAILURE;
        }
        exit(exit_code);
}

void signal_handler(int sig)
{
        char msg[16];
        snprintf(msg, sizeof(msg), "caught signal %d\n", sig);
        die(msg, EXIT_SUCCESS);
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

               " NOTE: running --start manually is not recommended\n");
}

unsigned short get_temp()
{
        FILE *fp;
        unsigned int read_temp;

        fp = fopen(temp_path, "r");
        if (!fp) die("error: could not read temperature input", EXIT_FAILURE);
        fscanf(fp, "%u", &read_temp);
        fclose(fp);

        return (unsigned short)(read_temp / 1000.);
}

unsigned short get_max_temp()
{
        FILE *fp;
        unsigned int read_temp;

        fp = fopen(max_temp_path, "r");
        if (!fp) die("error: could not read temperature input", EXIT_FAILURE);
        fscanf(fp, "%u", &read_temp);
        fclose(fp);

        return (unsigned short)(read_temp / 1000.);
}


unsigned short get_level(unsigned short curr_temp, int dir, struct config *cfg)
{
        unsigned level = INIT_GARBAGE;

        // might want to consider changing this if chain into arrays...might be worth it
        // 2 * 128*2 = 512 bytes to allow temp of 0-127 with should be plenty, x2 for dec/inc
        if (dir == INC) {
                if (curr_temp <= cfg->inc_low_temp)
                        level = cfg->base_lvl;
                else if (curr_temp <= cfg->inc_high_temp)
                        level = cfg->inc_low_lvl;
                else if (curr_temp <= cfg->inc_max_temp)
                        level = cfg->inc_high_lvl;
                else
                        level = cfg->inc_max_lvl;
        }

        if (dir == DEC) {
                if (curr_temp > cfg->dec_max_temp)
                        level = cfg->dec_max_lvl;
                else if (curr_temp > cfg->dec_high_temp)
                        level = cfg->dec_high_lvl;
                else if (curr_temp > cfg->dec_low_temp)
                        level = cfg->dec_low_lvl;
                else
                        level = cfg->base_lvl;
        }
        return level;
}

int detect_change(unsigned short curr_temp, unsigned short prev_lvl, int dir, struct config *cfg)
{
        unsigned level = get_level(curr_temp, dir, cfg);
        int change = 0;

        if (level == INIT_GARBAGE)
                die("error: get_level logic broken", 1);
        else if (level != prev_lvl)
                change = 1;

        return change;
}

void update_fan_level(const char *fan_path, unsigned int curr_lvl)
{
        int file;
        char lvl[LVL_LEN];

        snprintf(lvl, LVL_LEN, "level %d", curr_lvl);
        if ((file = open(fan_path, O_WRONLY)) == -1)
                die("error: could not open fan file", EXIT_FAILURE);
        if ((write(file, lvl, strlen(lvl))) == -1)
                die("error: could not write to fan file", EXIT_FAILURE);

        close(file);
}

void fan_control(const char *fan_path)
{
        unsigned short prev_temp, curr_temp;
        unsigned short curr_lvl, prev_lvl;
        struct config cfg;
        int change = 0;
        int diff;
        int dir;

        cfg.max_temp = get_max_temp();
        set_defaults(&cfg);
        parse_config(&cfg);

        dir = INC;
        curr_temp = get_temp();
        curr_lvl = INIT_GARBAGE; /* need to initialize it to something invalid to start it */

        // catch signals
        struct sigaction sig_int_handler;
        sig_int_handler.sa_handler = signal_handler;
        sigemptyset(&sig_int_handler.sa_mask);
        sig_int_handler.sa_flags = 0;
        sigaction(SIGINT, &sig_int_handler, NULL);
        sigaction(SIGTERM, &sig_int_handler, NULL);

        while (1) {
                prev_temp = curr_temp;
                curr_temp = get_temp();
                prev_lvl = curr_lvl;

                change = detect_change(curr_temp, prev_lvl, dir, &cfg);
                if (change || prev_lvl == INIT_GARBAGE) {
                        diff = curr_temp - prev_temp;
                        if (diff > 0)
                                dir = INC;
                        else if (diff < 0)
                                dir = DEC;

                        change = 0;
                        curr_lvl = get_level(curr_temp, dir, &cfg);
                        update_fan_level(fan_path, curr_lvl);
                }
                fprintf(stderr, "lvl: %d(%d) -> %d(%d)\n", prev_lvl, prev_temp, curr_lvl, curr_temp);
                sleep(cfg.poll_int);
         }
}

void set_path()
{
        FILE *fp = NULL;
        char* paths[] = {
                "/sys/devices/platform/coretemp.0/temp1_input",
                "/sys/devices/platform/coretemp.0/temp2_input",
                "/sys/devices/platform/coretemp.0/temp3_input",
                "/sys/devices/platform/coretemp.0/hwmon/hwmon1/temp1_input",
                "/sys/devices/platform/coretemp.0/hwmon/hwmon1/temp2_input",
                "/sys/devices/platform/coretemp.0/hwmon/hwmon1/temp3_input",
                "/sys/devices/platform/coretemp.0/hwmon/hwmon2/temp1_input",
                "/sys/devices/platform/coretemp.0/hwmon/hwmon2/temp2_input",
                "/sys/devices/platform/coretemp.0/hwmon/hwmon2/temp3_input",
                "/sys/devices/platform/coretemp.0/hwmon/hwmon3/temp1_input",
                "/sys/devices/platform/coretemp.0/hwmon/hwmon3/temp2_input",
                "/sys/devices/platform/coretemp.0/hwmon/hwmon3/temp3_input",
        };
        for(int i = 0; i < sizeof(paths); i++) {
                fp = fopen(paths[i], "r");
                if (fp != NULL) {
                        temp_path = paths[i];
                        fprintf(stderr, "found temp path: %s\n", temp_path);
                        fclose(fp);
                        return;
                }
        }
        die("error: could not find temp path\n", 1);
}

void set_max_path()
{
        FILE *fp = NULL;
        char* paths[] = {
                "/sys/devices/platform/coretemp.0/temp1_max",
                "/sys/devices/platform/coretemp.0/temp2_max",
                "/sys/devices/platform/coretemp.0/temp3_max",
                "/sys/devices/platform/coretemp.0/hwmon/hwmon1/temp1_max",
                "/sys/devices/platform/coretemp.0/hwmon/hwmon1/temp2_max",
                "/sys/devices/platform/coretemp.0/hwmon/hwmon1/temp3_max",
                "/sys/devices/platform/coretemp.0/hwmon/hwmon2/temp1_max",
                "/sys/devices/platform/coretemp.0/hwmon/hwmon2/temp2_max",
                "/sys/devices/platform/coretemp.0/hwmon/hwmon2/temp3_max",
                "/sys/devices/platform/coretemp.0/hwmon/hwmon3/temp1_max",
                "/sys/devices/platform/coretemp.0/hwmon/hwmon3/temp2_max",
                "/sys/devices/platform/coretemp.0/hwmon/hwmon3/temp3_max",
        };
        for(int i = 0; i < sizeof(paths); i++) {
                fp = fopen(paths[i], "r");
                if (fp != NULL) {
                        max_temp_path = paths[i];
                        fprintf(stderr, "found max temp path: %s\n", temp_path);
                        fclose(fp);
                        return;
                }
        }
        die("error: could not find max temp path\n", 1);
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
                                set_path();
                                set_max_path();
                                fan_control(fan_path);
                        } else {
                                // error msg from module_enabled
                                return EXIT_FAILURE;
                        }
                }
        } else {
                fprintf(stderr, "unknown option: %s\n", argv[1]);
        }
        return EXIT_SUCCESS;
}
