#include <pthread.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;

int turn = 1;
int interrupted = 0;

void handle_sigint(int sig){
    if (sig == SIGINT){
        interrupted = 1;
    }
}

void* thread1Function(void* arg){

    while(!interrupted){

        // lock to prevent t2 from running

        pthread_mutex_lock(&lock);

        printf("thread 1: ping thread 2\n");

        turn = 2;

        pthread_cond_signal(&cond1);

        pthread_mutex_unlock(&lock);
        

        // use variable to signal here

        pthread_mutex_lock(&lock);

        while (turn != 1 && !interrupted){
            pthread_cond_wait(&cond2, &lock);
        }

        //wait for response

        printf("thread 1: pong! thread 2 ping received\n");

        pthread_mutex_unlock(&lock);

        usleep(500000); //sleep to make prints more readable
    }

    return 0;
}

void* thread2Function(void* arg){

    while(!interrupted){
        // acquire lock

        pthread_mutex_lock(&lock);

        // wait for thread1 ping
        while (turn != 2 && !interrupted) {
            pthread_cond_wait(&cond1, &lock);
        }

        printf("thread 2: pong! thread 1 ping received\n");

        printf("thread 2: ping thread 1\n");

        // use variable to signal here
        turn = 1;

        pthread_cond_signal(&cond2);

        pthread_mutex_unlock(&lock);

        usleep(500000); //sleep to make prints more readable
    }

    return 0;
}


int main() {
    signal(SIGINT, handle_sigint);

    pthread_t thread1;
    pthread_t thread2;
    
    pthread_create(&thread1, NULL, thread1Function, NULL);
    pthread_create(&thread2, NULL, thread2Function, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond1);
    pthread_cond_destroy(&cond2);

    return 0;
}
