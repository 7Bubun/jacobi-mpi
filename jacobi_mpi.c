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

    if(rank != 0)
        b = (double*) malloc(matrix_size * sizeof(double));

    int *displacements = (int*) malloc(comm_size * sizeof(int));
    int *counts = (int*) malloc(comm_size * sizeof(int));

    //test start
    double *submatrix = divide_matrix_into_submatrixes(A, matrix_size, counts, displacements, rank, comm_size);
    double *part_of_result = multiply_submatrix_and_vector(submatrix, matrix_size, b, counts, displacements, rank, comm_size);
    gather_subvectors(part_of_result, b, matrix_size, counts, displacements, rank, comm_size);

    if(rank == 0) {
        int i;

        printf("Wynik:\n");

        for(i = 0; i < matrix_size; i++)
            printf("%lf\n", b[i]);
    }

    free(part_of_result);
    free(submatrix);
    //test end

    if(rank == 0)
       free(A);

    free(b);
    free(displacements);
    free(counts);
    MPI_Finalize();
    return 0;
}
