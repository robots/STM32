#ifndef MEMS_GYRO_h_
#define MEMS_GYRO_h_

#include <stdint.h>

#include "mems_kalman.h"
#include "mems_norm.h"
#include "mems_vector.h"
#include "mems_quaternion.h"

struct gyro_raw_t {
	uint16_t rate;
	uint16_t angle;
	uint16_t temp;
};

struct gyro_t {
	frac rate;
	// measured angle by the device
	// for testing purpose only
	frac angle;
	
	struct norm_t norm;
	struct kalman_t filter;
};

void gyro_init(struct gyro_t *);
void gyro_process(struct gyro_t *out, struct gyro_raw_t *in);

#endif

