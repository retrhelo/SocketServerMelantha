// Author: Artyom Liu 

#pragma GCC dependency "../inc/fifo.h"

#include <stdlib.h>

#include <unistd.h>

#include <sys/socket.h>

#include "fifo.h"

struct fifo_t *fifo_create(void) {
	struct fifo_t *ret;
	struct fifo_node *first_node;

	if (ret = malloc(sizeof(struct fifo_t))) {
		if (first_node = malloc(sizeof(struct fifo_node))) {
			ret->head = ret->tail = first_node;
			first_node->next = first_node;
			first_node->is_active = FALSE;
		}
		else {
			free(ret);
			ret = NULL;
		}
	}

	return ret;
}

int fifo_enqueue(struct fifo_t *fifo, int socket_fd, 
		FILE *fp, enum http_response_t ft) {
	struct fifo_node *tmp;

	tmp = fifo->tail->next;
	if (TRUE == tmp->is_active) {
		// a new node required 
		tmp = malloc(sizeof(struct fifo_node));
		if (NULL == tmp) return FALSE;

		tmp->next = fifo->tail->next;
		fifo->tail->next = tmp;
	}
	fifo->tail = tmp;

	// assign basic values 
	tmp->socket_fd = socket_fd;
	tmp->filetype = ft;
	tmp->p_file = fp;
	tmp->is_active = TRUE;

	return TRUE;
}

int fifo_dequeue(struct fifo_t *fifo) {
	if (fifo->head->is_active) {
		fifo->head->is_active = FALSE;
		fifo->head = fifo->head->next;
		return TRUE;
	}
	else return FALSE;
}

struct fifo_node *get_head(struct fifo_t *fifo, int *socket_fd, 
		FILE **fp, enum http_response_t *filetype ) {
	struct fifo_node *tmp;

	tmp = fifo->head;
	if (tmp && tmp->is_active) {
		*socket_fd = tmp->socket_fd;
		*fp = tmp->p_file;
		*filetype = tmp->filetype;
		return tmp;
	}
	else return NULL;
}

void fifo_destruct(struct fifo_t *fifo) {
	struct fifo_node *tmp;
	struct fifo_node *pre;

	tmp = fifo->head->next;
	while (fifo->head != tmp) {
		if (tmp->is_active) {
			close(tmp->socket_fd);
			if (tmp->p_file) fclose(tmp->p_file);
		}
		pre = tmp;
		tmp = tmp->next;
		free(pre);
	}
	if (fifo->head->is_active) {
		close(fifo->head->socket_fd);
		fclose(fifo->head->p_file);
	}
	free(fifo->head);

	free(fifo);
}
