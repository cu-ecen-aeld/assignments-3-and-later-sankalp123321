/**
 * @file aesdsocketThread.c
 * @author sankalp
 * @brief
 * @version 0.1
 * @date 2022-02-23
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
#include <pthread.h>
#include <time.h>
#include "queue.h"

#undef _DEBUG_

#define IP_MAX_LEN 100
#define MAX_BUFFER_SIZE 1024
#define TEMP_FILE "/var/tmp/aesdsocketdata"
#define ALLOWED_BACKLOG_CONNS 10
#define PORT "9000"

static int write_to_fd = 0;

pthread_mutex_t *gmutex = NULL;
struct ThreadIDStruct
{
    pthread_t tID;
    uint8_t isProcessingComplete;
    int clienFd;
    int fileFd;
    const char *strip;
    TAILQ_ENTRY(ThreadIDStruct) nextThread;
};

typedef TAILQ_HEAD(head_s, ThreadIDStruct) ThreadHead;
ThreadHead threadHead;

pthread_t threadCleanUp = 0;
pthread_t threadTimeStamp = 0;
int server_fd = 0;

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}


// Removes all of the elements from the queue before free()ing them.
static void freeQueue(ThreadHead * head)
{
    struct ThreadIDStruct * e = NULL;
    while (!TAILQ_EMPTY(head))
    {
        e = TAILQ_FIRST(head);
        TAILQ_REMOVE(head, e, nextThread);
        free(e);
        e = NULL;
    }
}


void gracefulShutdonw()
{
    int ret = EXIT_SUCCESS;

    printf("Caught Signal, exiting\n");

    pthread_kill(threadCleanUp, SIGKILL);
    pthread_kill(threadTimeStamp, SIGKILL);

    struct ThreadIDStruct *peruse = NULL;
    TAILQ_FOREACH(peruse, &threadHead, nextThread)
    {
        int ret = shutdown(peruse->clienFd, SHUT_RDWR);
        if (ret)
        {
            syslog(LOG_ERR, "shutdown %s", strerror(errno));
            _exit(EXIT_FAILURE);
        }
        ret = pthread_kill((pthread_t)peruse->tID, SIGKILL);
        if (ret)
        {
            syslog(LOG_ERR, "pthread_kill %s", strerror(errno));
            _exit(EXIT_FAILURE);
        }
    }

    freeQueue(&threadHead);

    pthread_mutex_destroy(gmutex);

    if (shutdown(server_fd, SHUT_RDWR))
        ret = EXIT_FAILURE;

    if (close(write_to_fd))
        ret = EXIT_FAILURE;

    unlink(TEMP_FILE);
    closelog();
    _exit(ret);
}

void cleanBeforeExit(int fd1, int fd2)
{
    remove(TEMP_FILE);
    close(fd1);
    close(fd2);
}

void *connectionThread(void *args)
{
    struct ThreadIDStruct *threadStruct = (struct ThreadIDStruct *)args;
    printf("STarting thread %ld\n", threadStruct->tID);
    uint8_t *buf = malloc(MAX_BUFFER_SIZE);
    if (buf == NULL)
    {
        syslog(LOG_ERR, "ENOMEM");
        // // cleanBeforeExit(write_to_fd, server_fd);
        // return EXIT_FAILURE;
    }
    uint16_t buffer_size = MAX_BUFFER_SIZE;
    uint16_t total_buf_size = 0, realloc_count = 1;
    memset(buf, 0, buffer_size);
    uint8_t shouldSend = 1;
    while (shouldSend)
    {
        uint8_t found_a_packet = 0;
        int ret_val = recv(threadStruct->clienFd, &buf[total_buf_size], buffer_size, 0);
        if (!ret_val)
        {
            // No more data to recv
            break;
        }
        else if (ret_val > 0)
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
            for (i = total_buf_size; i < (total_buf_size + ret_val); i++)
            {
                if (buf[i] == '\n')
                {
                    found_a_packet = 1;
                    pthread_mutex_lock(gmutex);
                    int ret = write(write_to_fd, buf, strlen((char *)buf));
                    if (ret == strlen((char *)buf))
                    {
#ifdef _DEBUG_
                        printf("success in writing %ld bytes.\n", strlen((char *)buf));
#endif
                        free(buf);
                    }
                    shouldSend = 0;
                    break;
                }
                found_a_packet = 0;
            }
            if (!found_a_packet)
            {
                realloc_count++;
                if ((buf = realloc(buf, realloc_count * buffer_size)) != NULL)
                {
                    syslog(LOG_ERR, "ENOMEM");
                    free(buf);
                    // cleanBeforeExit(write_to_fd, server_fd);
                    // return EXIT_FAILURE;
                }
                total_buf_size += buffer_size;

#ifdef _DEBUG_
                printf("Allocated more memory. %d\n", total_buf_size);
#endif
            }
        }
        else if (ret_val < 0)
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
                syslog(LOG_ERR, "recv: %s", strerror(errno));
                free(buf);
                // cleanBeforeExit(write_to_fd, server_fd);
                // return EXIT_FAILURE;
            }
            break;
            default:
                break;
            }
        }
    }
    uint8_t buffer[MAX_BUFFER_SIZE];
    buffer_size = MAX_BUFFER_SIZE;
    lseek(write_to_fd, 0, SEEK_SET);
    while (1)
    {
        memset(buffer, 0, buffer_size);
        int read_Val = read(write_to_fd, buffer, buffer_size);
        if (!read_Val)
        {
            break;
        }
        if (read_Val < 0)
        {
            syslog(LOG_ERR, "read: %s", strerror(errno));
            // cleanBeforeExit(write_to_fd, server_fd);
            // return EXIT_FAILURE;
        }

#ifdef _DEBUG_
        printf("Sending: %d bytes\n", read_Val);
        for (int i = 0; i < read_Val; i++)
        {
            printf("%c", buffer[i]);
        }
        printf("\n");
#endif
        int ret_val = send(threadStruct->clienFd, buffer, read_Val, 0);
        if (read_Val != ret_val)
        {
            syslog(LOG_ERR, "read_Val[%d] actual[%d] send: %s", read_Val, ret_val, strerror(errno));
            // cleanBeforeExit(write_to_fd, server_fd);
            // return EXIT_FAILURE;
        }

#ifdef _DEBUG_
        printf("send bytes: %d\n", ret_val);
#endif
    }
    threadStruct->isProcessingComplete = 1;
    pthread_mutex_unlock(gmutex);
    // pthread_mutex_unlock(gmutex);
    
    syslog(LOG_INFO, "Closed connection from %s\n", threadStruct->strip);

#ifdef _DEBUG_
    printf("Closed connection fd[%d] %s\n", threadStruct->clienFd, threadStruct->strip);
#endif
    return NULL;
}

void *connectionCleanUpThread(void *args)
{
    ThreadHead *tHead = args;
    static uint8_t count = 0;
    struct ThreadIDStruct *peruse = NULL;
    while (1)
    {
        TAILQ_FOREACH(peruse, tHead, nextThread)
        {
            peruse = TAILQ_FIRST(tHead);
            if (peruse->isProcessingComplete)
            {
                shutdown(peruse->clienFd, SHUT_RDWR);
                // printf("Joining thread[%ld]\n", peruse->tID);
                int ret = pthread_join(peruse->tID, NULL);
                if (ret)
                {

                }
                TAILQ_REMOVE(tHead, peruse, nextThread);
                printf("CLeaning thread [%d] iscomplete [%d] count[%d]\n", \
                    peruse->clienFd, peruse->isProcessingComplete, count++);
                free(peruse);
                peruse = NULL;
                break;
            }
        }
        usleep(10);
    }
    return NULL;
}

void *timeStampUpdater(void *args)
{
    while (1)
    {
        sleep(10);
        char outstr[200];
        memset(outstr, 0, sizeof outstr);
        const char *str = "timestamp:";
        time_t tim;
        struct tm *tmp = NULL;

        tim = time(NULL);
        tmp = localtime(&tim);
        if (tmp == NULL)
        {
            syslog(LOG_ERR, "time: %s", strerror(errno));
        }

        if (strftime(outstr, sizeof outstr, "%a, %d %b %Y %T", tmp) == 0)
        {
            // error
        }
        // if(tmp != NULL)
        // {
        //     free(tmp);
        // }

        if (write_to_fd < 0)
            continue;
        pthread_mutex_lock(gmutex);
        outstr[strlen(outstr)] = '\n';
        int ret = write(write_to_fd, str, strlen(str));
        ret = write(write_to_fd, outstr, strlen(outstr));
        // ret = write(write_to_fd, (const char*)'\n', 1);
        if (ret < 0)
        {
            syslog(LOG_ERR, "write: %s", strerror(errno));
        }
        pthread_mutex_unlock(gmutex);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    printf("Server application.\n");

    pthread_mutex_t mutex;
    TAILQ_INIT(&threadHead);

    signal(SIGINT, gracefulShutdonw);
    signal(SIGKILL, gracefulShutdonw);
    signal(SIGSTOP, gracefulShutdonw);
    signal(SIGTERM, gracefulShutdonw);

    openlog("aesdsocket", LOG_CONS | LOG_PERROR | LOG_PID, LOG_USER);

    write_to_fd = open(TEMP_FILE, O_RDWR | O_CREAT | O_APPEND);
    if (write_to_fd < 0)
    {
        syslog(LOG_ERR, "open file: %s", strerror(errno));
        return EXIT_FAILURE;
    }

    int mutex_ret = pthread_mutex_init(&mutex, NULL);
    if (mutex_ret != 0)
    {
        syslog(LOG_ERR, "mutex: %s", strerror(errno));
    }
    gmutex = &mutex;
    struct addrinfo s_addr, *new_addr;
    struct sockaddr_storage client_addr;
    memset(&s_addr, 0, sizeof(s_addr));
    s_addr.ai_family = AF_INET;
    s_addr.ai_socktype = SOCK_STREAM;
    s_addr.ai_flags = AI_PASSIVE;

    int ret = getaddrinfo(NULL, PORT, &s_addr, &new_addr);
    if (ret != 0)
    {
        remove(TEMP_FILE);
        syslog(LOG_ERR, "getaddrinfo error %s\n", gai_strerror(ret));
        close(write_to_fd);
        return EXIT_FAILURE;
    }

#ifdef _DEBUG_
    printf("Opening the socket.\n");
#endif

    server_fd = socket(s_addr.ai_family, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        remove(TEMP_FILE);
        syslog(LOG_ERR, "socket: %s", strerror(errno));
        close(write_to_fd);
        freeaddrinfo(new_addr);
        return EXIT_FAILURE;
    }
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
    {
        syslog(LOG_ERR, "socketopt: %s", strerror(errno));
        close(write_to_fd);
        freeaddrinfo(new_addr);
        exit(EXIT_FAILURE);
    }

#ifdef _DEBUG_
    printf("Binding the file descriptor[%d].\n", server_fd);
#endif

    struct addrinfo *tmp = NULL;
    int bind_ret = 0, binded = 0;
    for (tmp = new_addr; tmp != NULL; tmp = tmp->ai_next)
    {
        bind_ret = bind(server_fd, tmp->ai_addr, new_addr->ai_addrlen);
        if (bind_ret != -1)
        {
            binded = 1;
            break;
        }
    }

    if (!binded)
    {
        syslog(LOG_ERR, "bind: %s", strerror(errno));
        freeaddrinfo(new_addr);
        return EXIT_FAILURE;
    }

    freeaddrinfo(new_addr);

    if (argc == 2)
    {
        if (strncmp((const char *)argv[1], "-d", 2) != 0)
        {
            syslog(LOG_ERR, "Incorrect argument.\n");
        }
        else
        {
            syslog(LOG_ERR, "Daemonizing Parent uid[%d] pgid[%d] gid[%d].\n", getpid(), getpgid(getpid()), getgid());
            int ret_val = daemon(0, 0);
            if (ret_val < 0)
            {
                syslog(LOG_ERR, "daemon: %s", strerror(errno));
                cleanBeforeExit(write_to_fd, server_fd);
                return EXIT_FAILURE;
            }
        }
    }

    int ret_pthread = pthread_create(&threadCleanUp, NULL, connectionCleanUpThread, &threadHead);
    ret_pthread = pthread_create(&threadTimeStamp, NULL, timeStampUpdater, NULL);
    while (1)
    {
        static uint8_t count = 0;
#ifdef _DEBUG_
        printf("Listening on the socket.\n");
#endif

        int listen_ret = listen(server_fd, ALLOWED_BACKLOG_CONNS);
        if (listen_ret < 0)
        {
            syslog(LOG_ERR, "listen: %s", strerror(errno));
            cleanBeforeExit(write_to_fd, server_fd);
            return EXIT_FAILURE;
        }
        socklen_t add_size = sizeof(client_addr);
        struct sockaddr *sck_addr = (struct sockaddr *)&client_addr;

#ifdef _DEBUG_
        printf("Accpet new connection.\n");
#endif

        int new_fd = accept(server_fd, sck_addr, &add_size);
        if (new_fd < 0)
        {
            syslog(LOG_ERR, "accept: %s", strerror(errno));
            cleanBeforeExit(write_to_fd, server_fd);
            return EXIT_FAILURE;
        }

        char str_ip[IP_MAX_LEN];
        const char *ret = inet_ntop(s_addr.ai_family, get_in_addr((struct sockaddr *)&client_addr), str_ip, IP_MAX_LEN);
        if (ret == NULL)
        {
            syslog(LOG_ERR, "inet_ntop: %s", strerror(errno));
            cleanBeforeExit(write_to_fd, server_fd);
            return EXIT_FAILURE;
        }

        syslog(LOG_INFO, "Accepted connection from %s\n", str_ip);

#ifdef _DEBUG_
        printf("New connection fd[%d] %s\n", new_fd, str_ip);
#endif

        // Malloc a new node
        struct ThreadIDStruct *tStruct = malloc(sizeof(struct ThreadIDStruct));
        if (tStruct == NULL)
        {
#ifdef _DEBUG_
            printf("Memory could not be allocated\n");
#endif
             return EXIT_FAILURE;
        }
        tStruct->clienFd = new_fd;
        tStruct->isProcessingComplete = 0;
        tStruct->tID = 0;
        tStruct->strip = (const char *)&str_ip;
        // Create a thread here
        pthread_t thread;
        ret_pthread = pthread_create(&thread, NULL, connectionThread, tStruct);
        if (ret_pthread != 0)
        {
#ifdef _DEBUG_
            printf("Thread creation error %d\n", ret_pthread);
#endif
        }
        tStruct->tID = thread;
        printf("ThreadCreat COunt[%d]", count++);
        TAILQ_INSERT_TAIL(&threadHead, tStruct, nextThread);
        // close(new_fd);
    }
    cleanBeforeExit(write_to_fd, server_fd);
    return EXIT_SUCCESS;
}
