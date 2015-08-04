/*
 * config.h
 * Created on: 2014-9-10
 * Author: zhuli
 */
#ifndef TOGO_CONFIG_H
#define TOGO_CONFIG_H

#define TOGO_CONFIG_STRING_LINE 1024
#define TOGO_CONFIG_ARR_LEN 1024

typedef struct togo_config_s TOGO_CONFIG;

struct togo_config_s {
	u_char *key;
	u_char *value;
};

BOOL togo_read_config(u_char *config_file_path);
void togo_clear_config(void);
u_char *togo_get_config(u_char *key);
int togo_get_config_to_number(u_char *key, int default_val);
BOOL togo_set_config(u_char *key, u_char *value);

#endif
