// -----------------------
// MISCELLANEOUS FUNCTIONS
// -----------------------
#include <stdarg.h> // "varargs" for ptr_free_ifnotnull(int, ...)
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>   // directory_listing()
#include <sys/stat.h> // directory_listing()

#include "misc.h"

// PURPOSE: free() a bunch of pointers of any type in one call!
// num_ptr is the number of pointers you have as arguments
// eg. ptr_free_ifnotnull(3, &ptr1, &ptr2, &ptr3);
//
// OBS OBS!! =====> ONLY use on pointers which have been initialised to NULL
//
// checks for    NULL before free()
// sets pointer  NULL after  free()
//
//  returns: number of pointers that were freed    (wont free NULL pointers)
int ptr_free_ifnotnull(int num_ptr, ...)
{
    va_list ap;
    void **ptr;
    int i;
    int num_freed = 0;
    va_start(ap, num_ptr);

    for (i = 0; i < num_ptr; i++)
    {
        ptr = va_arg(ap, void *);

        if (*ptr != NULL)
        {
            num_freed++;
            free(*ptr);
            *ptr = NULL;
        }
    }

    va_end(ap);
    return num_freed;
}

// copied from lecture and translated to english to conform with rest of project
void directory_listing(char *filepath)
{
    struct stat stat_buffer;
    struct dirent *ent;
    DIR *dir;

    if ((dir = opendir(filepath)) == NULL)
    {
        perror("");
        exit(EXIT_FAILURE);
    }

    chdir(filepath);

    printf("Directory %s:\n", filepath);
    printf("------------------------------------\n");
    printf("Permissions\tUID\tGID\tFile\n");
    printf("------------------------------------\n");

    while ((ent = readdir(dir)) != NULL)
    {

        if (stat(ent->d_name, &stat_buffer) < 0)
        {
            perror("");
            exit(2);
        }

        printf("%o\t\t", stat_buffer.st_mode & 0777);
        printf("%d\t", stat_buffer.st_uid);
        printf("%d\t", stat_buffer.st_gid);
        printf("%s\n", ent->d_name);
    }

    closedir(dir);
}

// "filename" _can_ be a filepath
char *get_extension(char *filename)
{
    char *extension;
    char *dot;
    char *slash;

    if (filename == NULL)
        return NULL;
    slash = strrchr(filename, '/');

    // no slash
    if (!slash)
        dot = strrchr(filename, '.');

    // slash (example dir: /www.testdir/test)
    else
        dot = strrchr(slash, '.');

    if (!dot)
        return NULL;

    else
    {
        // dot is last character?
        if (*(dot + 1) == '\0')
            return NULL;

        extension = malloc(256 * sizeof(char));
        strncpy(extension, ++dot, 255);
        return extension;
    }
}

// converts /cgi-bin/cgiscript
// to       /var/www/cgi-bin/cgiscript
//
//          returns (char *) to the new path
//          returns NULL if old_path is not /cgi-bin/<something>
//
// if free_old_path = 1  then  free(old_path)
char *cgi_filepath_rewrite(char *old_path, int free_old_path)
{
    int len;
    char *new_path = NULL;

    if (sscanf(old_path, "/cgi-bin/%s", old_path) != 1)
        return NULL;

    // PRE_CHROOT_CGI_BIN_PATH = /var/www/cgi-bin/
    len = strlen(PRE_CHROOT_CGI_BIN_PATH) + strlen(old_path) + 1;
    new_path = malloc(len * sizeof(char));

    // int snprintf(char *str, size_t size, const char *format, ...);
    // str = buff, size = max bytes, format : C string that contains a format
    // example “%d%d”, myint, myint2
    snprintf(new_path, len, "%s%s", PRE_CHROOT_CGI_BIN_PATH, old_path);

    if (free_old_path == 1 && old_path != NULL)
        free(old_path);

    return new_path;
}

// check if setup_privsep() is working
void debug_check_privsep(char *when)
{
    if (file_accessible("/etc/shadow") == 1)
        fprintf(stderr, "DEBUG: /etc/shadow is accessible %s privsep()\n", when);
    else
        fprintf(stderr, "DEBUG: /etc/shadow NOT accessible %s privsep()\n", when);
}

// for use with BINARY_FILE_EXTENSIONS
// in our case, checking for PNG files
int is_binary_file_extension(char *ext)
{
    char curr_ext[256];
    char *extensions = BINARY_FILE_EXTENSIONS;
    int pos;

    // no extension
    if (ext == NULL)
        return 0;

    while (sscanf(extensions, "%255s%n", curr_ext, &pos) == 1)
    {
        extensions += pos;

        // match in BINARY_FILE_EXTENSIONS
        if (strcasecmp(ext, curr_ext) == 0)
            return 1;
    }

    // no match in BINARY_FILE_EXTENSIONS
    return 0;
}

// Determines to which degree file is "accessible"
// filepath can be relative or absolute
//
// return  1 if file    exists &  readable
// return  0 if file    exists & !readable
// return -1 if file   !exists
int file_accessible(char *filepath)
{
    // exists
    if (access(filepath, F_OK) == 0)
    {
        // readable
        if (access(filepath, R_OK) == 0)
            return 1;

        // !readable
        else
            return 0;
    }

    // !exists
    else
        return -1;
}