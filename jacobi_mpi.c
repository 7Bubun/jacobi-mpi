#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define ARG_NUM_ERROR_CODE 1
#define FILE_OPENING_ERROR_CODE 2
#define FILE_READING_ERROR_CODE 3


void exit_with_error(const char *message, const int exit_code) {
    fprintf(stderr, "%s\n", message);
    MPI_Finalize();
    exit(exit_code);
}

//args: A filename, b filename, number of iterations
int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    
    int comm_size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    double *A, *b;
    int matrix_size;

    if(rank == 0) {
        FILE *f;
        int i, j;
        
        if(argc != 4)
            exit_with_error("Niepoprawna liczba argumentow\n", ARG_NUM_ERROR_CODE);
        
        if((f = fopen(argv[1], "r")) == NULL)
            exit_with_error("Nie udalo sie otworzyc pliku z macierza A.\n", FILE_OPENING_ERROR_CODE);

        if(fscanf(f, "%d\n\n", &matrix_size) != 1)
            exit_with_error("Nie udalo sie wczytac rozmiaru macierzy A.\n", FILE_READING_ERROR_CODE);

        A = (double*) malloc(matrix_size * matrix_size * sizeof(double));

        for(i = 0; i < matrix_size; i++) {
            for(j = 0; j < matrix_size; j++) {
                if((fscanf(f, "%lf\n", &A[i * matrix_size + j]) != 1)) {
                    free(A);
                    exit_with_error("Nie udalo sie wczytac zawartosci macierzy A.\n", FILE_READING_ERROR_CODE);
                }
            }
        }

        fclose(f);

        if((f = fopen(argv[2], "r")) == NULL) {
            free(A);
            exit_with_error("Nie udalo sie otworzyc pliku z wektorem b.\n", FILE_OPENING_ERROR_CODE); 
        }

        b = (double*) malloc(matrix_size * sizeof(double));

        for(i = 0; i < matrix_size; i++) {
            if((fscanf(f, "%lf\n", &b[i])) != 1) {
                free(A);
                free(b);
                exit_with_error("Nie udalo sie wczytac zawartosci wektora b.\n", FILE_READING_ERROR_CODE);
            }
        }

        fclose(f);

        //TEST
        /*
        printf("A:\n");
        for(i = 0; i < matrix_size; i++) {
            for(j = 0; j < matrix_size; j++) {
                printf("%lf ", A[i * matrix_size + j]);
            }

            printf("\n");
        }

        printf("B:\n");
        for(i = 0; i < matrix_size; i++) {
            printf("%lf\n", b[i]);
        }
        */
    }


    
    if(rank == 0) {
       free(A);
       free(b);
    }

    MPI_Finalize();
    return 0;
}
