#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <string.h>

// https://stackoverflow.com/questions/3219393/stdlib-and-colored-output-in-c
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_RESET   "\x1b[0m"


// https://www.cs.yale.edu/homes/aspnes/pinewiki/C(2f)Randomization.html
// how to generate random numbers

int errorIncorrectUsage(){
    printf("Usage: ./lottery -n [Numbers to Generate] -r [Max Number] (optional: -p [Max Powerball Number]) -N [Numbers of Sets to Generate]\n");
    return 1;
}

int validateOptions(int argc, char** argv, int* n, int* r, int* p, int* N){

    // input validation
    
    // must have either 7 or 9 arguments, depending on if p is included
    if (!(argc == 7 || argc == 9)){
        return errorIncorrectUsage();
    }

    int i = 0;
    for (i = 1; i < argc - 1; i+=2) {

        // stop the program before reading an index that wouldn't be defined
        if (((int)strlen(argv[i]) != 2) || !isalpha(argv[i][1])){
            return errorIncorrectUsage();
        }

        char option = argv[i][1];
        long int passed=0;
        char * ptrend;
        long int test;

        switch(option){
            case 'n':
                passed = strtol(argv[i+1], &ptrend, 10);
                *n = passed;
                break;

            case 'r':
                passed = strtol(argv[i+1], &ptrend, 10);
                *r = passed;
                break;

            case 'p':
                passed = strtol(argv[i+1], &ptrend, 10);
                *p = passed;
                break;

            case 'N':
                passed = strtol(argv[i+1], &ptrend, 10);
                *N = passed;
                break;
            default:
                printf("invalid option passed: -%c\n", option);
                return errorIncorrectUsage();
        }
    }

    // printf("\noptions: \nn: %d\nr: %d \np: %d \nN: %d \n", *n,*r,*p,*N);

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
            printf(ANSI_COLOR_RED "%d" ANSI_COLOR_RESET, sets[i][j]);
        }
        else{
            printf("%d", sets[i][j]);
        }

        printf("\n\n");   
    }

    return 0;
}
