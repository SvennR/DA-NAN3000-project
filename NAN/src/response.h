#ifndef RESPONSE_H
#define RESPONSE_H

#include "project.h"

void init_http_response(struct http_response *res);
void free_http_response(struct http_response *res);
void populate_response(struct http_request *req, struct http_response *res);
int  populate_response_cgi_query_string(struct http_request *req, struct http_response *res);
void populate_response_version(struct http_request *req, struct http_response *res);
void populate_response_content_type(struct http_request *req, struct http_response *res);
void populate_response_statuscode(struct http_request *req, struct http_response *res);
void populate_response_send(struct http_request *req, struct http_response *res);
void response_send_head(struct http_response *res);
void response_send_body(struct http_request *req, struct http_response *res);
void debug_log_response_members(struct http_response *res);

#endif