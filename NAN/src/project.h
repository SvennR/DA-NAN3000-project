#ifndef PROJECT_H
#define PROJECT_H

#include <sys/types.h> // pid_t

#define HTTP_200 "200 OK"
#define HTTP_400 "400 Bad Request"
#define HTTP_403 "403 Forbidden"
#define HTTP_404 "404 Not Found"
#define HTTP_415 "415 Unsupported Media Type"
#define HTTP_500 "500 Internal Server Error"
#define HTTP_501 "501 Not Implemented"

#define DEFAULT_CONTENT_TYPE "text/plain"
#define DEFAULT_HTTP_VERSION "HTTP/1.1"

struct http_request
{
    char *type;
    char *resource;
    char *filename;
    char *extension;
    char *mime_type;
    char *version;
    char *header;
    unsigned int header_len;
    char *client_ip;
    char *query_string;
    int is_get_request;
    int is_post_request;
    int request_cgi;
    int request_has_mime;
    int request_webroot;
    int request_malformed;
    int request_not_implemented;
    pid_t pid;
};

struct http_response
{
    char *statuscode;
    char *version;
    char *content_type;
    char *header; // not in use
    char *body;   // not in use
    char *filename;
    char *cgi_resource;
    char *cgi_query_string;
    int response_send_filename;
    int response_send_webroot;
    int response_send_cgi;
};

#endif
