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


pid_t pid = -1;
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
    else if (sig == SIGINT) {
        printf("And I slaughtered them like animals\n");
        exit(EIO);
    }
}

void parent_signal_handler(int sig){
    
    // if (sig == SIGCONT){
        
    //     for (i = 0; i < n; i ++){
    //         kill(children[i], SIGCONT);
    //     }

    //     free(children);

    //     exit(0);
    // }
    if (sig == SIGINT){
        
        for (i = 0; i < n; i ++){
            kill(children[i], SIGINT);
        }

        free(children);

        exit(EOWNERDEAD);
    }
}

int childFunction(){
    // I am the child process
    printf("%d\n",pid);

    signal(SIGCONT, child_signal_handler);
    signal(SIGINT, child_signal_handler);

    pause();

    printf("I'm dying...\n");
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
        exit(1);
    }
    
    for (i = 0; i < n; i++){
        // https://search.brave.com/search?q=handling+multiple+children+c&source=desktop
        if ((children[i] = fork()) < 0){
            exit(ECHILD);
        }
        else if (children[i] == 0){
            childFunction();
        }
        else{
            printf("%d\n", children[i]);
        }
    }


    // parent process
    signal(SIGCONT, parent_signal_handler);
    signal(SIGINT, parent_signal_handler);

    pause();


    // https://search.brave.com/search?q=handling+multiple+children+c&source=desktop
    int status;
    pid_t pid;
    while (n > 0) {
        pid = wait(&status);
        printf("Child with PID %ld exited with status 0x%x.\n", (long)pid, status);
        --n;  // TODO(pts): Remove pid from the pids array.
    }

    for (i = 0; i < n; i ++){
        kill(children[i], SIGCONT);
    }

    free(children);

    exit(0);
}