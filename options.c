#include <stdio.h>
#include <getopt.h>
#include "options.h"

int read_command(int argc, char *argv[])
{
	int opt, option_index = 0;

	static struct option opts[] =
	{
		{"help", no_argument,	    0, 'h'},
		{"version", no_argument,    0, 'v'},
		{"start", no_argument,      0, 's'},
		{"stop", no_argument,       0, 't'},
		{0, 0, 0, 0}
	};

	while ((opt = getopt_long(argc, argv, "shvt", opts, &option_index)) != -1) 
        {
		switch (opt)
		{
			case 'h':
				return OPT_HELP;
			case 'v':
				return OPT_VERSION;
			case 's':
				return OPT_START;
			case 't':
				return OPT_STOP;
			default:
				return 0;
		}
	}
	return 0;
}

int module_enabled(char *fan_path, char *mode)
{
	int enabled = fopen("/proc/acpi/ibm/fan", "r") != NULL;
        if (!enabled) fprintf(stderr, "thinkpad_acpi fan_control option is disabled! Exiting\n");
        return enabled;
}

int arg_count(int argc)
{
        int enough = argc > 1;
        if (!enough) fprintf(stderr, "error: requires argument (-h for help)\n");
        return enough;
}
