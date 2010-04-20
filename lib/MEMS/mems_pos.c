#include <math.h>

#include "mems_pos.h"

// TODO: math to FP

/*
 * calculates modulo of the given angle
 * args:
 * angle - angle to normalize
 *
 * ret:
 * normalized angle \in [0; 2*PI]
 */
frac angle_normalize(frac angle) {
	if ((angle < 0) || (angle > M_2_PI))
		angle = fabs(M_2_PI - fabs(angle));
	return angle;
}

/* initializes the position_t struct
 * args:
 * *pos	- pointer to the position_t structure
 * idt  - time step of the measurements
 */
void position_init(struct position_t *pos, frac idt) {
	pos->dt = idt;

	pos->omega = mems_ZERO;
	pos->angle = mems_ZERO;

	pos->x = mems_ZERO;
	pos->y = mems_ZERO;

	pos->vx = mems_ZERO;
	pos->vy = mems_ZERO;

	pos->ax = mems_ZERO;
	pos->ay = mems_ZERO;
}

void position_process_measurement(struct position_t *pos, struct mems_sensors_t *sens) {
	position_process_angle(pos, sens->gyro.rate);
	position_process_accel(pos, sens->accel.data.coef[0], sens->accel.data.coef[1]);
}

/*
 * inserts the angle velocity into the structure, 
 * and calculates the current angle
 * args:
 * *pos  - pointer to the position_t structure
 * omega - angle velocity
 */
void position_process_angle(struct position_t *pos, frac omega) {
	pos->omega = omega;
	pos->angle = mems_add(pos->angle, mems_mul(pos->omega, pos->dt));
}

/* inserts the acceleration data into the structure, 
 * and calculates current velocity
 * args:
 * *pos - pointer to the position_t structure
 * iax  - x-axis accelerometer measurement
 * iay  - y-axis accelerometer measurement
 */
void position_process_accel(struct position_t *pos, frac iax, frac iay) {
	pos->ax = cos(pos->angle) * iax + sin(pos->angle) * iay;
	pos->ay = cos(pos->angle) * iay - sin(pos->angle) * iax;

	pos->vx = mems_add(pos->vx, mems_mul(pos->ax, pos->dt));
	pos->vy = mems_add(pos->vy, mems_mul(pos->ay, pos->dt));
}

/* inserts speed measurement into the structure
 * args:
 * *pos - pointer to the position_t structure
 * ivx  - x-axis speed
 * ivy  - y-axis speed
 */
void position_process_speed(struct position_t *pos, frac ivx, frac ivy) {
	pos->vx = ivx;
	pos->vy = ivy;
}

/* updates the position from previously inserted measurements
 * args:
 * *pos - pointer to the position_t structure
 */
void position_update(struct position_t *pos) {
	pos->x = mems_add(pos->x, mems_mul(pos->vx, pos->dt));
	pos->y = mems_add(pos->y, mems_mul(pos->vy, pos->dt));
}

/* prints the current state of the position
 * args:
 * *pos - pointer to the position_t structure
 */
void position_print(struct position_t *pos) {
//	printf("x,y,angle = %.2f %.2f %.2f vx,vy = %f %f  ax,ay = %f %f dt = %f", 
//		pos->x, pos->y, pos->angle, pos->vx, pos->vy, pos->ax, pos->ay, pos->dt);
}

