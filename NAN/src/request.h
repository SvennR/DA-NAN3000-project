#ifndef REQUEST_H
#define REQUEST_H

#include <stdio.h>      // FILE
#include <arpa/inet.h>  // struct sockaddr_in
#include "project.h"

#define BUFSIZE 4096
#define MAX_HEADER_SIZE 65536

int extract_request_header(struct http_request *req);
int process_request_header(struct http_request *req);
int extract_request_type(struct http_request *req);
int extract_request_tokens(struct http_request *req, FILE *mimefp);
int extract_get_tokens(struct http_request *req, FILE *mimefp);
int extract_post_tokens(struct http_request *req);
int extract_mimetype(struct http_request *req, FILE *mimefp);
int extract_request_header(struct http_request *req);
void debug_log_request_members(struct http_request *req);
int process_request_header(struct http_request *req);
void log_request(struct http_request *req);
void free_http_request(struct http_request *req);
void init_http_request(struct http_request *req, struct sockaddr_in *cli_adr);
int request_num_tokens(struct http_request *req);
int req_is_get_cgi(struct http_request *req);
int req_is_post_cgi(struct http_request *req);
void log_request(struct http_request *req);

#endif