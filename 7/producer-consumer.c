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
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>

#define SHM_NAME "/my_shared_mem"
#define SEM_EMPTY_NAME "/sem_empty"
#define SEM_FULL_NAME "/sem_full"
#define SEM_MUTEX_NAME "/sem_mutex"

#define MAX_MSG_LEN 256
#define MAX_QUEUE_SIZE 64

typedef struct {
    char messages[MAX_QUEUE_SIZE][MAX_MSG_LEN];
    int in;
    int out;
    int size;
} SharedQueue;

#define SOCKET_PATH "/tmp/producer_consumer_socket"

int producer_socket(char* message, int q, int echo){
    int i;
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

    for (i=0; i < q; i++) {
        
    char fixed_msg[256] = {0};
    strncpy(fixed_msg, message, sizeof(fixed_msg)-1);

        dprintf(sock_fd, "%s\n", message);
        
        if (echo){
            printf("Producer sent message: %s\n", message);
        }
    }

    close(sock_fd);
    return 0;
}

int producer_shared_mem(char* message, int q, int echo){
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    ftruncate(shm_fd, sizeof(SharedQueue));

    SharedQueue *queue = (SharedQueue*) mmap(NULL, sizeof(SharedQueue), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if (queue == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    if (queue->size != q) {
        queue->in = 0;
        queue->out = 0;
        queue->size = q;
    }

    sem_t *sem_empty = sem_open(SEM_EMPTY_NAME, O_CREAT, 0666, q);
    sem_t *sem_full = sem_open(SEM_FULL_NAME, O_CREAT, 0666, 0);
    sem_t *sem_mutex = sem_open(SEM_MUTEX_NAME, O_CREAT, 0666, 1);

    // delay to allow consumer to start but after semaphores are created
    sleep(15);

    for (int i = 0; i < q; i++) {
        sleep(1);

        sem_wait(sem_empty);
        sem_wait(sem_mutex);

        strncpy(queue->messages[queue->in], message, MAX_MSG_LEN - 1);
        queue->messages[queue->in][MAX_MSG_LEN - 1] = '\0';
        queue->in = (queue->in + 1) % queue->size;

        sem_post(sem_mutex);
        sem_post(sem_full);

        if (echo) {
            printf("Produced: %s\n", message);
        }
    }

    munmap(queue, sizeof(SharedQueue));
    close(shm_fd);

    return 0;
}


int consumer_socket(int q, int echo){
    int i = 0;

    int server_fd, client_fd;
    struct sockaddr_un address;
    char buffer[256];
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

    printf("Consumer waiting for message...\n");

    FILE *stream = fdopen(client_fd, "r");
    char line[1024];

    for (i=0; i < q; q++){

        if (fgets(line, sizeof(line), stream) != NULL) {
            line[strcspn(line, "\n")] = '\0';  // remove newline
            if (echo) {
                printf("Received message: %s\n", line);
            }
        } else {
            break;
        }
        
    }

    close(client_fd);
    close(server_fd);
    unlink(SOCKET_PATH);

    return 0;
}

int consumer_shared_mem(int q, int echo){
    // consumer-shared memory
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    SharedQueue *queue = (SharedQueue*) mmap(NULL, sizeof(SharedQueue), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (queue == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    sem_t *sem_empty = sem_open(SEM_EMPTY_NAME, 0);
    sem_t *sem_full = sem_open(SEM_FULL_NAME, 0);
    sem_t *sem_mutex = sem_open(SEM_MUTEX_NAME, 0);

    for (int i = 0; i < q; i++) {
        sem_wait(sem_full);
        sem_wait(sem_mutex);

        char msg[MAX_MSG_LEN];
        strncpy(msg, queue->messages[queue->out], MAX_MSG_LEN);
        msg[MAX_MSG_LEN - 1] = '\0';
        queue->out = (queue->out + 1) % queue->size;

        sem_post(sem_mutex);
        sem_post(sem_empty);

        if (echo) {
            printf("Received message: %s\n", msg);
        }
    }
    munmap(queue, sizeof(SharedQueue));
    close(shm_fd);

    return 0;
}


int errorIncorrectUsage(int err, char** argv){
    printf("\nUsage: ./%s (-p | -c) (-u | -s) [-m 'message'] [-q queue_depth] \n", argv[0]);
    return err;
}

int main(int argc, char** argv){
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
        int echo = 0;
        
        while((opt = getopt(argc, argv, "pcm:q:use")) != -1){
    
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
                case 'e':
                    echo = 1;
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
    
    // ======================Communication section======================

    if (option_producer && option_socket){
        int ret = producer_socket(message, q, echo);
        if (ret != 0){
            return ret;
        }
    }

    if (option_producer && option_sharedmem){
        int ret = producer_shared_mem(message, q, echo);
        if (ret != 0){
            return ret;
        }
    }

    if (option_consumer && option_socket){
        int ret = consumer_socket(q, echo);
        if (ret != 0){
            return ret;
        }
    }

    if (option_consumer && option_sharedmem){
        int ret = consumer_shared_mem(q, echo);
        if (ret != 0){
            return ret;
        }
    }


    if (option_sharedmem){
        shm_unlink(SHM_NAME);
        sem_unlink(SEM_EMPTY_NAME);
        sem_unlink(SEM_FULL_NAME);
        sem_unlink(SEM_MUTEX_NAME);
    }

    return 0;
}