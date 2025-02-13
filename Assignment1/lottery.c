#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <unistd.h>  


// https://stackoverflow.com/questions/3219393/stdlib-and-colored-output-in-c
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"


// https://www.cs.yale.edu/homes/aspnes/pinewiki/C(2f)Randomization.html
// how to generate random numbers

int errorIncorrectUsage(){
    printf("\nUsage: ./lottery -n [Numbers to Generate, int >= 1] -r [Max Number, int >= 1] (optional: -p [Max Powerball Number, int >= 0]) -N [Numbers of Sets to Generate, int >= 1]\n");
    return 1;
}

int validateOptions(int argc, char** argv, int* n, int* r, int* p, int* N){

    // input validation
    

    int opt;
    int passed;
    
    while((opt = getopt(argc, argv, "n:N:r:p:")) != -1){

        // USE GETOPTIONS INSTEAD

        switch(opt){
            case 'n':
                passed = atoi(optarg);
                // enforce integer
                if (passed == 0 && optarg[0] != '0'){
                    fprintf(stderr, "Error: -n requires a valid integer argument.\n");
                }

                *n = passed;
                // printf("%c = %d\n", opt, *n);

                break;

            case 'r':
                passed = atoi(optarg);
                // enforce integer
                if (passed == 0 && optarg[0] != '0'){
                    fprintf(stderr, "Error: -r requires a valid integer argument.\n");
                }
                *r = passed;
                // printf("%c = %d\n", opt, *r);

                break;

            case 'p':
                passed = atoi(optarg);
                // enforce integer
                if (passed == 0 && optarg[0] != '0'){
                    fprintf(stderr, "Error: -p requires a valid integer argument.\n");
                }
                *p = passed;
                // printf("%c = %d\n", opt, *p);
                break;

            case 'N':
                passed = atoi(optarg);
                // enforce integer
                if (passed == 0 && optarg[0] != '0'){
                    fprintf(stderr, "Error: -N requires a valid integer argument.\n");
                }
                *N = passed;
                // printf("%c = %d\n", opt, *N);
                break;

            case '?':
                return errorIncorrectUsage();
        }
    }

    // optind is for the extra arguments 
    // which are not parsed 
    if (optind > argc){      
        printf("more arguments passed than expected\n");
        errorIncorrectUsage();
    } 
    

    if (*n <= 0 || *r <= 0 || *N <= 0 || *p<0){
        return errorIncorrectUsage();
    }

    printf("\n");

    return 0;
}

int main(int argc, char** argv){

    int n = 0;
    int r = 0;
    int p = 0;
    int N = 0;

    int output = validateOptions(argc, argv, &n, &r, &p, &N);
    if (output != 0){
        return output;
    }

    srand(time(0));

    int sets [N][n];
    int i;

    if (p){
        printf(ANSI_COLOR_MAGENTA "Powerball numbers are labeled in this color\n\n" ANSI_COLOR_RESET);
    }


    for (i = 0; i < N; i++){

        int j;
        for (j = 0; j < n-1; j++){
            sets[i][j] = (rand() % r) + 1;
        }

        // special powerball case at the end
        if (p != 0){
            sets[i][j] = (rand() % p) + 1;
        }
        // otherwise treat it like a normal number and use the other range
        else{
            sets[i][j] = (rand() % r) + 1;
        }
    }

    for (i = 0; i < N; i++){
        int j;
        for (j = 0; j < n-1; j++){
            printf("%d, ", sets[i][j]);
        }

        // label powerball column
        if (p != 0){
            printf(ANSI_COLOR_MAGENTA "%d" ANSI_COLOR_RESET, sets[i][j]);
        }
        else{
            printf("%d", sets[i][j]);
        }
        printf("\n\n");   
    }

    return 0;
}
