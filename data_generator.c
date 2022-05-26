#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Bad args.\n");
        return 1;
    }

    MPI_Init(&argc, &argv);

    FILE *f;
    int rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0)
    {
        f = fopen("generatedA.txt", "w");
        int n = atoi(argv[1]);
        int i, j;

        fprintf(f, "%d\n\n", n);

        srand(time(NULL));

        for (i = 0; i < n; i++)
        {
            for (j = 0; j < n; j++)
            {
                double number = i == j ? 1000000.0 * ((double)rand() / RAND_MAX) + 500000 : 10 * ((double)rand() / RAND_MAX) - 5;
                fprintf(f, "%lf\n", number);
            }
        }

        fclose(f);
    }
    else if (rank == 1)
    {
        int n = atoi(argv[1]);
        int i;

        f = fopen("generatedB.txt", "w");

        for (i = 0; i < n; i++)
            fprintf(f, "%lf\n", 1000000.0 * ((double)rand() / RAND_MAX));

        fclose(f);
    }

    MPI_Finalize();
    return 0;
}
