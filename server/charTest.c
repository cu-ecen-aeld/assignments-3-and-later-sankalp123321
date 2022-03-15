/*
*   Mytest for custom driver
*   Author: Kamini Budke
*   Date: 02-12-2022
*   @ref: https://beej.us/guide/bgnet/html/
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <syslog.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>



#define MY_SOCK_PATH "/dev/aesdchar" 

int fd;
const char test_string[] = "Hello, this is my test\n";

int main(int argc, char *argv[])
{
    fd = open(MY_SOCK_PATH, O_CREAT | O_RDWR | O_APPEND, 0644);
    if (fd < 0)
    {
        perror("open failed");
        exit(EXIT_FAILURE);
    }
    printf("Open sucessful\n");
    int bytes = write(fd, test_string, sizeof(test_string));
    if (bytes < 0)
    {
        perror("write\n");
        _exit(EXIT_FAILURE);
    }
    if(bytes == strlen(test_string))printf("Write sucessful\n");
    char recv_test[100];
    bytes = read(fd, recv_test, sizeof(recv_test));
    if (bytes < 0)
    {
        perror("read\n");
        _exit(EXIT_FAILURE);
    }
    //if(bytes == sizeof(test_string))printf("String read is: %s \n", recv_test);
    printf("String read is: %s \n", recv_test);
    printf("Enf of test\n");

    close(fd);
}
