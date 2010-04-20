#ifndef MEMS_ACCEL_h_
#define MEMS_ACCEL_h_

#include <stdint.h>

#include "mems_kalman.h"
#include "mems_norm.h"
#include "mems_vector.h"
#include "mems_quaternion.h"

enum {
	axis_X,
	axis_Y,
	axis_Z
};

struct accel_raw_t {
	uint16_t data[3];
};

struct accel_t {
	struct vector_t data;
	
	struct norm_t norm[3];
	struct kalman_t filter[3];
	struct matrix_t align;
};

void accel_init(struct accel_t *);
void accel_process(struct accel_t *out, struct accel_raw_t *in);
void accel_normalize(struct accel_t *, struct accel_raw_t *);
void accel_filter(struct accel_t *);
void accel_align(struct accel_t *);

#endif

