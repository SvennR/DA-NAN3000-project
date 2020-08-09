#ifndef MISC_H
#define MISC_H

#define PRE_CHROOT_CGI_BIN_PATH "/var/www/cgi-bin/"
#define BINARY_FILE_EXTENSIONS "png ico"

char *cgi_filepath_rewrite(char *filepath, int free_old_path);
int is_binary_file_extension(char *extension);
int file_accessible(char *filepath);
char *get_extension(char *filename);
void directory_listing(char *filepath);
void debug_check_privsep(char *when);
int ptr_free_ifnotnull(int num_ptr, ...);

#endif