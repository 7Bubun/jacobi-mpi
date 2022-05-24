#ifndef INPUT_H
#define INPUT_H

double *read_matrix(const char *filename, int *matrix_size);

double *read_vector(const char *filename, int vector_size, double *matrix);

void exit_with_error(const char *message, const int exit_code);

#endif
