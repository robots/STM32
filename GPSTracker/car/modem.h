#ifndef MODEM_h_
#define MODEM_h_

enum {
	MDM_STATE_CMD,
	MDM_STATE_ANALYZER,
	MDM_STATE_COMM,
	MDM_STATE_SNIFF
};

extern uint16_t a_freq;
extern uint16_t a_delta;
extern uint16_t a_start;
extern uint16_t a_stop;
extern uint16_t timer;

extern uint16_t Mdm_SniffTimeout;

void Mdm_ProcessCmd(uint8_t *, uint8_t);
void Mdm_Worker();
void Mdm_RfmTxDoneCb();

#endif

