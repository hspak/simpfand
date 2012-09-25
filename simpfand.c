/*
 *  simpfand.c - Simple Thinkpad Fan Daemon
 *  (based on Allan McRae's MacBook Pro Fan Daemon)
 *  Copyright (C) 2012  Hong Shick Pak <hongshick.pak@gmail.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  20120924 - v1.0
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define min(a,b) a < b ? a : b
#define max(a,b) a > b ? a : b

#define INC_LOW_TEMP  	55
#define INC_HIGH_TEMP 	65
#define INC_MAX_TEMP  	82

#define DEC_LOW_TEMP  	50
#define DEC_HIGH_TEMP 	60
#define DEC_MAX_TEMP  	77

#define POLL_INTERVAL 	10

/* RPM's
 *  Level 1: 2000
 *  Level 2: 3200
 *  Level 3: 3400
 *  Level 4: 3600
 *  Level 5: 3900
 *  Level 6: 4500
 *  Level 7: 4500
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

int main(int argc, char const *argv[])
{
	unsigned short old_temp, new_temp;
	short temp_diff;
	char cmd[35];

	new_temp = get_temp();

	while (1) {
		old_temp = new_temp;
		new_temp = get_temp();

		temp_diff = new_temp - old_temp;

		if (temp_diff < 2)
			if (new_temp > DEC_MAX_TEMP)
				sprintf(cmd, "echo level %d > /proc/acpi/ibm/fan", 7);
			else if (new_temp > DEC_HIGH_TEMP)
				sprintf(cmd, "echo level %d > /proc/acpi/ibm/fan", 5);
			else if (new_temp > DEC_LOW_TEMP)
				sprintf(cmd, "echo level %d > /proc/acpi/ibm/fan", 4);
			else
				sprintf(cmd, "echo level %d > /proc/acpi/ibm/fan", 1);
		else
			if (new_temp <= INC_LOW_TEMP)
				sprintf(cmd, "echo level %d > /proc/acpi/ibm/fan", 1);
			else if (new_temp <= INC_HIGH_TEMP)
				sprintf(cmd, "echo level %d > /proc/acpi/ibm/fan", 2);
			else if (new_temp <= INC_MAX_TEMP)
				sprintf(cmd, "echo level %d > /proc/acpi/ibm/fan", 4);
			else
				sprintf(cmd, "echo level %d > /proc/acpi/ibm/fan", 7);

		// printf("temp = %u\t%s\n", new_temp, cmd);
		system(cmd);
		sleep(POLL_INTERVAL);
	}

	return 0;
}
