#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include "headers.h"

void exit_with_error(const char *message, const int exit_code)
{
    fprintf(stderr, "%s\n", message);
    MPI_Abort(MPI_COMM_WORLD, exit_code);
}

double *divide_matrix_into_submatrixes(double *matrix, int matrix_size, int *counts, int *displacements, int rank, int comm_size)
{
    int i;

    if (rank == 0)
    {
        int part_length = (int)(matrix_size / comm_size);

        for (i = 0; i < comm_size; i++)
            counts[i] = part_length * matrix_size;

        for (i = 0; matrix_size % (part_length * comm_size + i) != 0; i++)
            counts[i % comm_size] += matrix_size;

        int amount = 0;
        displacements[0] = 0;

        for (i = 1; i < comm_size; i++)
        {
            amount += counts[i - 1];
            displacements[i] = amount;
        }
    }

    MPI_Bcast(counts, comm_size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(displacements, comm_size, MPI_INT, 0, MPI_COMM_WORLD);

    int total_size = rank == comm_size - 1 ? matrix_size * matrix_size - displacements[rank] : displacements[rank + 1] - displacements[rank];
    double *A_fragment = (double *)malloc(total_size * sizeof(double));

    MPI_Scatterv(matrix, counts, displacements, MPI_DOUBLE, A_fragment, total_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    return A_fragment;
}

double *multiply_submatrix_and_vector(double *submatrix, int matrix_size, double *vector, int *counts, int *displacements, int rank, int comm_size)
{
    double *result_vector = (double *)malloc(matrix_size * sizeof(double));
    int i, j;

    MPI_Bcast(vector, matrix_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    for (i = 0; i < counts[rank] / matrix_size; i++)
    {
        double sum = 0.0;

        for (j = 0; j < matrix_size; j++)
            sum += submatrix[i * matrix_size + j] * vector[j];

        result_vector[i] = sum;
    }

    return result_vector;
}

void gather_subvectors(double *subvector, double *final_result, int matrix_size, int *counts, int *displacements, int rank, int comm_size)
{
    int i;

    int *counts_cpy = malloc(comm_size * sizeof(int));
    int *displacements_cpy = malloc(comm_size * sizeof(int));

    memcpy(counts_cpy, counts, comm_size * sizeof(int));
    memcpy(displacements_cpy, displacements, comm_size * sizeof(int));

    for (i = 0; i < comm_size; i++)
    {
        counts_cpy[i] /= matrix_size;
        displacements_cpy[i] /= matrix_size;
    }

    MPI_Gatherv(subvector, counts_cpy[rank], MPI_DOUBLE, final_result, counts_cpy, displacements_cpy, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    free(counts_cpy);
    free(displacements_cpy);
}
