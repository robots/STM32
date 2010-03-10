/*
 * quaternion 
 *
 * 2009-2010 Michal Demin
 *
 * parts of code based on work done by
 *
 * David Eberly in Wild Magic 4.0
 */

#include "fixed-point.h"

#include "vector.h"
#include "matrix.h"

#include "quaternion.h"

void quaternion_toRotMatrix(struct quaternion_t *q, struct matrix_t *m) {
	frac fTx = fp_mul(FP_TWO, q->coef[1]);
	frac fTy = fp_mul(FP_TWO, q->coef[2]);
	frac fTz = fp_mul(FP_TWO, q->coef[3]);
	
	frac fTwx = fp_mul(fTx, q->coef[0]);
	frac fTwy = fp_mul(fTy, q->coef[0]);
	frac fTwz = fp_mul(fTz, q->coef[0]);
	frac fTxx = fp_mul(fTx, q->coef[1]);
	frac fTxy = fp_mul(fTy, q->coef[1]);
	frac fTxz = fp_mul(fTz, q->coef[1]);
	frac fTyy = fp_mul(fTy, q->coef[2]);
	frac fTyz = fp_mul(fTz, q->coef[2]);
	frac fTzz = fp_mul(fTz, q->coef[3]);

	m->coef[0][0] = fp_sub(FP_ONE, fp_add(fTyy, fTzz));
	m->coef[1][1] = fp_sub(FP_ONE, fp_add(fTxx, fTzz));
	m->coef[2][2] = fp_sub(FP_ONE, fp_add(fTxx, fTyy));
	m->coef[0][2] = fp_add(fTxz, fTwy);
	m->coef[1][0] = fp_add(fTxy, fTwz);
	m->coef[2][1] = fp_add(fTyz, fTwx);
	m->coef[1][2] = fp_sub(fTyz, fTwx);
	m->coef[2][0] = fp_sub(fTxz, fTwy);
	m->coef[0][1] = fp_sub(fTxy, fTwz);
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

