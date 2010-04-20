
#include "mems_accel.h"

void accel_init(struct accel_t *in) {
	int i;

	for (i=0; i<3; i++) {
		kalman_init(&in->filter[i], 0.001, 10);

		in->norm[i].coef_num = 2;
		in->norm[i].coef[0] = mems_ONE;
		in->norm[i].coef[1] = mems_ZERO;

		in->data.coef[i] = 0;
	}
}

void accel_process(struct accel_t *out, struct accel_raw_t *in) {
	struct vector_t vect_tmp;
	int i;

	for (i=0; i<3; i++) {
		vect_tmp.coef[i] = norm_process(&out->norm[i], in->data[i]);
		vect_tmp.coef[i] = kalman_process(&out->filter[i], vect_tmp.coef[i]);
	}
	// apply alignment
	matrix_mul(&out->data, &out->align, &vect_tmp);
}

void accel_normalize(struct accel_t *out, struct accel_raw_t *in) {
	int i;

	for (i=0; i<3; i++) {
		out->data.coef[i] = norm_process(&out->norm[i], in->data[i]);
	}
}

void accel_filter(struct accel_t *in) {
	int i;

	for (i=0; i<3; i++) {
		in->data.coef[i] = kalman_process(&in->filter[i], in->data.coef[i]);
	}
}

void accel_align(struct accel_t *in) {
	struct vector_t vect_tmp;
	int i;

	for (i=0; i<3; i++) {
		vect_tmp.coef[i] = in->data.coef[i];
	}
	// apply alignment
	matrix_mul(&in->data, &in->align, &vect_tmp);
}

