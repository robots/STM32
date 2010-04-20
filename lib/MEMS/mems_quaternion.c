/*
 * quaternion 
 *
 * 2009-2010 Michal Demin
 *
 * parts of code based on work done by
 *
 * David Eberly in Wild Magic 4.0
 */

#include "mems_math.h"

#include "mems_vector.h"
#include "mems_matrix.h"

#include "mems_quaternion.h"

void quaternion_toRotMatrix(struct quaternion_t *q, struct matrix_t *m) {
	frac fTx = mems_mul(mems_TWO, q->coef[1]);
	frac fTy = mems_mul(mems_TWO, q->coef[2]);
	frac fTz = mems_mul(mems_TWO, q->coef[3]);

	frac fTwx = mems_mul(fTx, q->coef[0]);
	frac fTwy = mems_mul(fTy, q->coef[0]);
	frac fTwz = mems_mul(fTz, q->coef[0]);
	frac fTxx = mems_mul(fTx, q->coef[1]);
	frac fTxy = mems_mul(fTy, q->coef[1]);
	frac fTxz = mems_mul(fTz, q->coef[1]);
	frac fTyy = mems_mul(fTy, q->coef[2]);
	frac fTyz = mems_mul(fTz, q->coef[2]);
	frac fTzz = mems_mul(fTz, q->coef[3]);

	m->coef[0][0] = mems_sub(mems_ONE, mems_add(fTyy, fTzz));
	m->coef[1][1] = mems_sub(mems_ONE, mems_add(fTxx, fTzz));
	m->coef[2][2] = mems_sub(mems_ONE, mems_add(fTxx, fTyy));
	m->coef[0][2] = mems_add(fTxz, fTwy);
	m->coef[1][0] = mems_add(fTxy, fTwz);
	m->coef[2][1] = mems_add(fTyz, fTwx);
	m->coef[1][2] = mems_sub(fTyz, fTwx);
	m->coef[2][0] = mems_sub(fTxz, fTwy);
	m->coef[0][1] = mems_sub(fTxy, fTwz);
}

struct vector_t *quaternion_rotate(struct quaternion_t *q, struct vector_t *v) {
	int i;
	struct matrix_t tmpMatrix;
	struct vector_t tmpVector;
	
	for (i=0; i<3; i++)
		tmpVector.coef[i] = v->coef[i];

	quaternion_toRotMatrix(q, &tmpMatrix);
	matrix_mul(v, &tmpMatrix, &tmpVector);

	return v;
}

struct quaternion_t *quaternion_align(struct quaternion_t *q, struct vector_t *v1, struct vector_t *v2) {
	struct vector_t bis;
	struct vector_t cross;
	frac angleCosHalf;

	// first we compute the bisector of V and (0,0,1)
	// bis = V + (0,0,1)

	vector_add(&bis, v1, v2);
	vector_normalize(&bis);

	// compute the dot product
	// which is the angle between bisector and vector V
	angleCosHalf = vector_dot(&bis, v1); 

	// compute the cross product
	// which is the vector perpediculat to plane defined by bis and V
	vector_cross(&cross, &bis, v1);

	q->coef[1] = cross.coef[0];
	q->coef[2] = cross.coef[1];
	q->coef[3] = cross.coef[2];

	q->coef[0] = angleCosHalf;
	return q;
}

