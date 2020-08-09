#include <arpa/inet.h>  // struct sockaddr_in
#include <unistd.h>     // dup2, fork, close
#include <stdlib.h>     // exit
#include <stdio.h>      // fopen, fprintf, fclose
#include <sys/socket.h> // accept, listen, shutdown
#include <sys/types.h>  // accept, listen, fork
#include <signal.h>     // struct sigaction

#include "project.h"
#include "setup.h"
#include "request.h"
#include "response.h"

#define LOG_FILE "/var/log/debug.log"
#define MIME_FILE "/etc/mime.types"
#define BACKLOG 10 // size of the queue for pending requests

// TODO:    check if CGI script is executable before executing.. (access(file, X_OK))
//          check for occurrences of "../" in CGI resource before executing
//          CGI child should scan through http header and set a bunch of environment variables
//          split up and simplify response_send_body()

int main(int argc, char **argv)
{
    struct sockaddr_in loc_adr, cli_adr;
    socklen_t cli_adr_len = sizeof(cli_adr);
    struct sigaction sig_struct;
    int sd, new_sd, sid;
    FILE *mimefp = fopen(MIME_FILE, "r");
    FILE *logfp = fopen(LOG_FILE, "a");

    if (logfp == NULL)
        fprintf(stderr, "ERROR: unable to open log file (%s)\n", LOG_FILE);
    if (mimefp == NULL)
        fprintf(stderr, "ERROR: unable to open MIME-TYPE file (%s). "
                        "MIME-TYPE matching will fail!\n",
                MIME_FILE);

    if (check_root_privileges() != 0)
    {
        fprintf(stderr, "ERROR: Not running as root\nExiting..\n");
        exit(EXIT_FAILURE);
    }

    // DAEMONIZE
    setup_daemon(&sig_struct, &sid);

    // fileno examines the argument stream and returns its integer descriptor
    // STDERR_FILENO - fd of stderror stream
    // dup2(fileno(logfp), fileno(stderr));
    // duplicate logfp to fd2
    dup2(fileno(logfp), STDERR_FILENO);
    fclose(logfp);

    // SOCKET INIT
    setup_socket(&sd, &loc_adr);

    // Waiting for connection request
    listen(sd, BACKLOG);
    while (1)
    {

        // Accept received request
        // accept(socket, address, address_len) - saves address, address length
        new_sd = accept(sd, (struct sockaddr *)&cli_adr, &cli_adr_len);

        if (0 == fork()) // child
        {
            struct http_request req;
            struct http_response res;

            // fd0: sd      (we want new_sd here too!)
            // fd1: new_sd
            // fd2: logfp
            dup2(new_sd, 0);

            init_http_request(&req, &cli_adr);
            init_http_response(&res);

            // read http header from socket
            extract_request_header(&req);

            // determine ASAP whether or not to chroot(WEBROOT_PATH)
            if (setup_determine_chroot(&req) == 0)
                setup_chroot(WEBROOT_PATH);

            // DROP PRIVILEGES
            setup_privsep();

            // write request header to LOG_FILE
            log_request(&req);

            // populate the http_request struct
            extract_request_type(&req);
            extract_request_tokens(&req, mimefp);

            // populate http_response struct
            populate_response(&req, &res);

            // send response
            response_send_head(&res);
            response_send_body(&req, &res);

            // write struct members to LOG_FILE
            // clean up malloc()'ed members
            debug_log_request_members(&req);
            debug_log_response_members(&res);
            free_http_request(&req);
            free_http_response(&res);

            // Makes sure that the socket is closed for read & write
            // NB! Does not free up place in the filedescriptortable
            shutdown(STDOUT_FILENO, SHUT_RDWR);
            exit(EXIT_SUCCESS);
        }

        else
        { // parent
            close(new_sd);
        }
    }

    fclose(mimefp);
    return 0;
}