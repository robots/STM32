/*
 * Vector manipulation
 *
 * 2009-2010 Michal Demin
 *
 */
#include "mems_vector.h"

frac vector_dot(struct vector_t* v1, struct vector_t *v2) {
	return mems_mul(v1->coef[0], v2->coef[0]) +
		mems_mul(v1->coef[1], v2->coef[1]) +
		mems_mul(v1->coef[2], v2->coef[2]);
}

struct vector_t* vector_cross(struct vector_t *out, struct vector_t* v1, struct vector_t *v2) {
	out->coef[0] = mems_mul(v1->coef[1], v2->coef[2]) - mems_mul(v1->coef[2], v2->coef[1]);
	out->coef[1] = mems_mul(v1->coef[2], v2->coef[0]) - mems_mul(v1->coef[0], v2->coef[2]);
	out->coef[2] = mems_mul(v1->coef[0], v2->coef[1]) - mems_mul(v1->coef[1], v2->coef[0]);
	return out;
}

frac vector_len(struct vector_t *v) {
	// the tmp value is in 2Q-format, but running it through isqrt changes it back to Q-format
	accum tmp = v->coef[0] * v->coef[0] + v->coef[1] * v->coef[1] +	v->coef[2] * v->coef[2];
	return sqrt(tmp);
}


void vector_scale(struct vector_t *v, frac len) {
	v->coef[0] = mems_mul(len, v->coef[0]);
	v->coef[1] = mems_mul(len, v->coef[1]);
	v->coef[2] = mems_mul(len, v->coef[2]);
}

void vector_normalize(struct vector_t *v) {
	frac len = vector_len(v);
	v->coef[0] = mems_div(v->coef[0], len);
	v->coef[1] = mems_div(v->coef[1], len);
	v->coef[2] = mems_div(v->coef[2], len);
}

struct vector_t *vector_add(struct vector_t *out, struct vector_t* v1, struct vector_t *v2) {
	out->coef[0] = v1->coef[0] + v2->coef[0];
	out->coef[1] = v1->coef[1] + v2->coef[1];
	out->coef[2] = v1->coef[2] + v2->coef[2];
	return out;
}

