// Author: Artyom Liu 
// Description: The main part of the programme. 

// for socket networking 
#include <sys/socket.h>
#include <arpa/inet.h>

// linux-specified 
#include <unistd.h>

// generic 
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <stdlib.h>		// exit()

#include "fifo.h"
#include "http.h"
#include "config.h"
#include "mthread.h"

/* the global tasklist */
static struct fifo_t *tasklist = NULL;

// Socket stuff
static int socket_fd = 0;

void exit_handler(int sig) {
	printf("Exiting Melantha...\n");

	// shutdown all threads 
	mthread_end();
	printf("Close threads\n");

	// close listen socket 
	close(socket_fd);
	printf("Close Socket\n");

	// destruct tasklist 
	fifo_destruct(tasklist);
	printf("Destruct tasklist\n");

	// leaving programme 
	printf("Thank you for your using :)\n");
	exit(0);
}


int main(int argc, char *argv[]) {
	int i;

	static char const *version = "0.5";
	static char const *help_msg = 
			"Usage: %s [Options]\n"
			"Options:\n"
			"\t-f filename    specify a config file\n"
			"\t-r root_path   select root path\n"
			"\t-p port        select port\n"
			"\t-h             display this message\n"
			"\n";

	printf("Welcome to use Melantha!\n"
			"Version %s\n", version);

	{	// code block for load configurations 
		FILE *fp;
		char *tmp;
		int port;

		// init configurations 
		fp = NULL;
		tmp = NULL;
		port = 0;

#define _str_equal(str1, str2) \
	(0 == strcmp(str1, str2))

		for (i = 1; i < argc; i ++) {
			if (_str_equal(argv[i], "-h")) {
				printf(help_msg, argv[0]);
				return 0;
			}
			else if (_str_equal(argv[i], "-f")) 
				fp = fopen(argv[i + 1], "r");
			else if (_str_equal(argv[i], "-r"))
				tmp = argv[i + 1];
			else if (_str_equal(argv[i], "-p")) 
				port = atoi(argv[i + 1]);
		}

#undef _str_equal

		config_load(fp);
		if (NULL != fp) fclose(fp);

		if (NULL != tmp) strcpy(config_current.root, tmp);
		if (0 != port) config_current.port = port;
		printf("Server configured!\n"
				"root path: %-s\n"
				"port:      %-d\n", 
				config_current.root, config_current.port);
	}	// code block ends

	// init tasklist 
	tasklist = fifo_create();
	if (NULL == tasklist) {
		fprintf(stderr, "\e[31merror\e[0m: main(): failed to create tasklist\n");
		exit_handler(-1);
	}
	printf("tasklist created\n");

	// init Socket 
	struct sockaddr_in server;

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == socket_fd) {
		fprintf(stderr, "\e[31merror\e[0m: main(): failed to init sock\n");
		exit_handler(-1);
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(config_current.port);
	if (bind(socket_fd, (struct sockaddr*)&server, sizeof(server)) < 0) {
		fprintf(stderr, "\e[31merror\e[0m: main(): bind failed\n");
		exit_handler(-1);
	}
	listen(socket_fd, 3);
	printf("Listener Socket on\n");

	// init multi-threading 
	mthread_init(tasklist);

	// register signal handler
	signal(SIGINT, exit_handler);

	// start service 

	printf("Service Started...\n\n");

	mthread_enter(socket_fd);

	// the code should not reach here 
	perror("\e[31merror\e[0m: unexpected error");
	exit_handler(-1);

	return 0;
}
