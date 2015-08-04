/*
 * config.c
 *
 * Created on: 2014-9-10
 * Author: zhuli
 */
#include "togo.h"
#include "togo_load.h"

TOGO_CONFIG togo_config_arr[TOGO_CONFIG_ARR_LEN];
int togo_config_len;

BOOL togo_read_config(u_char *config_file_path)
{
	togo_log(INFO, "Try to read Config file:%s", config_file_path);
	u_char buf[TOGO_CONFIG_STRING_LINE];
	FILE * fp;

	fp = fopen(config_file_path, "r+");
	if (fp == NULL) {
		togo_log(INFO, "Can not find config file, "
				"please check your configuration file "
				"path is correct. path:%s", config_file_path);
		return FALSE;
	}

	int i = 0;
	while (!feof(fp)) {

		if (fgets(buf, TOGO_CONFIG_STRING_LINE, fp)) {

			u_char *tmp = togo_trim(buf);
			if (*tmp == '#') {
				continue;
			}

			u_char *key = togo_trim(strtok(tmp, "="));
			u_char *value = togo_trim(strtok(NULL, "="));
			if (key && value) {
				//Need to free the memory
				togo_config_arr[i].key = strdup(key);
				togo_config_arr[i].value = strdup(value);
				i++;
			}

		} else {
			break;
		}
	}

	togo_config_len = i;
	fclose(fp);

	return TRUE;
}

void togo_clear_config()
{
	int i;
	for (i = 0; i < togo_config_len; i++) {
		free(togo_config_arr[i].key); //Free the memory
		free(togo_config_arr[i].value);
	}
	togo_config_len = 0;
}

u_char * togo_get_config(u_char * key)
{
	int i;
	u_char *value = NULL;

	for (i = 0; i < togo_config_len; i++) {
		if (strcmp(togo_config_arr[i].key, key) == 0) {
			value = togo_config_arr[i].value;
			break;
		}
	}
	return value;
}

int togo_get_config_to_number(u_char *key, int default_val)
{
	u_char *str = togo_get_config(key);
	if (str != NULL) {
		int num = atoi(str);
		return num;
	} else {
		return default_val;
	}
}

BOOL togo_set_config(u_char *key, u_char *value)
{
	int i, j;
	j = -1;

	for (i = 0; i < togo_config_len; i++) {

		if (strcmp(togo_config_arr[i].key, key) == 0) {
			j = i;
			break;
		}
	}

	if (j != -1) {
		togo_config_arr[j].value = value;
	} else {
		if (togo_config_len < TOGO_CONFIG_ARR_LEN) {
			togo_config_arr[togo_config_len].key = key;
			togo_config_arr[togo_config_len].value = value;
			togo_config_len++;
		} else {
			return FALSE;
		}
	}
	return TRUE;
}
