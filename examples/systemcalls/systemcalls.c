#include "systemcalls.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>

/**
 * @param cmd the command to execute with system()
 * @return true if the commands in ... with arguments @param arguments were executed 
 *   successfully using the system() call, false if an error occurred, 
 *   either in invocation of the system() command, or if a non-zero return 
 *   value was returned by the command issued in @param.
*/
bool do_system(const char *cmd)
{

/*
 * TODO  add your code here
 *  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success 
 *   or false() if it returned a failure
*/
    int ret_val = system(cmd);
    if(ret_val == -1)
    {
        return false;
    }
    return true;
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the 
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    // command[count] = command[count];

/*
 * TODO:
 *   Execute a system command by calling fork, execv(),
 *   and wait instead of system (see LSP page 161).
 *   Use the command[0] as the full path to the command to execute
 *   (first argument to execv), and use the remaining arguments
 *   as second argument to the execv() command.
 *   
*/
    openlog("syscall", LOG_PID|LOG_NDELAY, LOG_USER);
    int status = 0;
    pid_t fork_ret_val = fork();
    if(fork_ret_val == -1)
    {
        syslog(LOG_ERR, "Fork_failed: %s", strerror(errno));
        return false;
    }
    else if(!fork_ret_val) // Child process
    {
        int exec_ret_val = execv(command[0], command);
        if(exec_ret_val == -1)
        {
            syslog(LOG_ERR, "Exec_failed: %s command[0]=%s", strerror(errno), command[0]);
            exit(EXIT_FAILURE);
        }
    }
    else // Parent process
    {
        pid_t pid = waitpid(fork_ret_val, &status, 0);
        if(pid == -1)
        {
            syslog(LOG_ERR, "wait_error: %s", strerror(errno));
            return false;
        }
        else if(pid == fork_ret_val)
        {      
            if (WIFEXITED (status))
            {
                // printf ("Normal termination with exit status=%d status=%d\n", WEXITSTATUS (status), status);
                if( WEXITSTATUS (status) == EXIT_SUCCESS)
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }
    }

    va_end(args);
    closelog();

    return true;
}

/**
* @param outputfile - The full path to the file to write with command output.  
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];


/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *   
*/
    bool ret_status = true;
    openlog("syscall", LOG_PID|LOG_NDELAY, LOG_USER);

    // printf("ret_val = %d\n", do_exec(count, args));

    
    int status = 0;
    pid_t fork_ret_val = fork();
    if(fork_ret_val == -1)
    {
        syslog(LOG_ERR, "Fork_failed: %s", strerror(errno));
        ret_status = false;
    }
    else if(!fork_ret_val) // Child process
    {
        int outputFileDesc = open(outputfile, O_WRONLY|O_TRUNC|O_CREAT);
        if(outputFileDesc == -1)
        {
            syslog(LOG_ERR, "fd_error: %s", strerror(errno));
        }
        if(dup2(outputFileDesc, STDOUT_FILENO) < 0) 
        {
            syslog(LOG_ERR, "dup2_error: %s", strerror(errno));
        }

        close(outputFileDesc);
        syslog(LOG_ERR, "command[0]=%s", command[0]);
        int exec_ret_val = execv(command[0], command);
        if(exec_ret_val == -1){}
        {
            syslog(LOG_ERR, "Exec_failed: %s command[0]=%s", strerror(errno), command[0]);
            exit(EXIT_FAILURE);
        }
    }
    else // Parent process
    {
        pid_t pid = waitpid(fork_ret_val, &status, 0);
        if(pid == -1)
        {
            syslog(LOG_ERR, "wait_error: %s", strerror(errno));
            ret_status = false;
        }
        else if(pid == fork_ret_val)
        {      
            if (WIFEXITED (status))
            {
                printf ("Normal termination with exit status=%d status=%d\n", WEXITSTATUS (status), status);
                if( WEXITSTATUS (status) == EXIT_SUCCESS)
                {
                    ret_status = true;
                }
                else
                {
                    ret_status = false;
                }
            }
        }
    }
    // close(outputFileDesc);
    closelog();

    va_end(args);
    
    return ret_status;
}
