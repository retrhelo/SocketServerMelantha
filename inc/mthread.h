// Author: Artyom Liu 
// Description: Multi-Threading related. Manage the threads. 

#ifndef __MTHREAD_H
#define __MTHREAD_H

/* 
	Args: 
		sender_num - the number of sender threads
*/
void mthread_init(struct fifo_t *fifo);

void mthread_enter(int socket_fd);

void mthread_end(void);

#endif
