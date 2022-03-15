#include <stdio.h>
#include <syslog.h>
#include <stdlib.h>

int main(int argc, char*argv[])
{
    // Enable syslogging, the identifier will be "writer" along with PID.
    openlog("writer", LOG_PID|LOG_CONS|LOG_NDELAY, LOG_USER);

    // If -h option is provided, display the help menu.
    if(argc == 2)
    {
        // Option -h check
        if(argv[1][0] == '-' && argv[1][1] == 'h')
        {
            printf("Usage:\n");
            printf("./writer-app <file> <string>\n");
            printf("    1. File name and path of the file.\n");
            printf("    2. String to be written into the file.\n");
            return 0;
        }
    }

    // 3 arguments are a must.
    if(argc != 3)
    {
        // log error
        syslog(LOG_ERR, "Not enough arguments. Press -h option for help\n");
        return 1;
    }

    // Get the file path
    char* file_path = argv[1];
    // Get the string to be written into the file
    char* str = argv[2];

    // Open the file.
    FILE* file = fopen(file_path, "w+");
    if(file == NULL)
    {
        // log error
        syslog(LOG_ERR, "File could not be opened.\n");
        return 1;
    }
    // Log success
    syslog(LOG_DEBUG, "File opened: %s.\n", file_path);
    // Store the stringg in the file
    fputs(str, file);
    // Close the file.
    fclose(file);
    // Close logging.
    closelog();

    // Return Success.
    return 0;
}
