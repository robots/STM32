#ifndef MEMS_KALMAN_h_
#define MEMS_KALMAN_h_

#include "mems_math.h"

struct kalman_t {
	/* estimated output */
	frac Xhat;
	/* estimated error covariance */
	frac P;
	/* process noise covariance*/
	frac Q;
	/* measurement noise covariance */
	frac R;

};

void kalman_init(struct kalman_t*, frac Q, frac R);
frac kalman_process(struct kalman_t *kf, frac z); 

#endif

