#include "systemcalls.h"

/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{

/*
 * TODO  add your code here
 *  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success
 *   or false() if it returned a failure
*/
    //create variable
    int retval_int = 0;

    //call the system() function
    retval_int = system(cmd);
    //check whether the process terminated normally
    if(WIFEXITED(retval_int)){
        //check whether the command invoked successfully
        if(WEXITSTATUS(retval_int) == 0) {
            return true;
        } 
    }
    return false;
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
    command[count] = command[count];

/*
 * TODO:
 *   Execute a system command by calling fork, execv(),
 *   and wait instead of system (see LSP page 161).
 *   Use the command[0] as the full path to the command to execute
 *   (first argument to execv), and use the remaining arguments
 *   as second argument to the execv() command.
 *
*/
    //create variables
    int status_int = 0;
    pid_t child_pid = 0;
    bool retval_b = false;
    //check intput parameters
    //no parameter
    if (count == 0) {
        printf("Error: invalid paremeters\n");
    } else {
        //create a child process
        child_pid = fork();
        if (child_pid == -1) { //error in creating a child
            perror("fork");
        } else if(child_pid == 0) { //success
            //create variable
            int retval_int = 0;
            char *p_path_ch = command[0];
            char *par_args_ch[count];
            //get the arguments
            for(i=1; i<=count; i++) {
                par_args_ch[i-1] = command[i];
            }
            //execute the command
            retval_int = execv(p_path_ch,par_args_ch);
            if (retval_int == -1) { //error
                perror ("execv");
                exit(EXIT_FAILURE);
            } else {
                exit(EXIT_SUCCESS);
            }
        }
        //in the parent
        child_pid = wait(&status_int);
        //error
        if (child_pid == -1){
            perror("wait");
        } else {
                    //check if the child terminated normally
            if(WIFEXITED(status_int)) {
                //if the exitstatus is success
                if(WEXITSTATUS(status_int) == EXIT_SUCCESS){
                    retval_b = true;
                }
            }
        }
    }
    
    va_end(args);    

    return retval_b;
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

    va_end(args);

    return true;
}
