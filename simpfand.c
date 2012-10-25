#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "parse.h"
#include "options.h"

#define CMD_MAX	35

/* Returns average CPU temp in degrees (ceiling) */
unsigned short get_temp()
{
	FILE *file;
	unsigned short temp;
	unsigned int t0, t1;

	file=fopen("/sys/devices/platform/coretemp.0/temp2_input", "r");
	if (!file) return 0;
	fscanf(file, "%u", &t0);
	fclose(file);

	file=fopen("/sys/devices/platform/coretemp.0/temp3_input", "r");
	if (!file) return 0;
	fscanf(file, "%u", &t1);
	fclose(file);

	temp = (unsigned short)(ceil((float)(t0 + t1) / 2000.));
	return temp;
}

unsigned short get_max_temp()
{
	FILE *fp;
	unsigned max_temp;

	fp = fopen("/sys/devices/platform/coretemp.0/temp2_max", "r");
	if (!fp) return 0;
	fscanf(fp, "%u", &max_temp);
	fclose(fp);

	max_temp = (unsigned short)(max_temp / 1000.);
	return max_temp;
}

void get_level(char* cmd, unsigned short old_temp, unsigned short new_temp,
               struct config *cfg)
{
	unsigned short temp_diff = new_temp - old_temp;

	if (temp_diff <= cfg->dec_thres)
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
	cfg->dec_thres     = DEC_THRESH;
	cfg->base_lvl      = BASE_LEVEL;
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
	       "  -v, --version		display version\n"
	       "  -h, --help		display help\n"
	       "  -s, --start 		starts daemon\n"
	       "  -t, --stop 		stops daemon\n\n"

	       " NOTE: running --start and --stop manually is not recommended!\n");
}

int main(int argc, char *argv[])
{
	unsigned short old_temp, new_temp;
	char cmd[CMD_MAX];
	struct config cfg;
	int action;
	FILE *fp;

	fp = fopen("/proc/acpi/ibm/fan", "r");
	if (!fp) {
		fprintf(stderr, "thinkpad_acpi fan_control option is disabled! Exiting\n");
		return 1;
	}
	fclose(fp);

	if (argc < 2) {
		fprintf(stderr, "error: requires argument (-h for help)\n");
		return 1;
	}

	if ((action = read_command(argc, argv)) != 0) {
		if (action == OPT_HELP) {
			print_help();
		} else if (action == OPT_VERSION) {
			print_version();
		} else if (action == OPT_STOP) {
			printf("Stopping simpfand. Fan level set to auto.\n");
			system("echo level auto > /proc/acpi/ibm/fan");
		} else if (action == OPT_START) {
			if (geteuid() != 0) {
				/* try to preven users from directly starting program*/
				fprintf(stderr, "permission denied: simpfand\n");
				return 1;
			}

			cfg.max_temp = get_max_temp();
			set_defaults(&cfg);
			parse_config(&cfg);

			new_temp = get_temp();
			if (new_temp == 0 || cfg.max_temp == 0) {
				fprintf(stderr, "error: cannot properly read temperature! "
                                                "Fan set to auto. Exiting.\n");
				system("echo level auto > /proc/acpi/ibm/fan");
				return 1;
			}

			while (1) {
				old_temp = new_temp;
				new_temp = get_temp();
				get_level(cmd, old_temp, new_temp, &cfg);
				system(cmd);
			 	sleep(cfg.poll_int);
			 }
		}
	}

	return 0;
}
