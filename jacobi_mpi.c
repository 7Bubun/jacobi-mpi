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
    int *A_counts, *A_displacements, *b_counts, *b_displacements;
    int matrix_size, rows_count;

    if(rank == 0) {
        int i;
        
        if(argc != 4)
            exit_with_error("Niepoprawna liczba argumentow\n", ARG_NUM_ERROR_CODE);
        
        A = read_matrix(argv[1], &matrix_size);
        b = read_vector(argv[2], matrix_size, A);

        A_counts = (int*) malloc(comm_size * sizeof(int));
        A_displacements = (int*) malloc(comm_size * sizeof(int));

        b_counts = (int*) malloc(comm_size * sizeof(int));
        b_displacements = (int*) malloc(comm_size * sizeof(int));

        rows_count = (int) (matrix_size / comm_size);
        int r = matrix_size % (rows_count * comm_size);

        A_counts[0] = (rows_count + r) * matrix_size;
        b_counts[0] = rows_count + r;

        A_displacements[0] = 0;
        b_displacements[0] = 0;

        for(i = 1; i < comm_size; i++) {
            A_counts[i] = rows_count * matrix_size;
            b_counts[i] = rows_count;

            A_displacements[i] = (i * rows_count + r) * matrix_size;
            b_displacements[i] = i * rows_count + r;
        }
    }

    MPI_Bcast(&rows_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&matrix_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    double *A_fragment, *b_fragment;
    
    if(rank == 0)
        rows_count = b_counts[0];

    A_fragment = (double*) malloc(rows_count * matrix_size * sizeof(double));
    b_fragment = (double*) malloc(rows_count * sizeof(double));

    MPI_Scatterv(A, A_counts, A_displacements, MPI_DOUBLE, A_fragment, rows_count * matrix_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatterv(b, b_counts, b_displacements, MPI_DOUBLE, b_fragment, rows_count, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    //test start
    printf("proces %d, liczba elementów A: %d, pierwszy element A: %lf\n", rank, rows_count * matrix_size, A_fragment[0]);
    
    MPI_Barrier(MPI_COMM_WORLD);

    printf("proces %d, liczba elementów b: %d, pierwszy element b: %lf\n", rank, rows_count, b_fragment[0]);
    //test end

    if(rank == 0) {
       free(A);
       free(b);
       free(A_counts);
       free(A_displacements);
       free(b_counts);
       free(b_displacements);
    }

    free(A_fragment);
    free(b_fragment);
    MPI_Finalize();
    return 0;
}
