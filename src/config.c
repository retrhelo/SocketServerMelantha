// Author: Artyom Liu 

#pragma GCC dependency "../inc/config.h"

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#include "config.h"

static int readline(FILE *fp, char *buf, int n) {
	int i = 0;
	char tmp;

	while (i < n) {
		tmp = fgetc(fp);
		if (EOF == tmp || '\n' == tmp) {
			buf[i] = '\0';
			break;
		}
		buf[i++] = tmp;
	}
	buf[n - 1] = '\0';

	return 0 != i || EOF != tmp;
}

static char const *key_root = "root";
static char const *key_port = "port";

#define _str_equal(str1, str2, size) \
	(0 == strncmp(str1, str2, size))

int config_load(FILE *fp) {
	char buf[CONFIG_ROOT_SIZE];
	char *tmp;
	int i;

	int const key_root_len = strlen(key_root);
	int const key_port_len = strlen(key_port);

	// load the default 
	strcpy(config_current.root, CONFIG_ROOT);
	config_current.port = CONFIG_PORT;

	if (fp) {
		while (readline(fp, buf, CONFIG_ROOT_SIZE)) {
			if (_str_equal(buf, key_root, key_root_len)) {
				// load 'root' value 
				tmp = buf + key_root_len;
				i = 0;
				while ('=' != *tmp++);	// escape to the value 
				while (isspace(*tmp)) tmp ++;
				while (!isspace(*tmp) && i < CONFIG_ROOT_SIZE) 
					config_current.root[i++] = *tmp++;
				config_current.root[CONFIG_ROOT_SIZE == i ? 
						CONFIG_ROOT_SIZE - 1: i] = '\0';
			}
			else if (_str_equal(buf, key_port, key_port_len)) {
				// load 'port' value 
				tmp = buf + key_port_len;
				while ('=' != *tmp++);	// escape to the value 
				while (isspace(*tmp)) tmp ++;

				config_current.port = atoi(tmp);
			}
		}
	}

	return NULL == fp;
}
