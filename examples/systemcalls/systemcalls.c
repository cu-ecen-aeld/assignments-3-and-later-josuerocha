#include "systemcalls.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{
    int code = system(cmd);
    bool is_success;
    if(code == 0) {
        is_success = true;
    } else {
        is_success = false;
    }
    return is_success;
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

/*
 *   Execute a system command by calling fork, execv(),
 *   and wait instead of system (see LSP page 161).
 *   Use the command[0] as the full path to the command to execute
 *   (first argument to execv), and use the remaining arguments
 *   as second argument to the execv() command.
 *
*/
    fflush(stdout);
    pid_t pid = fork();

    if(pid == -1) {
        // Failed to fork
        return false;
    } else if (pid > 0){
        // Parent
        int status;
        waitpid(pid, &status, 0);
    } {
        // Child has the control
        execv(command[0], command);
        return false;
    }

    va_end(args);

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


/*
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/
    fflush(stdout);
    pid_t pid = fork();
    int fd = open("redirected.txt", O_WRONLY|O_TRUNC|O_CREAT, 0644);
    if (fd < 0) { perror("open"); return false; }
    if(pid == -1) {
        // Failed to fork
        perror("fork");
        return false;
    } else if (pid > 0){
        // Parent
        int status;
        waitpid(pid, &status, 0);
    } {
        // Child has the control
        if (dup2(fd, 1) < 0) { perror("dup2"); return false; }
        close(fd);
        execv(command[0], command);
        return false;
    }

    va_end(args);
    close(fd);
    return true;
}
