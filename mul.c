/*
 * mul.c - Matrix multiply C = A * B
 *
 * Each multiply function must have a prototype of the form:
 * void mul(int M, int N, int A[N][M], int B[M][N], int C[N][N]);
 *
 * A multiply function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include <stdio.h>
#include "cachelab.h"

int is_mul(int M, int N, int A[N][M], int B[M][N], int C[N][N]);

/*
 * mul_submit - This is the solution multiply function that you will be
 *     graded on for Part C of the assignment. Do not change the
 *     description string "Multiply submission", as the driver searches
 *     for that string to identify the multiply function to be graded.
 *     C starts out holding garbage, so every element of C must be written.
 */
char mul_submit_desc[] = "Multiply submission";
void mul_submit(int M, int N, int A[N][M], int B[M][N], int C[N][N])
{
    int i0, j0, k0, ii, kk;
    int a0, a1, a2, a3, a4, a5, a6, a7, t;

    for (i0 = 0; i0 < N; i0 += 8) {
        for (j0 = 0; j0 < N; j0 += 8) {
            for (k0 = 0; k0 < M; k0 += 8) {
                for (ii = i0; ii < i0 + 8 && ii < N; ii++) {
                    if (k0 == 0) {
                        a0 = a1 = a2 = a3 = a4 = a5 = a6 = a7 = 0;
                    } else {
                        a0 = C[ii][j0];
                        a1 = C[ii][j0 + 1];
                        a2 = C[ii][j0 + 2];
                        a3 = C[ii][j0 + 3];
                        a4 = C[ii][j0 + 4];
                        a5 = C[ii][j0 + 5];
                        a6 = C[ii][j0 + 6];
                        a7 = C[ii][j0 + 7];
                    }
                    for (kk = k0; kk < k0 + 8 && kk < M; kk++) {
                        t = A[ii][kk];
                        a0 += t * B[kk][j0];
                        a1 += t * B[kk][j0 + 1];
                        a2 += t * B[kk][j0 + 2];
                        a3 += t * B[kk][j0 + 3];
                        a4 += t * B[kk][j0 + 4];
                        a5 += t * B[kk][j0 + 5];
                        a6 += t * B[kk][j0 + 6];
                        a7 += t * B[kk][j0 + 7];
                    }
                    C[ii][j0] = a0;
                    C[ii][j0 + 1] = a1;
                    C[ii][j0 + 2] = a2;
                    C[ii][j0 + 3] = a3;
                    C[ii][j0 + 4] = a4;
                    C[ii][j0 + 5] = a5;
                    C[ii][j0 + 6] = a6;
                    C[ii][j0 + 7] = a7;
                }
            }
        }
    }
}

/*
 * mul - A simple baseline multiply function, not optimized for the cache.
 */
char mul_desc[] = "Simple i-j-k multiply";
void mul(int M, int N, int A[N][M], int B[M][N], int C[N][N])
{
    int i, j, k, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            tmp = 0;
            for (k = 0; k < M; k++)
                tmp += A[i][k] * B[k][j];
            C[i][j] = tmp;
        }
    }
}

/*
 * registerFunctions - This function registers your multiply functions
 *     with the driver. At runtime, the driver will evaluate each of the
 *     registered functions and summarize their performance. This is a
 *     handy way to experiment with different multiply strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerMulFunction(mul_submit, mul_submit_desc);

    /* Register any additional multiply functions */
    registerMulFunction(mul, mul_desc);
}

/*
 * is_mul - This helper function checks if C is the product of A and B.
 *     You can check the correctness of your multiply by calling it
 *     before returning from the multiply function.
 */
int is_mul(int M, int N, int A[N][M], int B[M][N], int C[N][N])
{
    int i, j, k, num;

    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            num = 0;
            for (k = 0; k < M; k++)
                num += A[i][k] * B[k][j];
            if (num != C[i][j])
                return 0;
        }
    }
    return 1;
}
