#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <unistd.h>  
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>


int i, n;
pid_t* children = NULL;

int errorIncorrectUsage(int err){
    printf("\nUsage: ./zombifier -n [Number of Zombies, int >= 1] \n");
    exit(err);
}

void child_signal_handler(int sig){
    
    if (sig == SIGCONT){
        return;
    }
    if (sig == SIGINT) {
        printf("zombie interupted\n");

        exit(EIO);
    }
}

void parent_signal_handler(int sig){
    
    if (sig == SIGCONT){
        
        return;
    }
    if (sig == SIGINT){

        printf("process interrupted, cleaning up\n");
        
        for (i = 0; i < n; i ++){
            kill(children[i], SIGINT);
        }        

        free(children);

        exit(EOWNERDEAD);
    }
}

int childFunction(){
    // I am the child process

    signal(SIGCONT, child_signal_handler);
    signal(SIGUSR1, child_signal_handler);

    printf("Child pid %d\n", getpid());

    pause();

    printf("Child exiting gracefully\n");
    exit(EIO);
}

int main(int argc, char** argv){

    // input validation
    int opt, temp;
    extern int optind;
    extern char* optarg;
    
    
    while((opt = getopt(argc, argv, "n:")) != -1){

        switch(opt){
            case 'n':
                temp = atoi(optarg);
                // enforce integer
                if (temp == 0 && optarg[0] != '0'){
                    fprintf(stderr, "Error: -n requires a valid integer argument.\n");
                    errorIncorrectUsage(EINVAL);
                }

                n = temp;

                break;

            case '?':
                errorIncorrectUsage(EINVAL);
            }
    }

    if (optind < argc){      
        printf("More arguments passed than expected\n");
        errorIncorrectUsage(E2BIG);
    }

    if (n <= 0){
        printf("n out of range, must be n>0\n");
        errorIncorrectUsage(EPERM);
    }
    
    children = (pid_t*)malloc(n * sizeof(int));

    if (children == NULL){
        printf("Malloc Failed\n");
        exit(ENOMEM);
    }
    
    for (i = 0; i < n; i++){
        // https://search.brave.com/search?q=handling+multiple+children+c&source=desktop
        children[i] = fork();
        if (children[i] < 0){
            exit(ECHILD);
        }
        else if (children[i] == 0){
            // trap children in child function, which they must exit() from
            childFunction();
        }
        // else{
        //     printf("%d\n", children[i]);
        // }
    }

    printf("Parent pid %d\n", getpid());

    // parent process
    signal(SIGCONT, parent_signal_handler);
    signal(SIGINT, parent_signal_handler);

    pause();

    printf("Parent process continuing, cleaning up\n");

    for (i = 0; i < n; i ++){
        kill(children[i], SIGCONT);
    }

    free(children);

    exit(EXIT_SUCCESS);
}