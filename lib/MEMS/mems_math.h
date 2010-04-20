/*
 * Fixed point aritmetics
 *
 * 2009-2010 Michal Demin
 * 
 * http://infocenter.arm.com/help/topic/com.arm.doc.dai0033a/DAI0033A_fixedpoint_appsnote.pdf
 * http://home.utah.edu/~nahaj/factoring/squarerootconstraints.halleck.c.html
 */

#ifndef MEMS_MATH_H_
#define MEMS_MATH_H_

#include <stdint.h>

#ifdef MEMS_FIXEDPOINT

#define Q                14
#define K                (1 << ((Q) - 1))

#define mems_ZERO        0
#define mems_ONE         (1 << Q)
#define mems_TWO         (1 << ((Q)+1))

typedef int32_t frac;
typedef int64_t accum;

#define TODOUBLE(d, q)   ((double)(d) / (double)(1<<(q)))
#define TOFP(d, q)       ((frac) ((d) * (double)(1<<(q))))

extern accum isqrt_rem;
accum isqrt_rem;

inline frac mems_add(frac a, frac b) {
	return a+b;
}

inline frac mems_sub(frac a, frac b) {
	return a-b;
}

inline frac mems_mul(frac a, frac b) {
	accum tmp = (accum)a * (accum) b;
	tmp += K; // round midrange
	return tmp>>Q;
}

inline frac mems_div(frac a, frac b) {
	accum tmp = (accum)a << Q;
	tmp += b/2; // round midrange
	return (frac)tmp/b;
}

inline frac mems_addi(frac a, int16_t b) {
	return a+(b<<Q);
}

inline frac mems_subi(frac a, int16_t b) {
	return a-(b<<Q);
}

inline frac mems_muli(frac a, int16_t b) {
	return a*b;
}

inline frac mems_divi(frac a, int16_t b) {
	return a/b;
}

accum mems_sqrt (accum x);
#else
/* floating point math */
#include <math.h>

#define mems_ZERO        0.0
#define mems_ONE         1.0
#define mems_TWO         2.0

typedef double frac;
typedef double accum;

#define TODOUBLE(d, q)   (d)
#define TOFP(d, q)       (d)

#define mems_add(a, b) ((a)+(b))
#define mems_sub(a, b) ((a)-(b))
#define mems_mul(a, b) ((a)*(b))
#define mems_div(a, b) ((a)/(b))
#define mems_addi(a, b) ((a)+(b))
#define mems_subi(a, b) ((a)-(b))
#define mems_muli(a, b) ((a)*(b))
#define mems_divi(a, b) ((a)/(b))
#define mems_sqrt(x) sqrt(x)


#endif
#endif
