
#ifndef _matrix_h
#define _matrix_h 1

#ifdef __cplusplus
extern "C" {
#endif

void gr_solve(double *sol, double *mat, int n);

void print_mat(double *mat, int m, int n);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* defined _matrix_h */
