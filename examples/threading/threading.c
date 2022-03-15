#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    thread_data_t* thread_func_args = (thread_data_t *) thread_param;
    usleep(thread_func_args->wait_delay*1000);
    pthread_mutex_lock(thread_func_args->mutex);
    usleep(thread_func_args->release_delay*1000);
    pthread_mutex_unlock(thread_func_args->mutex);
    thread_func_args->thread_complete_success = true;
    return (void*)thread_func_args;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     * 
     * See implementation details in threading.h file comment block
     */
    thread_data_t* m_thread_data = malloc(sizeof(thread_data_t));
    m_thread_data->mutex = mutex;
    m_thread_data->wait_delay = wait_to_obtain_ms;
    m_thread_data->release_delay = wait_to_release_ms;

    bool retval = pthread_create(thread, NULL, threadfunc, (void*)m_thread_data);
    if(retval != 0)
    {
        perror("thread");
        return false;
    }
    return true;
}

