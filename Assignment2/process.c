#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
// https://www.codequoi.com/en/creating-and-killing-child-processes-in-c/#what-is-a-process

pid_t pid = -1;

void child_signal_handler(int sig){
    
    if (sig == SIGCONT){
        return;
    }
    else {
        exit(EIO);
    }
}

void parent_signal_handler(int sig){
    if (sig == SIGINT){
        if (pid > 0){
            kill(pid, SIGINT);
        }
        exit(EOWNERDEAD);
    }
}

int main(){

    pid = fork();

    if (pid < 0){
        exit(ECHILD);
    }

    if (pid == 0){
        // I am the child process
        printf("%d\n",pid);
        signal(SIGCONT, child_signal_handler);
        signal(SIGINT, child_signal_handler);

        pause();

        printf("resuming...\n");
        exit(EIO);
    }
    else{
        // Fork's return value is not 0
        // which means we are in the parent process
        signal(SIGINT, parent_signal_handler);

        // is there another way that I should suspend the parent process to give the child time to print?
        sleep(2);

        kill(pid, SIGCONT);
        int status;
        
        wait(&status);

        if (WIFEXITED(status)){
            printf("childpid=%d,exitstatus=%d\n", pid, WEXITSTATUS(status));
        }
        else{
            printf("child process didn't exit?\n");
        }

        exit(EXIT_SUCCESS);
    }
    
    
}