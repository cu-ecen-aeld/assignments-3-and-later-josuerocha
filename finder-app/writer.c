#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <syslog.h> 
#define _GNU_SOURCE
#include <string.h>

struct stat st = {0};

int main(int argc, char *argv[] )  {
    openlog("ELCB-Writer", LOG_PID, LOG_USER);
    if(argc != 3) {
        syslog (LOG_ERR,"Not enough arguments supplied (argc=%d)\n", argc);
        printf ("Not enough arguments supplied (argc=%d)\n", argc);
        syslog (LOG_ERR,"Usage: expecting 2 arguments\n");
        syslog (LOG_ERR,"- filepath: char array - path to file that will be written\n");
        syslog (LOG_ERR,"- content: char array - content to be written into file\n");
        closelog();
        return 1;
    }

    char* filepath = argv[1];
    char* content = argv[2];

    if(strcmp(filepath,"\0") == 0 || strcmp(argv[1],"\0") == 0){
        syslog(LOG_ERR,"Empty argument supplied");
        closelog();
        return 1;
    }

    char* folderpath = dirname(strdup(filepath));

    if (stat(folderpath, &st) == -1) {
        mkdir(folderpath, 0700);
    }

    FILE *fp = fopen(filepath, "w");
    if (fp != NULL)
    {
        fputs(content, fp);
        fclose(fp);
        syslog(LOG_DEBUG, "Writing %s to %s\n", content, filepath);
    }

    closelog();
    return 0;
}