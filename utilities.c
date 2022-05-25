#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "headers.h"


void exit_with_error(const char *message, const int exit_code) {
    fprintf(stderr, "%s\n", message);
    MPI_Finalize();
    exit(exit_code);
}

double *divide_matrix_into_submatrixes(double *matrix, int matrix_size, int *counts, int *displacements, int rank, int comm_size) {
    int i;

    if(rank == 0) {
        int part_length = (int) (matrix_size / comm_size);

        for(i = 0; i < comm_size; i++)
            counts[i] = part_length * matrix_size;

        for(i = 0; matrix_size % (part_length * comm_size + i) != 0; i++)
            counts[i % comm_size] += matrix_size;

        int amount = 0;
        displacements[0] = 0;

        for(i = 1; i < comm_size; i++) {
            amount += counts[i - 1];
            displacements[i] = amount;
        }
    }

    MPI_Bcast(counts, comm_size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(displacements, comm_size, MPI_INT, 0, MPI_COMM_WORLD);

    int total_size = rank == comm_size - 1 ? matrix_size * matrix_size - displacements[rank] : displacements[rank + 1] - displacements[rank];
    double *A_fragment = (double*) malloc(total_size * sizeof(double));

    MPI_Scatterv(matrix, counts, displacements, MPI_DOUBLE, A_fragment, total_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    return A_fragment;
}

double *multiply_submatrix_and_vector(double *submatrix, int matrix_size, double *vector, int *counts, int *displacements, int rank, int comm_size) {
    double *result_vector = (double*) malloc(matrix_size * sizeof(double));
    int i, j;

    MPI_Bcast(vector, matrix_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    for(i = 0; i < counts[rank] / matrix_size; i++) {
        double sum = 0.0;
        
        for(j = 0; j < matrix_size; j++)
            sum += submatrix[i * matrix_size + j] * vector[j];

        result_vector[i] = sum;
    }

    return result_vector;
}

double *gather_subvectors(double *result_vector, int matrix_size, int *counts, int *displacements, int rank, int comm_size) {
    double *final_result = rank == 0 ? (double*) malloc(matrix_size * sizeof(double)) : NULL;
    int i;

    for(i = 0; i < comm_size; i++) {
        counts[i] /= matrix_size;
        displacements[i] /= matrix_size;
    }

    MPI_Gatherv(result_vector, counts[rank], MPI_DOUBLE, final_result, counts, displacements, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    return final_result;
}
