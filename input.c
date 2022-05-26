#include <stdio.h>
#include <stdlib.h>
#include "headers.h"

#define FILE_OPENING_ERROR_CODE 2
#define FILE_READING_ERROR_CODE 3

int read_matrix_size(const char *filename)
{
    int result;
    FILE *f = fopen(filename, "r");

    if (f == NULL)
        exit_with_error("Nie udalo sie otworzyc pliku z macierza A.\n", FILE_OPENING_ERROR_CODE);

    if (fscanf(f, "%d\n\n", &result) != 1)
        exit_with_error("Nie udalo sie wczytac rozmiaru macierzy A.\n", FILE_READING_ERROR_CODE);

    fclose(f);
    return result;
}

double *read_matrix(const char *filename, int matrix_size)
{
    FILE *f;
    int i, j;

    if ((f = fopen(filename, "r")) == NULL)
        exit_with_error("Nie udalo sie otworzyc pliku z macierza A.\n", FILE_OPENING_ERROR_CODE);

    fscanf(f, "%d\n\n", &i);

    double *A = (double *)malloc(matrix_size * matrix_size * sizeof(double));

    for (i = 0; i < matrix_size; i++)
    {
        for (j = 0; j < matrix_size; j++)
        {
            if ((fscanf(f, "%lf\n", &A[i * matrix_size + j]) != 1) || (i == j && A[i * matrix_size + j] == 0))
            {
                free(A);
                exit_with_error("Niepoprawny format macierzy A.\n", FILE_READING_ERROR_CODE);
            }
        }
    }

    fclose(f);
    return A;
}

double *read_vector(const char *filename, int vector_size)
{
    FILE *f;
    int i;

    double *b = (double *)malloc(vector_size * sizeof(double));

    if ((f = fopen(filename, "r")) == NULL)
        exit_with_error("Nie udalo sie otworzyc pliku z wektorem b.\n", FILE_OPENING_ERROR_CODE);

    for (i = 0; i < vector_size; i++)
    {
        if ((fscanf(f, "%lf\n", &b[i])) != 1)
        {
            free(b);
            exit_with_error("Nie udalo sie wczytac zawartosci wektora b.\n", FILE_READING_ERROR_CODE);
        }
    }

    fclose(f);
    return b;
}
