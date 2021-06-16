// Author: Artyom Liu 
// Description: Configuration loading.

#ifndef __CONFIG_H
#define __CONFIG_H

#include <stdio.h>

#define CONFIG_FILENAME 	"configuration"

#define CONFIG_ROOT_SIZE 	256

struct config_t {
	char root[CONFIG_ROOT_SIZE];
	int port;
};

extern struct config_t config_current;

#define CONFIG_ROOT 	"/"		// generally this won't work
#define CONFIG_PORT 	8080

/* 
	Load configuration from given file. 
	Return
		0 - load the default
		non-zero - load from the given file 
*/
int config_load(FILE *fp);

#endif 
