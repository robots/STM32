#ifndef DTMF_DET_h_
#define DTMF_DET_h_


struct dtmf_det_t {
	int32_t prev[2];
	int32_t coeff;
};

void dtmf_det_init();
void dtmf_det_deinit(void);
uint8_t dtmf_det_get(void);

#endif
