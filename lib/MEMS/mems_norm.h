#ifndef MEMS_NORM_h_
#define MEMS_NORM_h_

#include "mems_math.h"

struct norm_t {
	int coef_num;
	frac coef[3];
};

frac norm_process(struct norm_t *, frac in);

#endif

