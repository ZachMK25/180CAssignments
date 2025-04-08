#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>

int numSIGINTs = 0;
int iterations = 0;
int debug = 0;

void signal_handler(int sig){
    if (sig == SIGINT){
        // toggle debug
        debug = !debug;
    }
    else if (sig == SIGUSR1){
        printf("terminating loop gracefully");
        exit(0);
    }

}

int main(int argc, char** argv){
    signal(SIGINT, signal_handler);
    signal(SIGUSR1, signal_handler);

    while(1){
        sleep(2);
        if (debug){
            printf("completed %d iterations\n", iterations);
        }
        iterations++;
    }
    
}