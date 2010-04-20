/*
 * Matrix multiplication 
 *
 * 2009-2010 Michal Demin
 *
 */
#include "mems_matrix.h"

void matrix_mul(struct vector_t *out, struct matrix_t *m, struct vector_t *v1) {
	int i;
	int j;

	for (i=0; i<3; i++) {
		out->coef[i] = mems_ZERO;
		for (j=0; j<3; j++) {
			frac tmp = mems_mul(m->coef[i][j], v1->coef[j]);
			out->coef[i] = mems_add(out->coef[i], tmp);
		}
	}

}

