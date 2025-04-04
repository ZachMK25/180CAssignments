#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define MSGSIZE 40
#define READ 0
#define WRITE 1

int main(){
    char inbuf[MSGSIZE];
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
        const char* msg1 = "I am your daddy! and my name is parent\n";

        write(to_child[WRITE], msg1, MSGSIZE);

        int status;
        wait(&status);
        printf("status: %d", status);
        return 0;
    }
    else {
        // child

        do {
            nbytes = read(to_child[READ], inbuf, MSGSIZE);
            printf("%s", inbuf);
        } while (nbytes >= 0);

        // while ((nbytes = read(to_child[READ], inbuf, MSGSIZE)) > 0)
        printf("nbytes: %d", nbytes);
        if (nbytes != 0)
            exit(2);
        printf("Finished reading\n");
        return 0;
    }

}