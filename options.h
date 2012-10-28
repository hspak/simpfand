#ifndef OPTIONS_H
#define OPTIONS_H

#define OPT_HELP  1
#define OPT_VERSION   2
#define OPT_START   3
#define OPT_STOP  4

int read_command(int argc, char *argv[]);
int module_enabled(char *path, char *mode);
int arg_count(int argc);

#endif
