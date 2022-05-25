#ifndef INPUT_H
#define INPUT_H

double *read_matrix(const char *filename, int *matrix_size);

double *read_vector(const char *filename, int vector_size, double *matrix);

void exit_with_error(const char *message, const int exit_code);

double *divide_matrix_into_submatrixes(double *matrix, int matrix_size, int *counts, int *displacements, int rank, int comm_size);

double *multiply_submatrix_and_vector(double *submatrix, int matrix_size, double *vector, int *counts, int *displacements, int rank, int comm_size);

double *gather_subvectors(double *result_vector, int matrix_size, int *counts, int *displacements, int rank, int comm_size);

#endif
