/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, k, l, a0, a1, a2, a3, a4, a5, a6, a7;
    if(M == 32){
       for (i = 0; i < N; i+=8) {
            for (j = 0; j < M; j+=8) {
                if(i == j){
                    for(k = i ;k < i + 8 && k<N;k++){ 
                        a0 = A[k][j];   
                        a1 = A[k][j+1];
                        a2 = A[k][j+2];
                        a3 = A[k][j+3];
                        a4 = A[k][j+4]; 
                        a5 = A[k][j+5];
                        a6 = A[k][j+6];
                        a7 = A[k][j+7];

                        B[j][k]   = a0;
                        B[j+1][k] = a1;
                        B[j+2][k] = a2;
                        B[j+3][k] = a3;
                        B[j+4][k] = a4;
                        B[j+5][k] = a5;
                        B[j+6][k] = a6;
                        B[j+7][k] = a7;
                    }

                }
                else{
                    for(k = i ;k < i + 8 && k<N;k++){
                        for(l = j ; l < j + 8 && l < M;l++)
                            B[l][k] = A[k][l];
                    }
                }
            }
        }     
    }
    else if(M == 64){
        for (i = 0; i < N; i += 8) {
        for (j = 0; j < M; j += 8) {
            for (k = i; k < i + 4; k++) {
                a0 = A[k][j];
                a1 = A[k][j + 1];
                a2 = A[k][j + 2];
                a3 = A[k][j + 3];
                a4 = A[k][j + 4];
                a5 = A[k][j + 5];
                a6 = A[k][j + 6];
                a7 = A[k][j + 7];

                B[j][k] = a0;
                B[j + 1][k] = a1;
                B[j + 2][k] = a2;
                B[j + 3][k] = a3;

                B[j][k + 4] = a4;
                B[j + 1][k + 4] = a5;
                B[j + 2][k + 4] = a6;
                B[j + 3][k + 4] = a7;
            }
            for (l = j + 4; l < j + 8; l++) {

                a4 = A[i + 4][l - 4]; // A left-down col
                a5 = A[i + 5][l - 4];
                a6 = A[i + 6][l - 4];
                a7 = A[i + 7][l - 4];

                a0 = B[l - 4][i + 4]; // B right-above line
                a1 = B[l - 4][i + 5];
                a2 = B[l - 4][i + 6];
                a3 = B[l - 4][i + 7];

                B[l - 4][i + 4] = a4; // set B right-above line 
                B[l - 4][i + 5] = a5;
                B[l - 4][i + 6] = a6;
                B[l - 4][i + 7] = a7;

                B[l][i] = a0;         // set B left-down line
                B[l][i + 1] = a1;
                B[l][i + 2] = a2;
                B[l][i + 3] = a3;

                B[l][i + 4] = A[i + 4][l];
                B[l][i + 5] = A[i + 5][l];
                B[l][i + 6] = A[i + 6][l];
                B[l][i + 7] = A[i + 7][l];
            }
        }
        }
    } 
    else if (M == 61){
        for (i = 0; i < N; i += 16) {
            for (j = 0; j < M; j += 16) {
                for (k = i; k < i + 16&& k<N; k++) {
                    for(l =j ;l<j+16&&l<M;l++)
                        B[l][k] = A[k][l];
                }
            }
        }
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

