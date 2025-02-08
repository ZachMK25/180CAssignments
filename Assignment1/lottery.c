#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

// https://www.cs.yale.edu/homes/aspnes/pinewiki/C(2f)Randomization.html
// how to generate random numbers

int errorIncorrectUsage(){
    printf("Usage: ./lottery -n [Numbers to Generate] -r [Max Number] (optional: -p [Max Powerball Number]) -N [Numbers of Sets to Generate]\n");
    return 1;
}

int main(int argc, char** argv){

    printf("cmdline args count=%d", argc);

    // must have an odd number of arguments
    // program name + some number of pairs of options and values
    if (argc & 1 != 1){
        return errorIncorrectUsage();
    }

    int n = 0;
    int r = 0;
    int p = 0;
    int N = 0;

    int i = 0;
    for (i = 1; i < argc - 1; i+=2) {
        // printf("\narg%d=%s\n", i, argv[i]);

        // this does not feel safe. need to figure out how to check for string of length 2
        if (!isalpha(argv[i][1])){
            printf("error");
            return errorIncorrectUsage();
        }

        char option = argv[i][1];
        long int passed=0;
        // long int passed_long = 0;
        char * ptrend;
        long int test;

        switch(option){
            case 'n':
                // printf("switch n\n");
            // https://stackoverflow.com/questions/29248585/c-checking-command-line-argument-is-integer-or-not
                
                passed = strtol(argv[i+1], &ptrend, 10);

                n = passed;

                break;

            case 'r':
                passed = strtol(argv[i+1], &ptrend, 10);
                
                r = passed;

                break;

            case 'p':
                passed = strtol(argv[i+1], &ptrend, 10);

                p = passed;

                break;

            case 'N':
                passed = strtol(argv[i+1], &ptrend, 10);
                
                N = passed;

                break;
        }
    }

    printf("\noptions: \nn: %d\nr: %d \np: %d \nN: %d \n", n,r,p,N);

    if (n == 0 || r == 0 || N == 0){
        return errorIncorrectUsage();
    }

    printf("\n");

    return 0;
}
