
#include "matrix.h"

#include <math.h>
#include <stdio.h>

#define at(mat, n, row, col) ((mat)[(row)*(n+1) + (col)])

void swap(double *mat, int n, int r1, int r2)
{
	double temp;
	int j;
	
	for (j=r1; j<n+1; j++) {
		temp = at(mat, n, r1, j);
		at(mat, n, r1, j) = at(mat, n, r2, j);
		at(mat, n, r2, j) = temp;
	}
}

void add(double *mat, int n, int r1, int r2, double f)
{
	int j;
	
	for (j=r1+1; j<n+1; j++) {
		at(mat, n, r2, j) += f * at(mat, n, r1, j);
	}
}

void pivot(double *mat, int n, int i)
{
	int best_row;
	double best;
	double here;
	int k;
	
	best = fabs(at(mat, n, i, i));
	best_row = i;
	
	for (k=i+1; k<n; k++) {
		here = fabs(at(mat, n, k, i));
		
		if (here > best) {
			best = here;
			best_row = k;
		}
	}
	
	if (best_row != i) {
		swap(mat, n, i, best_row);
	}
}

void elim(double *mat, int n, int i, int k)
{
	double f;
	
	f = - at(mat, n, k, i) / at(mat, n, i, i);
	add(mat, n, i, k, f);
}

void gr_solve(double *sol, double *mat, int n)
{
	int i, k;
	double sum;
	
	for (i=0; i<n-1; i++) {
		pivot(mat, n, i);
		
		for (k=i+1; k<n; k++) {
			elim(mat, n, i, k);
		}
	}
	
	for (i=n-1; i>=0; i--) {
		sum = at(mat, n, i, n);
		
		for (k=i+1; k<n; k++) {
			sum -= at(mat, n, i, k) * sol[k];
		}
		
		sol[i] = sum / at(mat, n, i, i);
	}
}

void print_mat(double *mat, int m, int n) {
	int i, j;
	
	for (i=0; i<m; i++) {
		
		for (j=0; j<n; j++) {
			printf("%9.3e ", mat[j + i*n]);
		}
		
		printf("\n");
	}
	
	printf("\n");
}
