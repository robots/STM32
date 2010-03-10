/*
 * Fixed point aritmetics 
 *
 * 2009-2010 Michal Demin
 *
 * inspired by various sources
 *
 */

#include "fixed-point.h"
/* http://infocenter.arm.com/help/topic/com.arm.doc.dai0033a/DAI0033A_fixedpoint_appsnote.pdf*/
/* http://home.utah.edu/~nahaj/factoring/squarerootconstraints.halleck.c.html */

accum isqrt_rem;

 frac fp_add(frac a, frac b) {
	return a+b;
}

 frac fp_sub(frac a, frac b) {
	return a-b;
}

 frac fp_mul(frac a, frac b) {
	accum tmp = (accum)a * (accum) b;
	tmp += K; // round midrange
	return tmp>>Q;
}

 frac fp_div(frac a, frac b) {
	accum tmp = (accum)a << Q;
	tmp += b/2; // round midrange
	return (frac)tmp/b;
}

 frac fp_addi(frac a, int16_t b) {
	return a+(b<<Q);
}

 frac fp_subi(frac a, int16_t b) {
	return a-(b<<Q);
}

 frac fp_muli(frac a, int16_t b) {
	return a*b;
}

 frac fp_divi(frac a, int16_t b) {
	return a/b;
}

accum isqrt (accum x) {
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

