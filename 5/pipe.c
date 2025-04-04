#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

#define READ 0
#define WRITE 1

int main(){
    // I am your daddy! and my name is <PID>
    int MSGSIZE1 = 33 + 10;
    int MSGSIZE2 = 19 + 10;

    char inbuf1[MSGSIZE1];
    char inbuf2[MSGSIZE2];
    int nbytes;
    int to_child[2];
    int to_parent[2];

    if (pipe(to_child) < 0)
        exit(1);
    if (pipe(to_parent) < 0)
        exit(1);

    int pid = fork();

    // https://www.geeksforgeeks.org/pipe-system-call/

    if (pid > 0){
        // parent

        char* msg1 = (char*)malloc(MSGSIZE1);
        if (msg1 == NULL){
            perror("malloc");
            exit(ECHILD);
        }

        snprintf(msg1, MSGSIZE1, "I am your daddy! and my name is %d\n", getpid());

        write(to_child[WRITE], msg1, MSGSIZE1);
        close(to_child[WRITE]);

        // reading the response from the child
        close(to_parent[WRITE]);

        while ((nbytes = read(to_parent[READ], inbuf2, MSGSIZE2)) > 0){
            printf("%s", inbuf2);
        }

        close(to_parent[READ]);

        if (nbytes != 0){
            printf("Error: %s\n", strerror(EPIPE));
            exit(EPIPE);
        }

        int status;
        wait(&status);
        // printf("status: %d\n", status);

        free(msg1);

        return 0;
    }
    else {
        close(to_child[WRITE]);
        // child

        while ((nbytes = read(to_child[READ], inbuf1, MSGSIZE1)) > 0){
            printf("%s", inbuf1);
        }

        close(to_child[READ]);

        if (nbytes != 0){
            printf("Error: %s\n", strerror(EPIPE));
            exit(EPIPE);
        }

        // write response

        char* msg2 = (char*)malloc(MSGSIZE2);
        if (msg2 == NULL){
            perror("malloc");
            exit(ECHILD);
        }

        snprintf(msg2, MSGSIZE1, "Daddy, my name is %d\n", getpid());

        write(to_parent[WRITE], msg2, MSGSIZE2);
        close(to_parent[WRITE]);

        return 0;
    }

}