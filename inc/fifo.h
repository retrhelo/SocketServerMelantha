// Author: Artyom Liu 
// Description: A FIFO data structure 

#ifndef __FIFO_H
#define __FIFO_H 

#include <stdio.h>

#include "http.h"

#define TRUE 	1
#define FALSE 	0

struct fifo_node {
	int socket_fd;
	FILE *p_file;			// the file to be sent 
	enum http_response_t filetype;
	int is_active;			// whether this node is active 
	struct fifo_node *next;
};

struct fifo_t {
	struct fifo_node *head;
	struct fifo_node *tail;
};

struct fifo_t *fifo_create(void);

/* 
	Return 
		FALSE - fail to enqueue due to various reasons 
		TRUE - operation succeed 
*/
int fifo_enqueue(struct fifo_t *fifo, int socket_fd, 
		FILE *fp, enum http_response_t ft);

/* 
	Return 
		FALSE - fail to enqueue due to various reasons 
		TRUE - operation succeed 
*/
int fifo_dequeue(struct fifo_t *fifo);

struct fifo_node *get_head(struct fifo_t *fifo, int *socket_fd, 
		FILE **fp, enum http_response_t *filetype);

void fifo_destruct(struct fifo_t *fifo);

#endif 
