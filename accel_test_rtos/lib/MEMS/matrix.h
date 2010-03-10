/*
 * Matrix multiplication 
 *
 * 2009-2010 Michal Demin
 *
 */
#ifndef MATRIX_H_
#define MATRIX_H_

#include "fixed-point.h"

#include "vector.h"

struct matrix_t {
	frac coef [3][3];
};

void matrix_mul(struct vector_t *out, struct matrix_t *m, struct vector_t *v1);

#endif

