#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "parse.h"

#define CMD_MAX	35

/* RPM's for ThinkPad T420
 *  Level 1: 2000
 *  Level 2: 3200
 *  Level 3: 3400
 *  Level 4: 3600
 *  Level 5: 3900
 *  Level 6: 4500
 *  Level 7: 5800
 */

/* Returns average CPU temp in degrees (ceiling) */
unsigned short get_temp()
{
	FILE *file;
	unsigned short temp; 
	unsigned int t0, t1;

	file=fopen("/sys/devices/platform/coretemp.0/temp2_input", "r");
	fscanf(file, "%u", &t0);
	fclose(file);

	file=fopen("/sys/devices/platform/coretemp.0/temp3_input", "r");
	fscanf(file, "%u", &t1);
	fclose(file);

	temp = (unsigned short)(ceil((float)(t0 + t1) / 2000.));
	return temp;
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

int main(int argc, char const *argv[])
{
	unsigned short old_temp, new_temp;
	char cmd[CMD_MAX];
	struct config cfg; 
	int pid_file;

	if ((pid_file = open("var/run/simpfand.pid", O_CREAT | O_EXCL)) == -1) {
			printf("simpfand: another instance running!"
			       "Do not manually run\n");
			return 1;
	}

	set_defaults(&cfg);
	parse_config(&cfg);

	new_temp = get_temp();
	if (new_temp == 0) {
		fprintf(stderr, "simpfand: cannot properly read temperature!"
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

	return 0;
}
