#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>  
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>

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
        char* message;
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


    printf("Params: isProducer: %d, isSocket: %d, q: %d, message: %s\n", option_producer, option_socket, q, message);

    return 0;
}