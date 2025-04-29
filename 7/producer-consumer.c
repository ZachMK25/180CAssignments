#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>  
#include <errno.h>
// #include <signal.h>

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/producer_consumer_socket"

int errorIncorrectUsage(int err, char** argv){
    printf("\nUsage: ./%s (-p | -c) (-u | -s) [-m 'message'] [-q queue_depth] \n", argv[0]);
    return err;
}

int main(int argc, char** argv){
        int portno = 51717;
        // input validation
        int opt, temp;
        extern int optind;
        extern char* optarg;
        
        int option_producer = 0;
        int option_consumer = 0;
        char* message = NULL;
        int option_socket = 0;
        int option_sharedmem = 0;
        int q = 1;
        
        while((opt = getopt(argc, argv, "pcm:q:us")) != -1){
    
            switch(opt){
                case 'p':
                    option_producer = 1;
    
                    break;
                case 'c':
                    option_consumer = 1;
    
                    break;
                case 'u':
                    option_socket = 1;
    
                    break;
                case 's':
                    option_sharedmem = 1;
    
                    break;
                case 'm':
                    message = optarg;
        
                    break;

                case 'q':
                    temp = atoi(optarg);
                    // enforce integer
                    if (temp == 0 && optarg[0] != '0'){
                        fprintf(stderr, "Error: -q requires a valid integer argument. Got %d\n", temp);
                        return errorIncorrectUsage(EINVAL, argv);
                    }
                    q = temp;

                    if (q <= 0) {
                        fprintf(stderr, "Error: -q requires a positive integer argument. Got %d\n", q);
                        return errorIncorrectUsage(EINVAL, argv);
                    }

                    break;
    
                case '?':
                    errorIncorrectUsage(EINVAL, argv);
                }
        }

            // optind is for the extra arguments 
    // which are not parsed 
    if (optind < argc){      
        printf("More arguments passed than expected\n");
        return errorIncorrectUsage(E2BIG, argv);
    }

    if (option_socket == option_sharedmem){
        fprintf(stderr, "Error: Unix Socket or Shared Memory status (-u | -s) must be set as an argument");
        return errorIncorrectUsage(EINVAL, argv);
    }
    if (option_consumer == option_producer){
        fprintf(stderr, "Error: Producer or Consumer status (-p | -c) must be set as an argument");
        return errorIncorrectUsage(EINVAL, argv);
    }

    if (message == NULL && option_producer){
        fprintf(stderr, "Error: Producer must be provided a message (-m [your-message-here])");
        return errorIncorrectUsage(EINVAL, argv);
    }
    // TODO: remove when finished
    printf("Params: isProducer: %d, isSocket: %d, q: %d, message: %s\n", option_producer, option_socket, q, message);

    
    // ======================Communication section======================


    if (option_producer){
        
        if (option_socket){
            int sock_fd;
            struct sockaddr_un address;

            if ((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
                perror("socket error");
                exit(EXIT_FAILURE);
            }

            memset(&address, 0, sizeof(address));
            address.sun_family = AF_UNIX;
            strncpy(address.sun_path, SOCKET_PATH, sizeof(address.sun_path) - 1);

            if (connect(sock_fd, (struct sockaddr*)&address, sizeof(address)) == -1) {
                perror("connect error");
                exit(EXIT_FAILURE);
            }
        
            if (write(sock_fd, message, strlen(message)) == -1) {
                perror("write error");
                exit(EXIT_FAILURE);
            }
        
            printf("Producer sent message: %s\n", message);
        
            close(sock_fd);
            return 0;
        }
        else{

        }
    }
    else {
        // consumer
        if (option_socket){
            int server_fd, client_fd;
            struct sockaddr_un address;
            char buffer[1024];
            int bytes_read;

            if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
                perror("socket error");
                exit(EXIT_FAILURE);
            }
        
            unlink(SOCKET_PATH);

            memset(&address, 0, sizeof(address));
            address.sun_family = AF_UNIX;
            strncpy(address.sun_path, SOCKET_PATH, sizeof(address.sun_path)-1);

            if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == -1) {
                perror("bind error");
                exit(EXIT_FAILURE);
            }

            if (listen(server_fd, 5) == -1) {
                perror("listen error");
                exit(EXIT_FAILURE);
            }

            printf("Consumer waiting for connection...\n");

            if ((client_fd = accept(server_fd, NULL, NULL)) == -1) {
                perror("accept error");
                exit(EXIT_FAILURE);
            }

            printf("Consumer connected. Waiting for message...\n");

            while ((bytes_read = read(client_fd, buffer, sizeof(buffer) - 1)) > 0) {
                buffer[bytes_read] = '\0';
                printf("Received message: %s\n", buffer);
            }

            // Cleanup
            close(client_fd);
            close(server_fd);
            unlink(SOCKET_PATH);

            return 0;

        }
        else{
            // shared memory

        }

    }
    return 0;
}