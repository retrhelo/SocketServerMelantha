// Author: Artyom Liu 

#include <pthread.h>
#include <semaphore.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "fifo.h"
#include "config.h"
#include "mthread.h"

/* tasklist */
static struct fifo_t *tasklist;
static pthread_mutex_t task_lock;
/* semaphores */
#define MAX_TASKS 	6
static sem_t empty, full;

#define _task_enter_cs() \
	pthread_mutex_lock(&task_lock) 
#define _task_leave_cs() \
	pthread_mutex_unlock(&task_lock) 

// handlers 
static void sender_handler(void *args);
static void receiver_handler(void *args);

// sender
#define SENDER_NUM 	3
static int mthread_sigrun = 1;

void mthread_init(struct fifo_t *fifo) {
	pthread_t new_thread;
	int i;

	// init tasklist
	pthread_mutex_init(&task_lock, NULL);
	sem_init(&empty, 0, MAX_TASKS);
	sem_init(&full, 0, 0);
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
		printf("Request from %s\n", inet_ntoa(client.sin_addr));
		if (pthread_create(&thr, NULL, receiver_handler, &new_socket) < 0) 
			perror("mthread_enter(): failed to create thread");
		else while (new_socket) 	// wait for receiver_handler to read new_socket 
			;
	}
}

void mthread_end(void) {
	mthread_sigrun = 0;

	sem_destroy(&empty);
	sem_destroy(&full);
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

static void receiver_handler(void *args) {
	int socket_fd = *(int*)args;
	*(int*)args = 0;
	FILE *fp;
	char filename[64];
	char fullname[256];
	enum http_response_t rt;

	while (http_resolve(socket_fd, filename, 64) >= 0) {
		if (0 == strcmp(filename, "")) continue;
		strcpy(fullname, config_current.root);
		strcat(fullname, filename);
		if (0 == strcmp(filename, "/")) {
			strcat(fullname, "index.html");
			strcat(filename, "index.html");
		}

		fp = fopen(fullname, "rb");
		rt = (NULL == fp ? TYPE_ERROR : get_type(filename));

		if (TYPE_ERROR == rt)
			printf("%s not found\n", filename);

		sem_wait(&empty);

		_task_enter_cs();
		fifo_enqueue(tasklist, socket_fd, fp, rt);
		_task_leave_cs();

		sem_post(&full);

		filename[0] = '\0';
	}
	close(socket_fd);
	pthread_detach(pthread_self());
}

static void sender_handler(void *args) {
	FILE *fp;
	int socket_fd;
	void *ret;
	enum http_response_t ftype;

	while (mthread_sigrun) {
		sem_wait(&full);

		_task_enter_cs();

		ret = get_head(tasklist, &socket_fd, &fp, &ftype);
		fifo_dequeue(tasklist);

		_task_leave_cs();

		sem_post(&empty);

		if (NULL == ret) continue;

		if (TYPE_ERROR == ftype) 
			http_send_404(socket_fd);
		else 
			http_send(socket_fd, ftype, fp);

		if (fp) fclose(fp);
	}

	pthread_detach(pthread_self());
}
