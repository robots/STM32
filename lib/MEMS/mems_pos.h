#ifndef MEMS_POSITION_h_
#define MEMS_POSITION_h_

#include "mems_math.h"
#include "mems_accel.h"
#include "mems_gyro.h"

struct mems_sensors_t {
	struct accel_t accel;
	struct gyro_t gyro;
};


struct position_t {
	frac dt;

	frac x;
	frac y;

	frac vx;
	frac vy;

	frac ax;
	frac ay;

	frac angle;
	frac omega;
	
};

frac angle_normalize(frac);

void position_init(struct position_t *pos, frac idt);

void position_process_measurement(struct position_t *pos, struct mems_sensors_t *sens);
void position_process_angle(struct position_t *pos, frac omega);
void position_process_accel(struct position_t *pos, frac iax, frac iay);
void position_process_speed(struct position_t *pos, frac ivx, frac ivy);

void position_update(struct position_t *pos);

void position_print(struct position_t *pos);

#endif

