// ----------------------
// HTTP_REQUEST FUNCTIONS
// ----------------------
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "project.h"
#include "misc.h"
#include "request.h"

// save request header in req->header
// everything before the first occurrence of "\n\n" or "\r\n\r\n"
// return  0 on success
// return -1 to indicate partial extraction (realloc() failed)
//
// based on code from lecture (translated for conformance reasons)
int extract_request_header(struct http_request *req)
{
    char *ptr_realloc = NULL;
    char *tmp = NULL;
    char curr = '\0';
    char prev = '\0';
    unsigned int count = 0;
    unsigned int buf_len = BUFSIZE; // 4KB

    req->header = malloc(BUFSIZE * sizeof(char));
    tmp = req->header;

    // read(int fd, void *buf, size_t count
    // read() attempts to read up to count bytes from file descriptor fd
    // into the buffer starting at buf.
    while (0 < read(0, &curr, 1)) // read one character at a time from sd
    {
        // stop reading at some point
        if (count == (MAX_HEADER_SIZE - 1))
        {
            *tmp = '\0';
            break; // exits loop
        }

        // allocate more memory if needed
        if (count == (buf_len - 1))
        {
            buf_len *= 2;
            fprintf(stderr, "DEBUG: Doubling req->header size (old: %u)(new: %u)\n", count + 1, buf_len);

            ptr_realloc = realloc(req->header, buf_len);

            // realloc() failed, continue with partial header..
            if (ptr_realloc == NULL)
            {
                fprintf(stderr, "DEBUG: realloc(req->header, %u) FAILED!\n", buf_len);
                perror("realloc()");
                *tmp = '\0';
                req->header_len = count;
                return -1;
            }

            req->header = ptr_realloc;
            ptr_realloc = NULL;
            tmp = req->header;
            tmp += count;
        }

        if (curr == '\r')
            continue;

        // end of header detected
        if (curr == '\n' && prev == '\n')
        {
            *tmp = '\0';
            break;
        }

        // add character to req->header
        *tmp = curr;
        tmp++;
        count++;

        prev = curr;
    }

    req->header_len = count;
    return 0;
}

// set various environment variables for CGI scripts
// currently only CONTENT_LENGTH
int process_request_header(struct http_request *req)
{
    int pos = 0;
    char *tmp = NULL;
    char *line = NULL;
    char *content_length = NULL;
    char *cookie = NULL;

    tmp = req->header;
    // dont do anything with first line
    if (sscanf(tmp, "%m[^\n]\n%n", &line, &pos) != 1)
        return -1;

    ptr_free_ifnotnull(1, &line);
    tmp += pos;

    while (sscanf(tmp, "%m[^\n]\n%n", &line, &pos) == 1)
    {
        tmp += pos;

        if (sscanf(line, "Content-Length: %ms", &content_length) == 1)
        { // int setenv(const char *name, const char *value, int overwrite);
            setenv("CONTENT_LENGTH", content_length, 1);
            fprintf(stderr, "DEBUG: Set CONTENT_LENGTH to %s\n", content_length);
        }

        if (sscanf(line, "Cookie: %ms", &cookie) == 1)
        {
            setenv("HTTP_COOKIE", cookie, 1);
            fprintf(stderr, "DEBUG: Set HTTP_COOKIE to %s\n", cookie);
        }

        ptr_free_ifnotnull(3, &line, &content_length, &cookie);
    }

    if (getenv("CONTENT_LENGTH") == NULL)
    {
        setenv("CONTENT_LENGTH", "0", 1);
        fprintf(stderr, "DEBUG: CONTENT_LENGTH was unset (now 0)\n");
    }

    if (getenv("HTTP_COOKIE") == NULL)
        fprintf(stderr, "DEBUG: HTTP_COOKIE is not set!\n");

    return 0;
}

// PURPOSE: used to verify that there are 3 (or more) tokens in request
// return:  number of tokens in the request eg. GET(1) /file(2) HTTP/1.1(3)
// OBS OBS! only counts up to 3!
int request_num_tokens(struct http_request *req)
{
    char *tmp0 = NULL, *tmp1 = NULL, *tmp2 = NULL;
    int ret;

    // sscanf() returns number of input items successfully matched and assigned
    // int sscanf(const char *str, const char *format, ...);
    // %ms: Read a string. Statement is similar to the same thing with %s,
    // but %ms allocates enough memory to hold the string that it reads,
    // and stores a pointer to the newly allocated array into s
    ret = sscanf(req->header, "%ms %ms %ms", &tmp0, &tmp1, &tmp2);
    ptr_free_ifnotnull(3, &tmp0, &tmp1, &tmp2);

    return ret;
}

// if GET request for CGI script:   return 1
// else:                            return 0
int req_is_get_cgi(struct http_request *req)
{
    char *tmp1 = NULL;
    char *tmp2 = NULL;
    char *filepath = NULL;
    int ret = 0;

    // GET request CGI detection
    if (sscanf(req->header, "GET /cgi-bin/%ms %ms", &tmp1, &tmp2) == 2)
    {
        // char *strchr(const char *s, int c);
        // The strchr() function returns a pointer to the first occurrence of the character c in the string s.

        // extract filepath (without QUERY_STRING)
        if (strchr(tmp1, '?') != NULL)
            // This is similar to %ms, but it omits skipping over initial white space and
            // reads a string consisting of zero or more letters
            // The ^ character means anything except these. So you get a string without ?.
            sscanf(req->header, "GET %m[^?]", &filepath);
        else
            sscanf(req->header, "GET %ms", &filepath);

        // rewrite to non-chroot path - from /cgi-bin/* to /var/www/cgi-bin/*
        filepath = cgi_filepath_rewrite(filepath, 1);

        // !chroot only if the file actually exists/readable/(executable *TODO*)
        if (file_accessible(filepath) == 1)
            ret = 1;
        else
            ret = 0;
    }

    ptr_free_ifnotnull(3, &tmp1, &tmp2, &filepath);

    return ret;
}

// if POST request for CGI script:  return 1
// else:                            return 0
int req_is_post_cgi(struct http_request *req)
{
    char *tmp1 = NULL;
    char *tmp2 = NULL;
    char *filepath = NULL;
    int ret = 0;

    // GET request CGI detection
    if (sscanf(req->header, "POST /cgi-bin/%ms %ms", &tmp1, &tmp2) == 2)
    {
        // extract filepath (without QUERY_STRING)
        if (strchr(tmp1, '?') != NULL)
            sscanf(req->header, "POST %m[^?]", &filepath);
        else
            sscanf(req->header, "POST %ms", &filepath);

        filepath = cgi_filepath_rewrite(filepath, 1);

        // !chroot only if the file actually exists/readable/(executable *TODO*)
        if (file_accessible(filepath) == 1)
            ret = 1;
        else
            ret = 0;
    }

    ptr_free_ifnotnull(3, &tmp1, &tmp2, &filepath);

    return ret;
}

// populate http_request.type field
// return  0 if request type is supported/implemented
// return  1 if request type is not implemented or unknown
int extract_request_type(struct http_request *req)
{
    req->type = malloc(20 * sizeof(char));

    if (sscanf(req->header, "%19s", req->type) != 1)
    {
        req->request_malformed = 1;
        fprintf(stderr, "ERROR: Malformed HTTP request!\n");

        ptr_free_ifnotnull(1, &req->type);
        return 1;
    }

    if (strcmp("GET", req->type) == 0)
    {
        req->is_get_request = 1;
        return 0;
    }

    else if (strcmp("POST", req->type) == 0)
    {
        req->is_post_request = 1;
        return 0;
    }

    else
    {
        req->request_not_implemented = 1;
        return 1;
    }
}

// populate http_request.resource and http_request.version
//          also filename, query_string, extension and mime_type fields if applicable
// returns 0 if everything goes smooth
// returns 1 if request is malformed
int extract_request_tokens(struct http_request *req, FILE *mimefp)
{
    if (strcmp("GET", req->type) == 0)
        return extract_get_tokens(req, mimefp);

    else if (strcmp("POST", req->type) == 0)
        return extract_post_tokens(req);

    // else
    fprintf(stderr, "ERROR: %s req->type not recognized!\n", req->type);
    return 1;
}

// *TODO* simplify and break up into smaller functions!
int extract_get_tokens(struct http_request *req, FILE *mimefp)
{
    // check for invalid request: "GET" should be followed by 2 tokens
    if (sscanf(req->header, "GET %ms %ms", &req->resource, &req->version) != 2)
    {
        fprintf(stderr, "ERROR: GET request lacks RESOURCE|VERSION\n");
        req->request_malformed = 1;
        return 1;
    }

    // GET request can be for:
    // 1. webroot
    // 2. cgi script            (matches subdirectory /cgi-bin/)
    // 3. /resource OR resource (leading slash or not)

    // webroot
    if (strcmp(req->resource, "/") == 0)
    {
        fprintf(stderr, "GET request for webroot\n");
        req->request_webroot = 1;
        fclose(mimefp);
        return 0;
    }

    // CGI
    // *should* check for "../" to avoid executing cgi scripts outside of /cgi-bin/
    else if (sscanf(req->resource, "/cgi-bin/%ms", &req->filename) == 1)
    {
        char *tmp = NULL;
        fclose(mimefp);

        // query_string
        if ((tmp = strchr(req->filename, '?')) != NULL)
        {
            // something after '?' => save query_string
            if (*(++tmp) != ' ')
                sscanf(req->resource, "/cgi-bin/%255[^?]?%ms", req->filename, &req->query_string);

            // nothing after '?' => dont save query_string
            else
                sscanf(req->resource, "/cgi-bin/%255[^?]", req->filename);

            // resource is filepath without query_string part
            sscanf(req->resource, "%[^?]", req->resource);
        }

        // no query_string
        // limit filename to 255 characters (NAME_MAX?)
        else
            sscanf(req->resource, "/cgi-bin/%255s", req->filename);

        // rewrite path to pre-chroot path
        req->resource = cgi_filepath_rewrite(req->resource, 1);

        fprintf(stderr, "DEBUG: GET request for CGI script (%s) (query_string: %s)\n", req->filename, req->query_string);

        if (file_accessible(req->resource) == 1)
        {
            fprintf(stderr, "DEBUG: CGI script exists! (%s)\n", req->resource);
            req->request_cgi = 1;
        }
        else
        {
            fprintf(stderr, "DEBUG: CGI script is not accessible! (%s)\n", req->resource);
            req->request_cgi = -1;
            return 1;
        }

        return 0;
    }

    // resource (both cases -- slash and no slash)
    // not cgi or webroot (only /)
    else
    {
        // extract filename and extension
        // populate http_request.filename and http_request.extension
        // check mime-type
        if (sscanf(req->resource, "/%ms", &req->filename) != 1)
            sscanf(req->resource, "%ms", &req->filename);

        fprintf(stderr, "GET request for resource: %s\n", req->resource);

        req->extension = get_extension(req->filename);

        // if file has extension:
        // check mime-type and populate http_request.mime_type
        if (req->extension != NULL)
        {
            if (extract_mimetype(req, mimefp) == 0)
            {
                req->request_has_mime = 1;
                fprintf(stderr, "File \"%s\" has supported mime-type (%s)\n",
                        req->filename, req->mime_type);
            }
            else
            {
                fprintf(stderr, "File \"%s\" has UNKNOWN mime-type!\n", req->filename);
            }
        }

        else
            fprintf(stderr, "Resource \"%s\" is a file w/o extension (or is a directory)\n",
                    req->filename);

        return 0;
    }
}

int extract_post_tokens(struct http_request *req)
{
    /*
        CGI program will receive the encoded form input on stdin. 
        The server will NOT send you an EOF on the end of the data, 
        instead you should use the environment variable CONTENT_LENGTH 
        to determine how much data you should read from stdin
    */

    // check for invalid request: "POST" should be followed by 2 tokens
    if (sscanf(req->header, "POST %ms %ms", &req->resource, &req->version) != 2)
    {
        fprintf(stderr, "ERROR: POST request lacks RESOURCE|VERSION\n");
        req->request_malformed = 1;
        return 1;
    }

    // CGI
    else if (sscanf(req->resource, "/cgi-bin/%ms", &req->filename) == 1)
    {
        char *tmp = NULL;

        // query_string
        if ((tmp = strchr(req->filename, '?')) != NULL)
        {
            // something after '?' => save query_string
            if (*(++tmp) != ' ')
                sscanf(req->resource, "/cgi-bin/%255[^?]?%ms", req->filename, &req->query_string);

            // nothing after '?' => dont save query_string
            else
                sscanf(req->resource, "/cgi-bin/%255[^?]", req->filename);

            // resource is filepath without query_string part
            sscanf(req->resource, "%[^?]", req->resource);
        }

        // no query_string
        // limit filename to 255 characters (NAME_MAX?)
        else
            sscanf(req->resource, "/cgi-bin/%255s", req->filename);

        // rewrite path to pre-chroot path
        req->resource = cgi_filepath_rewrite(req->resource, 1);

        fprintf(stderr, "DEBUG: GET request for CGI script (%s) (query_string: %s)\n", req->filename, req->query_string);

        if (file_accessible(req->resource) == 1)
        {
            fprintf(stderr, "DEBUG: CGI script exists! (%s)\n", req->resource);
            req->request_cgi = 1;
        }
        else
        {
            fprintf(stderr, "DEBUG: CGI script is not accessible! (%s)\n", req->resource);
            req->request_cgi = -1;
            return 1;
        }

        return 0;
    }

    return 0;
}

// extract  MIME-TYPE of file extension
//          eg. "text/plain" for "txt" extension
//          assign request_has_mime = 1 if appropriate
// populate http_request.mime_type field
//
// returns 0 if MIME-TYPE is extracted
// returns 1 if MIME-TYPE is not found
int extract_mimetype(struct http_request *req, FILE *mimefp)
{
    int ret;
    int len; // line position offset for sscanf()
    char f_ext[1024];
    char *line = malloc(1024 * sizeof(char));
    char *tmp; // points to (line+len) for lines with multiple file extensions

    req->mime_type = malloc(256 * sizeof(char));

    // sets the file position to the beginning of the file of the given stream.
    rewind(mimefp);

    // char *fgets(char *str, int n, FILE *stream) reads a line from the specified
    // stream and stores it into the string pointed to by str. n = max char read
    while (fgets(line, 1022, mimefp) != NULL)
    {
        // skip if comment
        if (line[0] == '#')
            continue;

        tmp = line;
        // populate mime_type field
        ret = sscanf(line, "%s%n", req->mime_type, &len);
        tmp += len;

        // empty line
        if (ret == EOF)
            continue;

        // loops over all listed extensions and checks for a match
        while ((ret = sscanf(tmp, "%1022s%n", f_ext, &len) == 1))
        {
            tmp += len;

            if (strcasecmp(f_ext, req->extension) == 0)
            {
                // match
                free(line);
                fclose(mimefp);
                return 0;
            }
        }
    }

    // no match
    ptr_free_ifnotnull(2, &line, &req->mime_type);
    fclose(mimefp);
    return 1;
}

void debug_log_request_members(struct http_request *req)
{
    fprintf(stderr, "----------------------------\n"
                    "http_request members:\n"
                    "----------------------------\n"
                    "type: %s\n"
                    "resource: %s\n"
                    "filename: %s\n"
                    "extension: %s\n"
                    "mime_type: %s\n"
                    "query_string: %s\n"
                    "version: %s\n"
                    "client_ip: %s\n"
                    "request_cgi: %d\n"
                    "request_has_mime: %d\n"
                    "request_webroot: %d\n"
                    "request_malformed: %d\n"
                    "request_not_implemented: %d\n"
                    "is_get_request: %d\n"
                    "is_post_request: %d\n"
                    "----------------------------\n",
            req->type, req->resource, req->filename,
            req->extension, req->mime_type, req->query_string,
            req->version, req->client_ip, req->request_cgi,
            req->request_has_mime, req->request_webroot,
            req->request_malformed, req->request_not_implemented,
            req->is_get_request, req->is_post_request);
}

// initialize http_* struct members
// ATTENTION: these need to be updated whenever members are added to http_request and http_response
//            see "project.h"
void init_http_request(struct http_request *req, struct sockaddr_in *cli_adr)
{
    req->type = NULL;
    req->resource = NULL;
    req->version = NULL;
    req->client_ip = NULL;
    req->header = NULL;
    req->header_len = 0;
    req->filename = NULL;
    req->extension = NULL;
    req->mime_type = NULL;
    req->query_string = NULL;
    req->is_get_request = 0;
    req->is_post_request = 0;
    req->request_cgi = 0;
    req->request_has_mime = 0;
    req->request_webroot = 0;
    req->request_malformed = 0;
    req->request_not_implemented = 0;

    // for debugging file descriptor issues
    req->pid = getpid();

    // set client_ip to address of connecting peer
    // INET_ADDRSTRLEN = Max length of the IPv4 as string
    req->client_ip = malloc(INET_ADDRSTRLEN * sizeof(char));
    // inet_ntop - convert IPv4 and IPv6 addresses from binary to text form
    inet_ntop(AF_INET, &((*cli_adr).sin_addr), req->client_ip, INET_ADDRSTRLEN);
}

// free_http* functions should free() all malloc()'ed memory in the http_* structs
// ATTENTION: these need to be updated whenever pointers are added to http_request & http_response
//            see "project.h"
void free_http_request(struct http_request *req)
{
    // char *type, *resource, *version, *header, *filename,
    //      *extension, *mime_type, *client_ip, *query_string;

    ptr_free_ifnotnull(4, &req->type, &req->resource, &req->version, &req->header);
    ptr_free_ifnotnull(5, &req->filename, &req->extension, &req->mime_type, &req->client_ip, &req->query_string);
}

// save request header in LOG_FILE
void log_request(struct http_request *req)
{
    fprintf(stderr, "\n\nReceived request from %s (%u bytes)(pid %d):\n", req->client_ip, req->header_len, req->pid);
    fprintf(stderr, "<HEADER>\n%s</HEADER>\n\n", req->header);
}