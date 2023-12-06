// matgen.cxx
// Generate a m*n double precision matrix file

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

char filename_A[100];
char filename_B[100];
char filename_C[100];
char cmd[100];

int main(int argc, char **argv)
{
    int n1, n2, n3;

    // get the loops that this procedure will do.
    if (argc < 2)
    {
        printf("wrong arg\n");
        return 1;
    }
    int testtimes = atoi(argv[1]);
    int i;

    srand48(time(NULL)); // Use time as a seed

    // clean dir
    system("rm -r ../test");
    system("mkdir ../test");

    for (i = 0; i < testtimes; i++)
    {
        n1 = rand() % 100 + 1;
        n2 = rand() % 100 + 1;
        n3 = rand() % 100 + 1;

        
        // makedir
        sprintf(cmd, "mkdir ../test/testcase_%d", i);
        system(cmd);
        sprintf(cmd, "mkdir ../test/testcase_%d/input", i);
        system(cmd);
        
        sprintf(cmd, "mkdir ../test/testcase_%d/output", i);
        system(cmd);
        
        sprintf(filename_A, "../test/testcase_%d/input/A", i);
        sprintf(filename_B, "../test/testcase_%d/input/B", i);
        sprintf(filename_C, "../test/testcase_%d/input/C", i);

        
        printf("dir gen\n");
        // gen matrix A
        sprintf(cmd, "./matgen %d %d %s", n1, n2, filename_A);
        system(cmd);
        // gen matrix B
        sprintf(cmd, "./matgen %d %d %s", n2, n3, filename_B);
        system(cmd);
        // cal matrix C
        sprintf(cmd, "./serial %s %s %s", filename_A, filename_B, filename_C);
        system(cmd);
    }

    printf("%d matrix have generated in input\n", testtimes);

    return 0;
}
