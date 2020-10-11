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


/* timer */
#define TIMER_SIZE 		32
#define TIMER_INIT 		3
static unsigned timer[TIMER_SIZE] = {0};
static pthread_t pthr_arr[TIMER_SIZE];
static unsigned timer_hash(pthread_t thr) {
	int index;

	index = thr % TIMER_SIZE;
	while (timer[index]) 
		index = (index + 1) % TIMER_SIZE;

	return index;
}

static pthread_mutex_t timer_lock;

static void _timer_reset(pthread_t thr) {
	int i;

	pthread_mutex_lock(&timer_lock);
	timer[i = timer_hash(thr)] = TIMER_INIT;
	pthr_arr[i] = thr;
	pthread_mutex_unlock(&timer_lock);
}

static void _timer_down(void) {
	int i;

	pthread_mutex_lock(&timer_lock);
	for (i = 0; i < TIMER_SIZE; i ++) {
		if (timer[i] > 0) timer[i] --;
		if (timer[i] == 0) {
			pthread_cancel(pthr_arr[i]);
			pthread_join(pthr_arr[i], NULL);
		}
	}
	pthread_mutex_unlock(&timer_lock);
}

/* tasklist */
static struct fifo_t *tasklist;

static pthread_mutex_t task_lock;

#define _task_enter_cs() \
	pthread_mutex_lock(&task_lock) 
#define _task_leave_cs() \
	pthread_mutex_unlock(&task_lock) 

#define SENDER_NUM 		3
static int mthread_sigrun = 1;
static pthread_t sender_arr[SENDER_NUM];

// handlers 
static void *sender_handler(void *args);
static void *receiver_handler(void *args);

void mthread_init(struct fifo_t *fifo) {
	int i;

	pthread_mutex_init(&timer_lock, NULL);
	pthread_mutex_init(&task_lock, NULL);

	/*tasklist = fifo;*/

	/*mthread_sigrun = 1;*/
	/*for (i = 0; i < SENDER_NUM; i ++) {*/
		/*pthread_create(sender_arr + i, */
				/*NULL, sender_handler, sender_arr + i);*/
	/*}*/
}

void mthread_enter(int socket_fd) {
	pthread_t thr;
	int new_socket;
	struct sockaddr_in client;
	size_t n = sizeof(client);

	while ((new_socket = accept(socket_fd, &client, &n)) >= 0) {
		pthread_create(&thr, NULL, receiver_handler, &new_socket);
		pthread_join(thr, NULL);
	}
}

void mthread_end(void) {
	int i;

	mthread_sigrun = 0;
	for (i = 0; i < SENDER_NUM; i ++) 
		pthread_join(sender_arr[i], NULL);
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
	int const socket_fd = *(int*)args;
	char filename[64];
	char fullpath[256];
	FILE *fp;
	enum http_response_t rt;

	if (http_resolve(socket_fd, filename, 64) >= 0) {
		strcpy(fullpath, config_current.root);
		strcat(fullpath, filename);
		if (0 == strcmp(filename, "/")) 
			strcat(fullpath, "index.html");

		fp = fopen(fullpath, "rb");
		rt = (NULL == fp ? TYPE_ERROR : get_type(filename));

		/*_task_enter_cs();*/
		/*fifo_enqueue(tasklist, socket_fd, fp, rt);*/
		/*_task_leave_cs();*/
		http_send(socket_fd, rt, fp);

		filename[0] = '\0';
	}
	close(socket_fd);

	pthread_exit(NULL);
}

static void *sender_handler(void *args) {
	FILE *fp;
	int socket_fd;
	void *ret;
	enum http_response_t ftype;

	pthread_t thr = *(pthread_t*)args;

	int counter = 0;

	while (mthread_sigrun) {
		_task_enter_cs();

		if (ret = get_head(tasklist, &socket_fd, &fp, &ftype)) 
			fifo_dequeue(tasklist);

		_task_leave_cs();

		if (NULL == ret) {
			counter = (counter == 1000 ? 0: counter + 1);
			sleep(0.005);		// sleep for 5ms 
			continue;
		}

		printf("sender_handler(%lu): counter: %d\n", thr, counter);
		printf("sender_handler(%lu): start send: %d\n", thr, socket_fd);
		http_send(socket_fd, ftype, fp);
		printf("sender_handler(%lu): send finish\n", thr);
		if (fp) fclose(fp);
	}

	pthread_exit(NULL);
}
