/*
 * DTMF detector based on Goertzel algorithm
 * with Q(15) arithmetics
 *
 * TODO:
 *  - tune detector thresholds
 *  - single freq detection (?)
 *  - callback on valid detection
 */
#include "stm32f10x.h"

#include "ad.h"

#include "dtmf_det.h"

uint8_t dtmf_det_output = 0;
uint8_t dtmf_det_updated = 0;

struct dtmf_det_t dtmf_detector[8];
uint16_t dtmf_det_N = 0;


static void dtmf_det_process(int16_t sample);


void dtmf_det_init()
{
	int i;

	dtmf_detector[0].coeff = 0x6d02;
	dtmf_detector[1].coeff = 0x68b1;
	dtmf_detector[2].coeff = 0x63fc;
	dtmf_detector[3].coeff = 0x5ee7;
	dtmf_detector[4].coeff = 0x4a70;
	dtmf_detector[5].coeff = 0x4090;
	dtmf_detector[6].coeff = 0x3290;
	dtmf_detector[7].coeff = 0x23ce;

	for (i = 0; i < 8; i++) {
		dtmf_detector[i].prev[0] = 0;
		dtmf_detector[i].prev[1] = 0;
	}

	dtmf_det_N = 0;

	AD_Init(dtmf_det_process);
	AD_SetTimer(100, 90);
	AD_Start();
}

void dtmf_det_deinit(void)
{
	AD_Stop();
}

uint8_t dtmf_det_get(void)
{
	uint8_t out = 0;

	if (dtmf_det_updated == 1) {
		out = dtmf_det_output;
		dtmf_det_updated = 0;
	}

	return out;
}

static int8_t find_max(int32_t *s)
{
	int8_t i = 0;
	int8_t idx = -1;
	int32_t max = 0;

	for (i = 0; i < 4; i++) {
		if (max < s[i]) {
			max = s[i];
			idx = i;
		}
	}

	if (idx < 0)
		return -1;

	max /= 5;

	for (i = 0; i < 4; i++) {
		if (idx != i && s[i] > max) {
			return -1;
		}
	}

	return idx;	
}

static void dtmf_det_process(int16_t sample)
{
	struct dtmf_det_t *d;
	int i;

	for (i = 0; i < 8; i++) {
		d = &dtmf_detector[i];

		int32_t s = sample + ((d->coeff * d->prev[0]) >> 14) - d->prev[1];
		d->prev[1] = d->prev[0];
		d->prev[0] = s;
	}


	dtmf_det_N++;
	if (dtmf_det_N >= 205) {
		int8_t high, low;
		int32_t e[8];
		int32_t tot = 0;

		dtmf_det_N = 0;

		// calculate energies
		for (i = 0; i < 8; i++) {
			d = &dtmf_detector[i];

			e[i] = d->prev[1] * d->prev[1] + d->prev[0] * d->prev[0] - ((d->coeff * d->prev[0] * d->prev[1]) >> 14);
			e[i] >>= 5;
			tot += e[i];

			d->prev[0] = 0;
			d->prev[1] = 0;
		}

		if (tot < 1000000)
			return;

		low = find_max(&e[0]);
		high = find_max(&e[4]);

		if (low < 0 || high < 0)
			return;

		if (tot * 4 > (e[low] + e[high]) * 10)
			return;

		dtmf_det_output = (low+1) | ((high+1) << 4);
		dtmf_det_updated = 1;

		// TODO: callback
		dtmf_set(dtmf_det_output);
	}
}

