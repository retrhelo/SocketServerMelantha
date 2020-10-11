// Author: Artyom Liu 

#pragma GCC dependency "../inc/http.h"

// network 
#include <sys/socket.h>
#include <arpa/inet.h>

// linux specified 
#include <unistd.h>

// generic 
#include <stdio.h>
#include <string.h>

#include "http.h"

#define RECV_BUF_SIZE 	4096u 	// 4K buf for receive 

enum http_cmd_t http_resolve(int socket_fd, char *filename, int size) {
	char buf[RECV_BUF_SIZE];
	char *tmp;
	int ret;
	int i;

	static char const *HTTP_CMD_GET = "GET";

	while ((ret = recv(socket_fd, buf, RECV_BUF_SIZE, 0)) >= 0) {
		i = 0;
		if (tmp = strstr(buf, HTTP_CMD_GET)) {
			tmp = tmp + 4;
			while (i < size && ' ' != *tmp) 
				filename[i++] = *tmp++;
			break;
		}
	}
	filename[i == size ? size - 1: i] = '\0';

	return ret;
}

static char const *HTTP_RESPONSE_HEAD_FORMAT = 
	"HTTP/1.1 %s\r\n"
	"Content-Type: %s; charset=utf-8\r\n"
	"Content-Length: %d\r\n\r\n";

#define SEND_BUF_SIZE 	4096u 

static int http_send_head(int socket_fd, char *buf, 
		enum http_response_t rt, int file_size) {
	static char const *response_str[] = {
		"", 
		"text/html", 
		"image/jpg", 
		"image/png", 
		"image/gif", 
		"text/css", 
	};
	static char const *response_type[] = {
		"404 Not Found", 
		"200 Ok", 
	};

	int ret;

	sprintf(buf, HTTP_RESPONSE_HEAD_FORMAT, 
			response_type[TYPE_ERROR == rt ? 0 : 1], 
			response_str[rt], 
			file_size);
	send(socket_fd, buf, (ret = strlen(buf)), 0);

	return ret;
}

int http_send(int socket_fd, enum http_response_t rt, FILE *fp) {
	char buf[SEND_BUF_SIZE];
	long file_size;
	long head, text;
	long cnt;
	int timer;

	static int const TIMER_INIT = 5;

	// get file size
	if (fp) {
		fseek(fp, 0, SEEK_END);
		file_size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
	}
	else file_size = 0;

	head = http_send_head(socket_fd, buf, rt, file_size);

	// start to send the file 
	text = 0;
	timer = TIMER_INIT;
	while (text < file_size) {
		if ((file_size - text) > SEND_BUF_SIZE) {
			fread(buf, 1, SEND_BUF_SIZE, fp);
			cnt = send(socket_fd, buf, SEND_BUF_SIZE, 0);
		}
		else {
			fread(buf, 1, file_size - text, fp);
			cnt = send(socket_fd, buf, file_size - text, 0);
		}

		if (-1 == cnt) {
			perror("http_send(): fail to send");
			if (0 == --timer) break;
		}
		else {
			timer = TIMER_INIT;
			text += cnt;
		}
	}

	return head + text;
}
