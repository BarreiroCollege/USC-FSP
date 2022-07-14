#include <math.h>
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#define MASTER 0
#define COLS 16


void print(double M[COLS][COLS]) {
    for (int i=0; i < COLS; i++) {
        for (int j=0; j < COLS; j++) {
            printf("%lf\t", M[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

double time_diff(struct timeval *start, struct timeval *end) {
    return (end->tv_sec - start->tv_sec) + 1e-6*(end->tv_usec - start->tv_usec);
}


int main(int argc, char** argv) {
    int numtasks, taskid, len;
    char hostname[MPI_MAX_PROCESSOR_NAME];

    struct timeval start, end;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
    MPI_Get_processor_name(hostname, &len);

    if (numtasks != 1 && numtasks != 2 && numtasks != 4 && numtasks != 8 && numtasks != 16) {
        printf("Número de tareas %d incorrecto\n", numtasks);
        return EXIT_FAILURE;
    }
    int C = COLS / numtasks;

    double M[COLS][COLS] = {
        {4., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.},
        {3., 2., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.},
        {6., 7., 7., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.},
        {2., 8., 3., 2., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.},
        {9., 2., 2., 5., 9., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.},
        {1., 4., 1., 9., 6., 3., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.},
        {2., 5., 8., 4., 4., 5., 5., 0., 0., 0., 0., 0., 0., 0., 0., 0.},
        {9., 1., 8., 1., 8., 6., 2., 6., 0., 0., 0., 0., 0., 0., 0., 0.},
        {4., 5., 2., 8., 6., 5., 1., 9., 4., 0., 0., 0., 0., 0., 0., 0.},
        {3., 2., 4., 8., 3., 4., 5., 2., 3., 2., 0., 0., 0., 0., 0., 0.},
        {6., 7., 7., 6., 1., 7., 4., 1., 6., 7., 7., 0., 0., 0., 0., 0.},
        {2., 8., 3., 2., 1., 6., 2., 9., 2., 8., 3., 2., 0., 0., 0., 0.},
        {9., 2., 2., 5., 9., 3., 8., 2., 9., 2., 2., 5., 9., 0., 0., 0.},
        {1., 4., 1., 9., 6., 3., 7., 6., 1., 4., 1., 9., 6., 3., 0., 0.},
        {2., 5., 8., 4., 4., 5., 5., 3., 2., 5., 8., 4., 4., 5., 5., 0.},
        {9., 1., 8., 1., 8., 6., 2., 6., 9., 1., 8., 1., 8., 6., 2., 6.},
    };

    double MT[COLS][COLS];
    double MAT[COLS * COLS];
    if (taskid == MASTER) {
        print(M);
        printf("\n");

        /* double M3[COLS][COLS];
        for (int i = 0; i < COLS; i++) {
            for (int j = 0; j < COLS; j++) {
                M3[i][j] = 0.;
            }
        }

        for (int j=0; j < COLS; j++) {
            M3[j][j] = 1. / M[j][j];

            for (int i=j+1; i < COLS; i++) {
                for (int k=j; k < i; k++) {
                    M3[i][j] += M[i][k] * M3[k][j];
                }
                M3[i][j] /= -M[i][i];
            }
        }
        print(M3);
        printf("\n"); */

        for (int i=0; i < COLS; i++) {
            for (int j=i; j < COLS; j++) {
                MT[i][j] = M[j][i];
                if (i != j) MT[j][i] = M[i][j];
            }
        }
        
        for (int i=0; i < COLS; i++) {
            for (int j=i; j < COLS; j++) {
                MAT[i * COLS + j] = MT[i][j];
                if (i != j) MAT[j * COLS + i] = MT[j][i];
            }
        }
        gettimeofday(&start, NULL);
    }

    int N = COLS * C;
    double MLOCAL[N], MLOCAL2[N];
    MPI_Scatter(
        MAT, N, MPI_DOUBLE,
        MLOCAL, N, MPI_DOUBLE,
        MASTER, MPI_COMM_WORLD
    );

    for (int i = 0; i < N; i++) MLOCAL2[i] = 0.;

    for (int col = 0; col < C; col++) {
        int j = taskid + col;
        int offset = col * COLS;
        MLOCAL2[j + offset] = 1. / MLOCAL[j + offset];

        for (int i=j+1; i < COLS; i++) {
            for (int k=j; k < i; k++) {
                MLOCAL2[i + offset] += M[i][k] * MLOCAL2[k + offset];
            }
            MLOCAL2[i + offset] /= -M[i][i];
        }
    }

    double RESP[COLS * COLS];
    MPI_Gather(
        MLOCAL2, N, MPI_DOUBLE,
        RESP, N, MPI_DOUBLE,
        MASTER, MPI_COMM_WORLD
    );

    if (taskid == MASTER) {
        gettimeofday(&end, NULL);
        double T = time_diff(&start, &end);
        printf("T=%lf\n", T);

        double M2[COLS][COLS];
        for (int i=0; i < COLS; i++) {
            for (int j=0; j < COLS; j++) {
                M2[i][j] = RESP[j*COLS + i];
            }
        }
        print(M2);
        printf("\n");
    }

    MPI_Finalize();

    return EXIT_SUCCESS;
}
