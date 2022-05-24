#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "headers.h"


void exit_with_error(const char *message, const int exit_code) {
    fprintf(stderr, "%s\n", message);
    MPI_Finalize();
    exit(exit_code);
}
