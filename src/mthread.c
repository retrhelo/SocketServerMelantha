// Author: Artyom Liu 

#include <pthread.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>

#include "fifo.h"
#include "config.h"
#include "mthread.h"

/* tasklist */
static struct fifo_t *tasklist;
static pthread_mutex_t task_lock;

#define _task_enter_cs() \
	pthread_mutex_lock(&task_lock) 
#define _task_leave_cs() \
	pthread_mutex_unlock(&task_lock) 

// handlers 
static void *sender_handler(void *args);
static void *receiver_handler(void *args);

// sender
#define SENDER_NUM 	3
static int mthread_sigrun = 1;

void mthread_init(struct fifo_t *fifo) {
	pthread_t new_thread;
	int i;

	// init tasklist
	pthread_mutex_init(&task_lock, NULL);
	tasklist = fifo;
	// init mthread_sigrun 
	mthread_sigrun = 1;
	for (i = 0; i < SENDER_NUM; i ++) {
		pthread_create(&new_thread, NULL, sender_handler, NULL);
	}
}

void mthread_enter(int socket_fd) {
	pthread_t thr;
	int new_socket;
	struct sockaddr_in client;
	size_t n = sizeof(client);

	while ((new_socket = accept(socket_fd, (struct sockaddr*)&client, &n)) >= 0) {
		if (pthread_create(&thr, NULL, receiver_handler, &new_socket) < 0) 
			perror("mthread_enter(): failed to create thread");
		else while (new_socket) 	// wait for receiver_handler to read new_socket 
			;
	}
}

void mthread_end(void) {
	int i;

	mthread_sigrun = 0;
}

static enum http_response_t get_type(char const *filename) {
	enum http_response_t ret;

	while (*filename && '.' != *filename) 
		filename ++;
	if ('\0' == *filename) return TYPE_ERROR;

	switch (tolower(filename[1])) {		// this may not be a good way 
	case 'h': ret = HTML; break;
	case 'j': ret = JPG; break;
	case 'p': ret = PNG; break;
	case 'g': ret = GIF; break;
	case 'c': ret = CSS; break;
	default: ret = TYPE_ERROR;
	}

	return ret;
}

static void *receiver_handler(void *args) {
	int socket_fd = *(int*)args;
	*(int*)args = 0;
	FILE *fp;
	char filename[64];
	char fullname[256];
	enum http_response_t rt;

	while (http_resolve(socket_fd, filename, 64) >= 0) {
		// for debug
		printf("receiver_handler(): %lu: %s\n", pthread_self(), 
				filename);
		strcpy(fullname, config_current.root);
		strcat(fullname, filename);
		if (0 == strcmp(filename, "/")) 
			strcat(fullname, "index.html");

		fp = fopen(fullname, "rb");
		rt = (NULL == fp ? TYPE_ERROR : get_type(filename));

		_task_enter_cs();
		fifo_enqueue(tasklist, socket_fd, fp, rt);
		_task_leave_cs();
	}
	close(socket_fd);
	pthread_detach(pthread_self());
}

static void *sender_handler(void *args) {
	FILE *fp;
	int socket_fd;
	void *ret;
	enum http_response_t ftype;

	while (mthread_sigrun) {
		_task_enter_cs();

		if (ret = get_head(tasklist, &socket_fd, &fp, &ftype)) 
			fifo_dequeue(tasklist);

		_task_leave_cs();

		if (NULL == ret) {
			//sleep(0.005);		// sleep for 5ms 
			continue;
		}

		http_send(socket_fd, ftype, fp);
		if (fp) fclose(fp);
	}

	pthread_detach(pthread_self());
}
