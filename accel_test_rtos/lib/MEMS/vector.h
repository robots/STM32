/*
 * Vector manipulation 
 *
 * 2009-2010 Michal Demin
 *
 */
#ifndef VECTOR_H_
#define VECTOR_H_

#include "fixed-point.h"

struct vector_t {
	frac coef[3];
};


frac vector_dot(struct vector_t* v1, struct vector_t *v2); 
struct vector_t* vector_cross(struct vector_t *out, struct vector_t* v1, struct vector_t *v2);
frac vector_len(struct vector_t *v);
void vector_scale(struct vector_t *v, frac);
void  vector_normalize(struct vector_t *v);
struct vector_t *vector_add(struct vector_t *out, struct vector_t* v1, struct vector_t *v2);

#endif

