#include <float.h>
#include <limits.h>
#include <math.h>
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#define PI_REF  3.1415926535897932384626433832795028841971693993751058209749446
#define MASTER	0
#define THROWS  1000000


double time_diff(struct timeval *start, struct timeval *end) {
    return (end->tv_sec - start->tv_sec) + 1e-6*(end->tv_usec - start->tv_usec);
}

double f(double x) {
    return 1.0 / sqrt(1.0 - x*x);
}


int main(int argc, char** argv) {
    int numtasks, taskid, len;
    char hostname[MPI_MAX_PROCESSOR_NAME];

    struct timeval start, end;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
    MPI_Get_processor_name(hostname, &len);

    gettimeofday(&start, NULL);

    double a = -1.0, b = 1.0;
    double h = fabs(b-a) / (double) (THROWS*numtasks), x, sum = 0.0;

    for (int i=(THROWS*taskid)+1; i < (THROWS*(taskid+1)); i++) {
        x = a + i*h;
        sum += f(x);
    }

    if (taskid == MASTER) {
        double sumRecv;
        MPI_Status status;
        for (int i = 1; i < numtasks; i++) {
            MPI_Recv(&sumRecv, 1, MPI_DOUBLE, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);
            sum += sumRecv;
        }

        double pi = (h/2.0) * (f(a+0.0001) + f(b-0.0001) + 2.0*sum);

        gettimeofday(&end, NULL);
        double T = time_diff(&start, &end);

        printf("PI('Trapecio', %d) = %lf\n", numtasks, pi);

        double E = fabs(PI_REF - pi);
        double C = 1.0 / (T*E);

        printf("T=%lf\n", T);
        printf("E=%lf\n", E);
        printf("C=%lf\n", C);
    } else {
        MPI_Send(&sum, 1, MPI_DOUBLE, MASTER, 1, MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return EXIT_SUCCESS;
}
