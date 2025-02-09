#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

// https://www.cs.yale.edu/homes/aspnes/pinewiki/C(2f)Randomization.html
// how to generate random numbers

int errorIncorrectUsage(){
    printf("Usage: ./lottery -n [Numbers to Generate] -r [Max Number] (optional: -p [Max Powerball Number]) -N [Numbers of Sets to Generate]\n");
    return 1;
}

int validateOptions(int argc, char** argv, int* n, int* r, int* p, int* N){

    // input validation
    
    // must have an odd number of arguments
    // program name + some number of pairs of options and values
    if ((argc & 1) != 1){
        return errorIncorrectUsage();
    }

    int i = 0;
    for (i = 1; i < argc - 1; i+=2) {

        // this does not feel safe. need to figure out how to check for string of length 2
        if (!isalpha(argv[i][1])){
            printf("error");
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
        }
    }

    printf("\noptions: \nn: %d\nr: %d \np: %d \nN: %d \n", *n,*r,*p,*N);

    if (*n <= 0 || *r <= 0 || *N <= 0){
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
            sets[i][j] = rand() % r;
        }

        // special powerball case at the end
        if (p != 0){
            sets[i][j] = rand() % p;
        }
        // otherwise treat it like a normal number and use the other range
        else{
            sets[i][j] = rand() % r;
        }

    }

    // label powerball column
    if (p != 0){
        // trying to place the powerball label in the right spot
        for (i = 0; i < n-1; i++){
            printf("  ");
        }
        printf("pb!\n");
    }


    for (i = 0; i < N; i++){
        int j;
        for (j = 0; j < n; j++){
            printf("%d ", sets[i][j]);
        }
        printf("\n");
        for (j = 0; j < n; j++){
            printf("--");
        }
        printf("\n");
    }


    return 0;
}
