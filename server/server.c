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
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>

int main(int argc, char* argv[])
{
    printf("Server application.\n");
    
    struct addrinfo s_addr, *new_addr;
    struct sockaddr_storage client_addr;
    memset(&s_addr, 0, sizeof(s_addr));
    s_addr.ai_family = AF_INET6;
    s_addr.ai_socktype = SOCK_STREAM;
    s_addr.ai_flags = AI_PASSIVE;
    int ret = getaddrinfo(NULL, "57600", &s_addr, &new_addr);
    if(ret != 0)
    {
        printf("getaddrinfo error %s\n", gai_strerror(ret));
        return EXIT_FAILURE;
    }

    printf("Opening the socket.\n");
    int server_fd = socket(PF_INET6, SOCK_STREAM, 0);
    if(server_fd < 0)
    {
        perror("socket");
    }

    printf("Binding the file descriptor[%d] to the address[%s].\n", server_fd, new_addr->ai_addr->sa_data);
    int bind_ret = bind(server_fd, new_addr->ai_addr, new_addr->ai_addrlen);
    if(bind_ret < 0)
    {
        perror("bind");
    }

    printf("Listening on the socket.\n");
    int listen_ret = listen(server_fd, 10);
    if(listen_ret < 0)
    {
        perror("listen");
    } 
    socklen_t add_size = sizeof(client_addr);
    printf("Accpet new connection.\n");
    int new_fd = accept(server_fd, (struct sockaddr*)&client_addr, &add_size);
    if(new_fd < 0)
    {
        perror("accept");
    } 

    printf("New connection fd[%d]\n", new_fd);
    uint8_t buffer[1024];
    uint16_t buffer_size = sizeof(buffer)/sizeof(buffer[0]);
    int ret_val = recv(new_fd, buffer, buffer_size, 0);
    if(buffer_size == ret_val)
    {
        perror("recv");
    }
    const char* str = "This is super good stuff\n"; 
    memcpy(buffer, str, strlen(str));
    ret_val = send(new_fd, buffer, buffer_size, 0);
    if(buffer_size == ret_val)
    {
        perror("send");
    }
    close(server_fd);
    freeaddrinfo(new_addr);
    return EXIT_SUCCESS;
}