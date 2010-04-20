/*
 * Matrix multiplication 
 *
 * 2009-2010 Michal Demin
 *
 */
#ifndef MEMS_MATRIX_H_
#define MEMS_MATRIX_H_

#include "mems_math.h"

#include "mems_vector.h"

struct matrix_t {
	frac coef [3][3];
};

void matrix_mul(struct vector_t *out, struct matrix_t *m, struct vector_t *v1);

#endif

