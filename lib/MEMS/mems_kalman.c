
#include "mems_kalman.h"

/** Initializes Kalman filter internal structure
 *
 * @param kf kalman_t structure pointer
 * @param Q process noise covariance
 * @param R measurement noise covariance
 */
void kalman_init(struct kalman_t *kf, frac Q, frac R) {
	kf->Xhat = mems_ZERO;
	kf->P = mems_ZERO;

	kf->Q = Q;
	kf->R = R;
}

/** Processes measurement, and predicts true state of aparatus
 * predicted result is available as "kf->Xhat"
 *
 * @param kf initialized kalman_t structure
 * @param z measured value
 */
frac kalman_process(struct kalman_t *kf, frac z) {
	frac Xhat_last;
	frac Plast;
	frac K;
	frac Inn;

	Xhat_last = kf->Xhat;
	Plast = mems_add(kf->P, kf->Q);

	/* UPDATE */
	/* calculate Kalman gain */
	K = mems_div(Plast, mems_add(Plast, kf->R)); 


	/* calculate inovation vector */
	Inn = mems_sub(z, Xhat_last);

	/* update estimate of state */
	kf->Xhat = mems_add(Xhat_last, mems_mul(K, Inn));

	/* update estimate error covariance */
	kf->P = mems_mul(mems_sub(mems_ONE, K), Plast);

	return kf->Xhat;
}

