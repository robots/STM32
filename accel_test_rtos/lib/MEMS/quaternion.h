/*
 * quaternion 
 *
 * 2009-2010 Michal Demin
 *
 */
#ifndef QUATERNION_H_
#define QUATERNION_H_

#include "fixed-point.h"

#include "matrix.h"
#include "vector.h"

struct quaternion_t {
	frac coef[4];
};

void quaternion_toRotMatrix(struct quaternion_t *q, struct matrix_t *m);
struct vector_t *quaternion_rotate(struct quaternion_t *q, struct vector_t *v);
struct quaternion_t *quaternion_align(struct quaternion_t *q, struct vector_t *v1, struct vector_t *v2); 

#endif
