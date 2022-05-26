#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <mpi.h>
#include "headers.h"

#define ARG_NUM_ERROR_CODE 1
#define MAX_ERR 1e-9

// args: A filename, b filename, number of iterations
int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int comm_size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    clock_t start, after_input, end;

    if (rank == 0)
        start = clock();

    double *A, *b;
    int matrix_size;

    if (rank == 0)
    {
        if (argc != 4)
            exit_with_error("Niepoprawna liczba argumentow\n", ARG_NUM_ERROR_CODE);

        A = read_matrix(argv[1], &matrix_size);
        b = read_vector(argv[2], matrix_size, A);
    }

    if (rank == 0)
        after_input = clock();

    MPI_Bcast(&matrix_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank != 0)
        b = (double *)malloc(matrix_size * sizeof(double));

    int *displacements = (int *)malloc(comm_size * sizeof(int));
    int *counts = (int *)malloc(comm_size * sizeof(int));

    double *I_minus_diag_inv_A;
    double *diag_inv_times_b;

    if (rank == 0)
    {
        double *diag_inv = (double *)malloc(matrix_size * sizeof(double));
        diag_inv_times_b = (double *)malloc(matrix_size * sizeof(double));
        I_minus_diag_inv_A = (double *)malloc(matrix_size * matrix_size * sizeof(double));

        for (int i = 0; i < matrix_size; i++)
        {
            diag_inv[i] = 1 / A[i + i * matrix_size];
            diag_inv_times_b[i] = diag_inv[i] * b[i];
        }

        for (int i = 0; i < matrix_size; i++)
        {
            for (int j = 0; j < matrix_size; j++)
            {
                I_minus_diag_inv_A[i * matrix_size + j] = -diag_inv[i] * A[i * matrix_size + j];

                if (i == j)
                    I_minus_diag_inv_A[i * matrix_size + j]++; // should be = 0
            }
        }

        free(diag_inv);
    }

    double *submatrix = divide_matrix_into_submatrixes(I_minus_diag_inv_A, matrix_size, counts, displacements, rank, comm_size);
    double *submatrix2 = divide_matrix_into_submatrixes(A, matrix_size, counts, displacements, rank, comm_size);
    double *X = (double *)malloc(matrix_size * sizeof(double));

    if (rank == 0)
        memcpy(X, diag_inv_times_b, matrix_size * sizeof(double));

    double err = 1000000.0;
    int i;

    for (i = 1; i < atoi(argv[3]) && err > MAX_ERR * matrix_size; i++)
    {
        MPI_Bcast(X, matrix_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        double *part_of_result = multiply_submatrix_and_vector(submatrix, matrix_size, X, counts, displacements, rank, comm_size);
        gather_subvectors(part_of_result, X, matrix_size, counts, displacements, rank, comm_size);

        if (rank == 0)
        {
            for (int j = 0; j < matrix_size; j++)
                X[j] += diag_inv_times_b[j];
        }

        free(part_of_result);

        if (i % 10 == 0)
        {
            double *R = (double *)malloc(matrix_size * sizeof(double));
            double *part_of_result2 = multiply_submatrix_and_vector(submatrix2, matrix_size, X, counts, displacements, rank, comm_size);
            gather_subvectors(part_of_result2, R, matrix_size, counts, displacements, rank, comm_size);

            if (rank == 0)
            {
                err = 0.0;

                for (int k = 0; k < matrix_size; k++)
                    err += pow(R[k] - b[k], 2);

                err = sqrt(err);
            }

            free(R);
            free(part_of_result2);
            MPI_Bcast(&err, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        }
    }

    if (rank == 0)
    {
        end = clock();
        int old_i = i;
        printf("Wynik:\n");

        for (i = 0; i < matrix_size; i++)
            printf("%0.8lf\n", X[i]);

        printf("Zakonczono po %d iteracjach.\n", old_i);
        printf("Czas dzialania programu: %lf s.\n", ((double)(end - start)) / CLOCKS_PER_SEC);
        printf("Czas obliczen (czas dzialania programu bez czasu wczytywania danych): %lf s.\n", ((double)(end - after_input)) / CLOCKS_PER_SEC);
    }

    if (rank == 0)
    {
        free(A);
        free(diag_inv_times_b);
        free(I_minus_diag_inv_A);
    }

    free(submatrix);
    free(submatrix2);
    free(X);
    free(b);
    free(displacements);
    free(counts);
    MPI_Finalize();
    return 0;
}
