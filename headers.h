#ifndef INPUT_H
#define INPUT_H

double *read_matrix(const char *filename, int *matrix_size);

double *read_vector(const char *filename, int vector_size, double *matrix);

void exit_with_error(const char *message, const int exit_code);

double *multiply_matrix_and_vector_paralell(double *A, int A_size, double *B, int rank, int comm_size);

#endif
