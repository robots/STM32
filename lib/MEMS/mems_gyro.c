#include "mems_gyro.h"

void gyro_init(struct gyro_t *g) {

	kalman_init(&g->filter, 0.5*0.05, 0.1);
	g->norm.coef_num = 2;
	// TODO: move from here 
	g->norm.coef[0] = 0.07326; // Hw dependent
	g->norm.coef[1] = mems_ZERO;

	g->rate = 0;
	g->angle = 0;
}

void gyro_process(struct gyro_t *out, struct gyro_raw_t *in) {
	frac tmp_rate;

	// 12bit 2s complement 
	tmp_rate = (in->rate>0x2000)?in->rate-0x4000:in->rate;

	out->rate = norm_process(&out->norm, tmp_rate);
	out->rate = kalman_process(&out->filter, out->rate);

	out->angle = in->angle * 0.03663; // HW dependent (testing only)
}
