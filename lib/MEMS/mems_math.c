#include "mems_math.h"

#ifdef MEMS_FIXEDPOINT

accum mems_sqrt (accum x) {
	accum squaredbit, root;

	if (x<1) return 0;

	squaredbit  = (accum) ((((uint64_t) ~0L) >> 1) & ~(((uint64_t) ~0L) >> 2));

	/* Form bits of the answer. */
	isqrt_rem = x;
	root = 0;

	while (squaredbit > 0) {
		if (isqrt_rem >= (squaredbit | root)) {
			isqrt_rem -= (squaredbit | root);
			root >>= 1;
			root |= squaredbit;
		} else { 
			root >>= 1; 
		}
		squaredbit >>= 2; 
	}
	return root;
}

#endif
