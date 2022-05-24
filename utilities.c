#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "headers.h"


void exit_with_error(const char *message, const int exit_code) {
    fprintf(stderr, "%s\n", message);
    MPI_Finalize();
    exit(exit_code);
}

double *multiply_matrix_and_vector_paralell(double *A, int A_size, double *B, int rank, int comm_size) {
    int *counts, *displacements;
    double *final_result;
    int i, j, k;

    displacements = (int*) malloc(comm_size * sizeof(int));

    if(rank == 0) {
        int part_length = (int) (A_size / comm_size);
        counts = (int*) malloc(comm_size * sizeof(int));

        for(i = 0; i < comm_size; i++) {
            counts[i] = part_length * A_size;
        }

        i = 0;

        while(A_size % (part_length * comm_size + i) != 0) {
            counts[i % comm_size] += A_size;
            i++;
        }

        int amount = 0;
        displacements[0] = 0;

        for(i = 1; i < comm_size; i++) {
            amount += counts[i - 1];
            displacements[i] = amount;
        }

    } else {
        B = (double*) malloc(A_size * sizeof(double));
    }

    int *ones = (int*) malloc(comm_size * sizeof(int));
    int *disps = (int*) malloc(comm_size * sizeof(int));

    for(i = 0; i < comm_size; i++) {
        ones[i] = 1;
        disps[i] = i;
    }

    MPI_Bcast(displacements, comm_size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(B, A_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    int total_size = rank == comm_size - 1 ? A_size * A_size - displacements[rank] : displacements[rank + 1] - displacements[rank];
    double *A_fragment = (double*) malloc(total_size * sizeof(double));

    MPI_Scatterv(A, counts, displacements, MPI_DOUBLE, A_fragment, total_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    
    double *result_vector = (double*) malloc(A_size * sizeof(double));
    int end = rank == comm_size - 1 ? A_size : displacements[rank + 1] / A_size;

    for(i = displacements[rank] / A_size; i < end; i++) {
        double sum = 0.0;
        k = i - displacements[rank] / A_size;
        
        for(j = 0; j < A_size; j++)
            sum += A_fragment[k * A_size + j] * B[j];

        result_vector[k] = sum;
    }

    if(rank == 0) {
        final_result = (double*) malloc(A_size * sizeof(double));

        for(i = 0; i < comm_size; i++) {
            counts[i] /= A_size;
            displacements[i] /= A_size;
        }
    }

    MPI_Gatherv(result_vector, k + 1, MPI_DOUBLE, final_result, counts, displacements, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if(rank == 0)
        free(counts);

    free(displacements);
    free(ones);
    free(disps);
    free(A_fragment);
    free(result_vector);

    return final_result;
}
