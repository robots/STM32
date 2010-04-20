#include "mems_norm.h"

/* applies coeficients using horners scheme
 * args:
 * *norm - pointer to the structure with coeficients
 * z     - input value
 *
 */
frac norm_process(struct norm_t *norm, frac z) {
	frac out = mems_ZERO;
	int i;

	for (i=0; i<norm->coef_num; i++) {
		out = mems_add(mems_mul(out, z), norm->coef[i]);
	}

	return out;

}

