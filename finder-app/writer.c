#include <stdio.h>
#include <syslog.h>
#include <stdlib.h>

int main(int argc, char*argv[])
{
    openlog("writer-app", LOG_PID|LOG_CONS|LOG_NDELAY, LOG_USER);

    if(argc == 2)
    {
        if(argv[1][0] == '-' && argv[1][1] == 'h')
        {
            printf("Usage:\n");
            printf("./writer-app <file> <string>\n");
            printf("    1. File name and path of the file.\n");
            printf("    2. String to be written into the file.\n");
            return 0;
        }
    }

    if(argc != 3)
    {
        // log error
        syslog(LOG_ERR, "Not enough arguments. Press -h option for help\n");
        return 1;
    }

    if((argv[0] == NULL) && (argv[1] == NULL)) 
    {
        // log error
        syslog(LOG_ERR, "Invalid Arguments. Press -h option for help\n");
        return 1;
    }

    char* file_path = argv[1];
    char* str = argv[2];

    if((str[0] == ' ') || (file_path[0] == ' '))
    {
        // log error
        syslog(LOG_ERR, "Empty String Passed.\n");
        return 1;
    }

    FILE* file = fopen(file_path, "w+");
    if(file == NULL)
    {
        // log error
        syslog(LOG_ERR, "File could not be opened.\n");
        return 1;
    }
    syslog(LOG_DEBUG, "File opened: %s.\n", file_path);
    fputs(str, file);
    fclose(file);
    closelog();
    return 0;
}
