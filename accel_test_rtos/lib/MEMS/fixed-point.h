/*
 * Fixed point aritmetics 
 *
 * 2009-2010 Michal Demin
 *
 */
#ifndef FIXED_POINT_H_
#define FIXED_POINT_H_


#include <stdint.h>

#define Q              14
#define K              (1 << ((Q) - 1))

#define FP_ONE         (1 << Q)
#define FP_TWO         (1 << ((Q)+1))

typedef int32_t frac;
typedef int64_t accum;

#define TODOUBLE(d, q) ((double)(d) / (double)(1<<(q)))
#define TOFP(d, q)     ((frac) ((d) * (double)(1<<(q))))

extern accum isqrt_rem;

accum isqrt (accum x);
frac fp_add(frac a, frac b);
frac fp_sub(frac a, frac b);
frac fp_mul(frac a, frac b);
frac fp_div(frac a, frac b);
frac fp_addi(frac a, int16_t b);
frac fp_subi(frac a, int16_t b);
frac fp_muli(frac a, int16_t b);
frac fp_divi(frac a, int16_t b);

#endif
