// Author: Artyom Liu 
// Description: Functions for sending data or responding requests 
// 			over HTTP 1.1 protocol. 

#ifndef __HTTP_H
#define __HTTP_H

#include <stdio.h>	// for FILE 

enum http_cmd_t {
	UNSUPPORTED = 0, 
	GET, 
	// other commands to be supported
};

/* 
	Receive http messages from Socket, and resolve it. 
	Write the target file name into filename. 
*/
enum http_cmd_t http_resolve(int socket_fd, char *filename, int size);

enum http_response_t {
	TYPE_ERROR = 0, 
	HTML, 
	JPG, 
	PNG, 
	GIF, 
	CSS, 
};

/* 
	Inspect the queue, and send http messages according to its node. 
	Return 
		the length of bytes sent via Socket
*/
int http_send(int socket_fd, enum http_response_t rt, FILE *fp);

#endif
