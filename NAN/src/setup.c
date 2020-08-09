// ---------------
// SETUP FUNCTIONS
// ---------------
#include <stdio.h>      // perror, fprintf
#include <stdlib.h>     // exit
#include <unistd.h>     // fork
#include <sys/socket.h> // socket
#include <sys/types.h>  // fork, socket

#include "setup.h"
#include "request.h"

void setup_signals(struct sigaction *sig_struct)
{
    // Sets the action of signal to ignore
    sig_struct->sa_handler = SIG_IGN;
    // Do not modifiy behaviour of signals
    sig_struct->sa_flags = 0;
    // Initialise the signal set to empty
    sigemptyset(&sig_struct->sa_mask);
    // Sigaction(signaltype, new action (sig_action_struct), old action (NULL))
    // sigaction (/* address of signal handler */, /* additional signals to block */, /* signal options */)
    // Set the sighup signal to SIG_IGN - no flags
    sigaction(SIGHUP, sig_struct, 0);
    // Set the SIGCHLD signal to SIG_IGN - no flags
    sigaction(SIGCHLD, sig_struct, 0); // solves zombie issue
}

void setup_daemon(struct sigaction *sig_struct, int *sid)
{
    // fork() = 0 i child, >0 i parent
    if (fork() != 0)
        exit(EXIT_SUCCESS);    // server runs in the background and is not process leader
    *sid = setsid();           // Calling process will be process leader and session leader
    setup_signals(sig_struct); // ignore SIGHUP
    if (fork() != 0)
        exit(EXIT_SUCCESS); // "Disassociate from the control terminal (and take steps not to reacquire one)"
    if (close(0) != 0)
        perror("ERROR close()"); // closing unnecessary file descriptors (stdin, stdout)
    if (close(1) != 0)
        perror("ERROR close()");
}

// NOTE: the calls to exit() will only exit the child process
// since setup_chroot() is called by the child
void setup_chroot(char *path)
{
    if (chdir(WEBROOT_PATH) == 0)
        fprintf(stderr, "chdir(%s) successful!\n", WEBROOT_PATH);

    else
    {
        perror("Failed chdir()");
        fprintf(stderr, "Exiting..\n");
        exit(EXIT_FAILURE);
    }

    if (chroot(WEBROOT_PATH) == 0)
        fprintf(stderr, "chroot(%s) successful!\n", WEBROOT_PATH);

    else
    {
        perror("Failed chroot()");
        fprintf(stderr, "Exiting..\n");
        exit(EXIT_FAILURE);
    }
}

// code from lecture: translated to english for conformance reasons
void setup_socket(int *sd, struct sockaddr_in *loc_adr)
{
    // Setting up socket structure
    // socket(ipv4, connection-based protocol, TCP)
    *sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // setsockopt - set options on sockets
    // setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
    // SOL_SOCKET = API level, SO_REUSEDDR Indicates that the rules used in validating addresses supplied
    // in a bind(2) call should allow reuse of local addresses
    // optval is a pointer to an int
    // Making sure that the os should not reserve the port after the server dies
    setsockopt(*sd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    // Initiates local address
    loc_adr->sin_family = AF_INET;                  // IPv4
    loc_adr->sin_port = htons((u_short)LOCAL_PORT); // 16 bit hostshort -> netshort
    loc_adr->sin_addr.s_addr = htonl(INADDR_ANY);   // 32 bit netlong -> hostlong.

    // Connecting socket and local address together
    if (0 == bind(*sd, (struct sockaddr *)loc_adr, sizeof(*loc_adr)))
        fprintf(stderr, "Process %d is attached to port %d.\n", getpid(), LOCAL_PORT);
    else
    {
        perror("ERROR bind()");
        fprintf(stderr, "Exiting..\n");
        exit(EXIT_FAILURE);
    }
}

//  PURPOSE  chroot() or not?
//  chroot:  any request NOT involving CGI scripts
// !chroot:  CGI script (needs access to /bin directory)
//
// return  0 to indicate  chroot
// return  1 to indicate !chroot
int setup_determine_chroot(struct http_request *req)
{
    // malformed request
    if (request_num_tokens(req) != 3)
        return 0;

    // GET request with CGI
    else if (req_is_get_cgi(req) == 1)
        return 1;

    // POST request with CGI
    else if (req_is_post_cgi(req) == 1)
        return 1;

    // no CGI
    return 0;
}

void setup_privsep()
{
    int ret;
    ret = drop_root_privileges();

    if (ret == 0)
        fprintf(stderr, "Successfully dropped root privileges.\n");
    else if (ret == -1)
        fprintf(stderr, "Failed to drop root privileges.\n");
    else if (ret == -2)
        fprintf(stderr, "Failed to drop root privileges (process not running as root)\n");
}

// Check if process is running as root
// return  0 if root
// return -1 if not root
int check_root_privileges(void)
{
    if (geteuid() != 0)
        return -1;
    else
        return 0;
}

// drop root privilege
// return  0 on success
// return -1 on failure
// return -2 if not running as root
int drop_root_privileges(void)
{
    // process is running as root, drop privileges
    if (check_root_privileges() == 0)
    {
        fprintf(stderr, "Dropping root privileges.\n");
        if (setgid(PRIVSEP_GID) != 0)
        {
            perror("setgid() failed!");
            return -1;
        }

        if (setuid(PRIVSEP_UID) != 0)
        {
            perror("setuid() failed!");
            return -1;
        }
    }
    // not running as root
    else
        return -2;

    if (setuid(0) == 0 || seteuid(0) == 0)
    {
        fprintf(stderr, "ERROR: Failed to drop privileges.\nExiting..\n");
        exit(EXIT_FAILURE);
    }
    else
        perror("setuid() / seteuid()");

    return 0;
}