#ifndef SETUP_H
#define SETUP_H

#include <arpa/inet.h>  // struct sockaddr_in
#include <signal.h>     // struct sigaction, sigaction, sigemptyset
#include "project.h"

#define WEBROOT_PATH "/var/www/"
#define PRIVSEP_UID 1000
#define PRIVSEP_GID 1000
#define LOCAL_PORT 81

void setup_privsep(void);
void setup_signals(struct sigaction *sig_struct);
void setup_daemon(struct sigaction *sig_struct, int *sid);
void setup_socket(int *sd, struct sockaddr_in *loc_adr);
void setup_chroot(char *path);
int setup_determine_chroot(struct http_request *req);
int check_root_privileges(void);
int drop_root_privileges(void);

#endif
