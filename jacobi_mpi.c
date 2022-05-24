#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "headers.h"

#define ARG_NUM_ERROR_CODE 1


//args: A filename, b filename, number of iterations
int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    
    int comm_size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    double *A, *b;
    int matrix_size;

    if(rank == 0) {
        if(argc != 4)
            exit_with_error("Niepoprawna liczba argumentow\n", ARG_NUM_ERROR_CODE);
        
        A = read_matrix(argv[1], &matrix_size);
        b = read_vector(argv[2], matrix_size, A);
    }

    MPI_Bcast(&matrix_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    //test start
    double *test = multiply_matrix_and_vector_paralell(A, matrix_size, b, rank, comm_size);

    if(rank == 0){
        printf("Wynik mnozenia:\n");

        int i;
        for(i = 0; i < matrix_size; i++)
            printf("%lf\n", test[i]);
    }
    
    //test end

    if(rank == 0) {
       free(A);
       free(b);
       free(test);
    }

    MPI_Finalize();
    return 0;
}
