// -----------------------
// HTTP_RESPONSE FUNCTIONS
// -----------------------
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "project.h"
#include "misc.h"
#include "response.h"
#include "request.h"

// populate http_response members
void populate_response(struct http_request *req, struct http_response *res)
{
    populate_response_cgi_query_string(req, res);
    populate_response_version(req, res);
    populate_response_content_type(req, res);
    populate_response_statuscode(req, res);
    populate_response_send(req, res); // response_send_filename, response_send_webroot, response_send_cgi;
}

int populate_response_cgi_query_string(struct http_request *req, struct http_response *res)
{
    // if not cgi or query_string is empty, return
    if (req->request_cgi <= 0 || req->query_string == NULL)
        return 1;
    //else - copy query string from request to respose
    res->cgi_query_string = malloc((strlen(req->query_string) + 1) * sizeof(char));
    strncpy(res->cgi_query_string, req->query_string, strlen(req->query_string) + 1);

    return 0;
}

// populate http_response.version field. default is HTTP/1.1
void populate_response_version(struct http_request *req, struct http_response *res)
{
    // copy http_request.version from request?
    // or use DEFAULT_HTTP_VERSION?
    if (req->version != NULL)
    {
        res->version = malloc((strlen(DEFAULT_HTTP_VERSION) + 1) * sizeof(char));
        strncpy(res->version, DEFAULT_HTTP_VERSION, strlen(DEFAULT_HTTP_VERSION) + 1);
    }

    // if request is malformed/not implemented
    // use DEFAULT_HTTP_VERSION
    else if (req->version == NULL || req->request_malformed == 1 || req->request_not_implemented == 1)
    {
        ptr_free_ifnotnull(1, &res->version);

        res->version = malloc((strlen(DEFAULT_HTTP_VERSION) + 1) * sizeof(char));
        strncpy(res->version, DEFAULT_HTTP_VERSION, strlen(DEFAULT_HTTP_VERSION) + 1);
    }
}

// populate http_reponse.content_type field
void populate_response_content_type(struct http_request *req, struct http_response *res)
{
    if (req->request_has_mime == 1 && file_accessible(req->filename) == 1)
    {
        res->content_type = malloc((strlen(req->mime_type) + 1) * sizeof(char));
        strncpy(res->content_type, req->mime_type, strlen(req->mime_type) + 1);
    }

    // CGI script sets the content type instead of server
    else if (req->request_cgi == 1)
    {
        // not much to do here
    }

    // DEFAULT_CONTENT_TYPE for 404's and the like
    else
    {
        res->content_type = malloc((strlen(DEFAULT_CONTENT_TYPE) + 1) * sizeof(char));
        strncpy(res->content_type, DEFAULT_CONTENT_TYPE, strlen(DEFAULT_CONTENT_TYPE) + 1);
    }
}

// populate statuscode field
// 200 OK:                       mime,  exists,  readable, (or webroot)
// 403 Forbidden:               (mime?) exists, !readable  (leaks info?)
// 404 Not Found:                      !exists
// 415 Unsupported Media Type:  !mime,  exists,  readable
//
// 400 Bad Request:     Request type  implemented, but request is malformed.
// 501 Not Implemented: Request type !implemented, no syntax checking.
//
void populate_response_statuscode(struct http_request *req, struct http_response *res)
{
    res->statuscode = malloc(32 * sizeof(char));

    // 400 Bad Request
    if (req->request_malformed)
        strncpy(res->statuscode, HTTP_400, strlen(HTTP_400) + 1);

    // 501 Not Implemented
    else if (req->request_not_implemented)
        strncpy(res->statuscode, HTTP_501, strlen(HTTP_501) + 1);

    // 415 Unsupported Media Type
    else if (req->request_has_mime == 0 && file_accessible(req->filename) == 1 && req->request_cgi == 0)
        strncpy(res->statuscode, HTTP_415, strlen(HTTP_415) + 1);

    // 404 Not Found (file resource)
    else if (req->request_webroot == 0 && req->request_cgi == 0 && file_accessible(req->filename) == -1)
    {
        strncpy(res->statuscode, HTTP_404, strlen(HTTP_404) + 1);
        // we'll add info about which file (for the response body)
        res->filename = malloc((strlen(req->filename) + 1) * sizeof(char));
        strncpy(res->filename, req->filename, strlen(req->filename) + 1);
    }

    // 404 (CGI)
    else if (req->request_cgi == -1) // CGI script is not accessible
    {
        strncpy(res->statuscode, HTTP_404, strlen(HTTP_404) + 1);
        // for the CGI case, "filename" member will not contain any path information, therefore use "resource" instead
        res->filename = malloc((strlen(req->resource) + 1) * sizeof(char));
        strncpy(res->filename, req->resource, strlen(req->resource) + 1);
    }

    // 403 Forbidden (should we lie and say 404?)
    else if (file_accessible(req->filename) == 0)
        strncpy(res->statuscode, HTTP_403, strlen(HTTP_403) + 1);

    // 200 OK (CGI)
    else if (req->request_cgi == 1)
        strncpy(res->statuscode, HTTP_200, strlen(HTTP_200) + 1);

    // 200 OK (webroot)
    else if (req->request_webroot == 1)
        strncpy(res->statuscode, HTTP_200, strlen(HTTP_200) + 1);

    // 200 OK (file)
    else if (req->request_has_mime == 1 && file_accessible(req->filename) == 1)
        strncpy(res->statuscode, HTTP_200, strlen(HTTP_200) + 1);

    // shouldn't happen..
    // ..but if it does.. use HTTP_500
    else
    {
        fprintf(stderr, "ERROR: Unable to determine suitable HTTP_STATUS for response!\n");
        strncpy(res->statuscode, HTTP_500, strlen(HTTP_500) + 1);
    }
}

// populate http_response.response_send_* fields (and filename/resource in some cases)
void populate_response_send(struct http_request *req, struct http_response *res)
{
    // should CGI script be executed?
    if (req->request_cgi == 1)
    {
        res->response_send_cgi = 1;
        res->filename = malloc((strlen(req->filename) + 1) * sizeof(char));
        strncpy(res->filename, req->filename, strlen(req->filename) + 1);
        res->cgi_resource = malloc((strlen(req->resource) + 1) * sizeof(char));
        strncpy(res->cgi_resource, req->resource, strlen(req->resource) + 1);
    }

    // Should requested file be sent?
    if (req->request_has_mime == 1 && file_accessible(req->filename) == 1)
    {
        res->filename = malloc((strlen(req->filename) + 1) * sizeof(char));
        strncpy(res->filename, req->filename, strlen(req->filename) + 1);
        res->response_send_filename = 1;
    }

    // Should webroot be sent?
    if (req->request_webroot == 1)
        res->response_send_webroot = 1;
}

// send http_response HEAD
// VERSION STATUSCODE
// CONTENT-TYPE
void response_send_head(struct http_response *res)
{
    char *extension = NULL;

    extension = get_extension(res->filename);

    if (res->response_send_cgi == 1)
    {
        printf("%s %s\n", res->version, res->statuscode);

        fprintf(stderr, "Sent response HEAD (CGI/partial) (%s %s)\n",
                res->version, res->statuscode);
    }

    // send "Content-Length:" for binary files
    else if (is_binary_file_extension(extension))
    {
        // int stat(const char *pathname, struct stat *statbuf);
        struct stat st;

        if (stat(res->filename, &st) != 0)
        {
            fprintf(stderr, "ERROR: failed to determine filesize (file: %s)\n", res->filename);
            perror("stat()");
        }

        printf("%s %s\n", res->version, res->statuscode);
        printf("Content-Type: %s\n", res->content_type);
        printf("Content-Length: %ld\n", st.st_size); // length modifier is platform dependent (%ld,%lld)
        printf("\n");

        fprintf(stderr, "Sent response HEAD (%s %s %s Content-Length: %ld)\n",
                res->version, res->statuscode, res->content_type, st.st_size);
    }
    // text files
    else
    {
        printf("%s %s\n", res->version, res->statuscode);
        printf("Content-Type: %s\n", res->content_type);
        printf("\n");

        fprintf(stderr, "Sent response HEAD (%s %s %s)\n",
                res->version, res->statuscode, res->content_type);
    }

    free(extension);
    fflush(stdout);
}

// http_response BODY
// send file, directory_listing() or some kind of error code
void response_send_body(struct http_request *req, struct http_response *res)
{
    int status_code;

    sscanf(res->statuscode, "%d", &status_code);

    // deal with "not 200" codes here
    switch (status_code)
    {
    case 501:
        printf("%s\nThe requested functionality is not implemented\n", HTTP_501);
        fprintf(stderr, "Sent response BODY (%s)\n", HTTP_501);
        break;

    case 500:
        printf("%s\nAn unexpected condition was encountered\n", HTTP_500);
        fprintf(stderr, "Sent response BODY (%s)\n", HTTP_500);
        break;

    case 400:
        printf("%s\nYour request appears to be malformed..\n", HTTP_400);
        fprintf(stderr, "Sent response BODY (%s)\n", HTTP_400);
        break;

    case 415:
        printf("%s\nServer does not support the requested MIME-TYPE\n", HTTP_415);
        fprintf(stderr, "Sent response BODY (%s)\n", HTTP_415);
        break;

    case 404:
        printf("%s\nServer could not find the requested resource (%s)\n", HTTP_404, res->filename);
        fprintf(stderr, "Sent response BODY (%s)\n", HTTP_404);
        break;

    case 403:
        printf("%s\nPermission denied.\n", HTTP_403);
        fprintf(stderr, "Sent response BODY (%s)\n", HTTP_403);
        break;

    default:
        break;
    }

    // 200 OK
    // execute CGI script
    if (res->response_send_cgi == 1)
    {
        int pid;

        if ((pid = fork()) == 0) // child
        {
            // set env variables
            process_request_header(req); // cookie/content length;
            setenv("REQUEST_METHOD", req->type, 1);

            if (res->cgi_query_string == NULL)
                setenv("QUERY_STRING", "", 1);
            else
                setenv("QUERY_STRING", res->cgi_query_string, 1);

            // int execl(const char *path, const char *arg, ...);
            execl(res->cgi_resource, res->filename, NULL);

            perror("execl()"); // only runs if execl() fails
            exit(EXIT_FAILURE);
        }

        else
        {
            fprintf(stderr, "DEBUG: child (PID %d) is executing CGI script! (%s)\n", pid, res->filename);
            wait(NULL); // Block parent until child is finished
        }

        fprintf(stderr, "DEBUG: CGI script executed! (%s)\n", res->filename);
    }

    // 200 OK
    // send webroot directory listing
    if (res->response_send_webroot == 1)
    {
        fprintf(stderr, "Sent response BODY (directory listing)\n");
        directory_listing("/");
    }

    // 200 OK
    // send file
    else if (status_code == 200 && res->response_send_filename == 1)
    {
        FILE *fp;
        char *extension = NULL;
        char line[1024];

        extension = get_extension(res->filename);

        // handle BINARY_FILE_EXTENSIONS differently
        // using send() because printf() mangles some bytes
        // *should* probably have a general case using send()
        if (is_binary_file_extension(extension))
        {
            long size;
            char *buf = NULL;
            fp = fopen(res->filename, "rb");

            // filesize "standard library way" (implementation dependent..)
            // int fseek(FILE *stream, long int offset, int whence)
            fseek(fp, 0, SEEK_END);
            size = ftell(fp); // returns the current file position of the given stream
            rewind(fp);       // reset stream

            buf = malloc(size * sizeof(char));
            // read 1 member from fp, of size "filesize", into buf
            // problematic for large files!
            // size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
            fread(buf, size, 1, fp);

            // ssize_t send(int sockfd, const void *buf, size_t len, int flags);
            send(STDOUT_FILENO, buf, size, 0);
            fprintf(stderr, "Sent response BODY (file: %s) (%ld bytes)\n", res->filename, size);
            free(buf);
        }
        // printable files
        // more sensible memory usage: line-by-line
        else
        {
            fp = fopen(res->filename, "r");

            while (fgets(line, 1022, fp) != NULL)
                printf("%s", line);

            fprintf(stderr, "Sent response BODY (file: %s)\n", res->filename);
        }

        fclose(fp);
        free(extension);
    }

    fflush(stdout);
}

void debug_log_response_members(struct http_response *res)
{
    fprintf(stderr, "http_response members:\n"
                    "----------------------------\n"
                    "statuscode: %s\n"
                    "version: %s\n"
                    "content_type: %s\n"
                    "filename: %s\n"
                    "cgi_resource: %s\n"
                    "cgi_query_string: %s\n"
                    "response_send_filename: %d\n"
                    "response_send_webroot: %d\n"
                    "response_send_cgi: %d\n"
                    "----------------------------\n",
            res->statuscode, res->version,
            res->content_type, res->filename,
            res->cgi_resource, res->cgi_query_string,
            res->response_send_filename,
            res->response_send_webroot,
            res->response_send_cgi);
}

void init_http_response(struct http_response *res)
{
    res->statuscode = NULL;
    res->version = NULL;
    res->content_type = NULL;
    res->header = NULL;
    res->body = NULL;
    res->filename = NULL;
    res->cgi_resource = NULL;
    res->cgi_query_string = NULL;
    res->response_send_filename = 0;
    res->response_send_webroot = 0;
    res->response_send_cgi = 0;
}

void free_http_response(struct http_response *res)
{
    // char *statuscode, *version, *content_type, *header, *body,
    //      *filename, *cgi_resource, *cgi_query_string;

    ptr_free_ifnotnull(4, &res->statuscode, &res->version, &res->content_type, &res->header);
    ptr_free_ifnotnull(4, &res->body, &res->filename, &res->cgi_resource, &res->cgi_query_string);
}