/**
 * @file server.c
 * @author Sankalp Agrawal
 * @brief 
 * @version 0.1
 * @date 2022-02-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define _DEBUG_

#define ALLOWED_BACKLOG_CONNS 10
#define PORT "9000"

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void gracefulShutdonw()
{
    syslog(LOG_INFO, "Caught Signal, exiting\n");
    remove("/var/tmp/aesdsocketdata");
    _exit(0);
}

int main(int argc, char* argv[])
{
    printf("Server application.\n");
    signal(SIGINT, gracefulShutdonw);
    signal(SIGKILL, gracefulShutdonw);
    signal(SIGSTOP, gracefulShutdonw);
    signal(SIGTERM, gracefulShutdonw);
    
    openlog("aesdsocket", LOG_CONS|LOG_PERROR|LOG_PID, LOG_USER);

    int write_to_fd = open("/var/tmp/aesdsocketdata", O_RDWR|O_CREAT|O_APPEND, S_IRWXU|S_IRWXO);
    if(write_to_fd < 0)
    {
        syslog(LOG_ERR, "open file: %s", strerror(errno));
        return EXIT_FAILURE;
    }

    struct addrinfo s_addr, *new_addr;
    struct sockaddr_storage client_addr;
    memset(&s_addr, 0, sizeof(s_addr));
    s_addr.ai_family = AF_INET;
    s_addr.ai_socktype = SOCK_STREAM;
    s_addr.ai_flags = AI_PASSIVE;
    int ret = getaddrinfo(NULL, PORT, &s_addr, &new_addr);
    if(ret != 0)
    {
        remove("/var/tmp/aesdsocketdata");
        syslog(LOG_ERR, "getaddrinfo error %s\n", gai_strerror(ret));
        close(write_to_fd);
        return EXIT_FAILURE;
    }
#ifdef _DEBUG_
    printf("Opening the socket.\n");
#endif
    int server_fd = socket(s_addr.ai_family, SOCK_STREAM, 0);
    if(server_fd < 0)
    {
        remove("/var/tmp/aesdsocketdata");
        // perror("socket");
        syslog(LOG_ERR, "socket: %s", strerror(errno));
        close(write_to_fd);
        freeaddrinfo(new_addr);
        return EXIT_FAILURE;
    }
#ifdef _DEBUG_
    printf("Binding the file descriptor[%d].\n", server_fd);
#endif
    int bind_ret = bind(server_fd, new_addr->ai_addr, new_addr->ai_addrlen);
    if(bind_ret < 0)
    {
        remove("/var/tmp/aesdsocketdata");
        // perror("bind");
        syslog(LOG_ERR, "bind: %s", strerror(errno));
        close(write_to_fd);
        close(server_fd);
        freeaddrinfo(new_addr);
        return EXIT_FAILURE;
    }

    if(argc == 2)
    {
        if(strncmp((const char*)argv[1], "-d", 2) != 0)
        {
            syslog(LOG_ERR, "Incorrect argument.\n");
        }
        else
        {
            syslog(LOG_ERR, "Daemonizing Parent uid[%d] pgid[%d] gid[%d].\n", getpid(), getpgid(getpid()), getgid());
            int ret_val = daemon(0, 0);
            if(ret_val < 0)
            {
                remove("/var/tmp/aesdsocketdata");
                syslog(LOG_ERR, "daemon: %s", strerror(errno));
                close(write_to_fd);
                close(server_fd);
                freeaddrinfo(new_addr);
                return EXIT_FAILURE;
            }
        }
    }

    while (1)
    {
#ifdef _DEBUG_
        printf("Listening on the socket.\n");
#endif
        int listen_ret = listen(server_fd, ALLOWED_BACKLOG_CONNS);
        if(listen_ret < 0)
        {
            // perror("listen");
            remove("/var/tmp/aesdsocketdata");
            syslog(LOG_ERR, "listen: %s", strerror(errno));
            close(write_to_fd);
            close(server_fd);
            freeaddrinfo(new_addr);
            return EXIT_FAILURE;
        } 
        socklen_t add_size = sizeof(client_addr);
        struct sockaddr* sck_addr = (struct sockaddr*)&client_addr;
#ifdef _DEBUG_
        printf("Accpet new connection.\n");
#endif
        int new_fd = accept(server_fd, sck_addr, &add_size);
        if(new_fd < 0)
        {
            // perror("accept");
            remove("/var/tmp/aesdsocketdata");
            syslog(LOG_ERR, "accept: %s", strerror(errno));
            close(write_to_fd);
            close(server_fd);
            freeaddrinfo(new_addr);
            return EXIT_FAILURE;
        }
        
        char str_ip[100];
        inet_ntop(s_addr.ai_family , get_in_addr((struct sockaddr *)&client_addr), str_ip, sizeof str_ip);
        
        syslog(LOG_INFO, "Accepted connection from %s\n", str_ip);
#ifdef _DEBUG_
        printf("New connection fd[%d] %s\n", new_fd, str_ip);
#endif
        uint8_t *buf = (uint8_t*)malloc(1024);
        if(buf == NULL)
        {
            remove("/var/tmp/aesdsocketdata");
            syslog(LOG_ERR, "ENOMEM");
            close(write_to_fd);
            close(server_fd);
            freeaddrinfo(new_addr);
            return EXIT_FAILURE;
        }

        uint16_t buffer_size = 1024;
        uint16_t total_buf_size = 0, realloc_count = 1;
        memset(buf, 0, buffer_size);

        while (1)
        {
            uint8_t found_a_packet = 0;
            int ret_val = recv(new_fd, &buf[total_buf_size], buffer_size, 0);
            if (!ret_val)
            {
                // No more data to recv
                break;
            }
            else if((buffer_size == ret_val) || (buffer_size != ret_val))
            {
#ifdef _DEBUG_
                printf("\nrecvd: %d bytes\n", ret_val);

                for (size_t i = 0; i < ret_val; i++)
                {
                    printf("%c", buf[i]);
                }
                printf("\n");
#endif
                size_t i = 0;
                for (i = total_buf_size; i < (total_buf_size+ret_val); i++)
                {
                    if(buf[i] == '\n')
                    {
                        found_a_packet = 1;
                        int ret = write(write_to_fd, buf, strlen((char*)buf));
                        if(ret == strlen((char*)buf))
                        {
#ifdef _DEBUG_
                            printf("success in writing %ld bytes.\n", strlen((char*)buf));
#endif
                        }
                        break;
                    }
                    found_a_packet = 0;
                }

                if(!found_a_packet)
                {
                    realloc_count++;
                    buf = realloc(buf, realloc_count*buffer_size);
                    total_buf_size += buffer_size;
                    if(buf == NULL)
                    {
                        remove("/var/tmp/aesdsocketdata");
                        syslog(LOG_ERR, "ENOMEM");
                        free(buf);
                        close(write_to_fd);
                        close(server_fd);
                        freeaddrinfo(new_addr);
                        return EXIT_FAILURE;
                    }
#ifdef _DEBUG_
                    printf("Allocated more memory. %d\n", total_buf_size);
#endif
                }
                continue;
            }
            else if(ret_val < 0)
            {
                switch (errno)
                {
                case EINTR:
                    continue;
                    break;
                case ENOTSOCK:
                case ENOTCONN:
                case EFAULT:
                case EBADF:
                case ECONNREFUSED:
                    {
                        remove("/var/tmp/aesdsocketdata");
                        syslog(LOG_ERR, "recv: %s", strerror(errno));
                        free(buf);
                        close(write_to_fd);
                        close(server_fd);
                        freeaddrinfo(new_addr);
                        return EXIT_FAILURE;
                    }
                    break;
                default:
                    break;
                }
            }
        }
        uint8_t buffer[512];
        buffer_size = sizeof buffer/ sizeof buffer[0];
        lseek(write_to_fd, 0, SEEK_SET);
        while (1)
        {
            memset(buffer, 0, buffer_size);
            int read_Val = read(write_to_fd, buffer, buffer_size);
            if(!read_Val)
            {
                break;
            }
            if(read_Val < 0)
            {
                remove("/var/tmp/aesdsocketdata");
                syslog(LOG_ERR, "read: %s", strerror(errno));
                free(buf);
                close(write_to_fd);
                close(server_fd);
                freeaddrinfo(new_addr);
                return EXIT_FAILURE;
            }
#ifdef _DEBUG_
            syslog(LOG_INFO, "Sending: %d bytes\n", read_Val);
#endif
            int ret_val = send(new_fd, buffer, read_Val, 0);
            if(read_Val != ret_val)
            {
                remove("/var/tmp/aesdsocketdata");
                // perror("send");
                syslog(LOG_ERR, "read_Val[%d] actual[%d] send: %s", read_Val, ret_val, strerror(errno));
                free(buf);
                close(write_to_fd);
                close(server_fd);
                freeaddrinfo(new_addr);
                return EXIT_FAILURE;
            }
#ifdef _DEBUG_
            syslog(LOG_INFO, "send bytes: %d\n", ret_val);
#endif
        }
        close(new_fd);
        syslog(LOG_INFO, "Closed connection from %s\n", str_ip);
#ifdef _DEBUG_
        printf("Closed connection fd[%d] %s\n", new_fd, str_ip);
#endif
        free(buf);
    }
    remove("/var/tmp/aesdsocketdata"); 
    close(write_to_fd);
    close(server_fd);
    freeaddrinfo(new_addr);
    return EXIT_SUCCESS;
}