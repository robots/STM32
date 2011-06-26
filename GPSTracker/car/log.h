#ifndef LOG_h_
#define LOG_h_

enum log_state {
	LOG_STOP,
	LOG_RUNNING,
};

struct pos_delta {
	uint16_t dlat;
	uint16_t dlon;
	uint8_t dalt;
	uint8_t dspeed;
};

struct rec_full_t {
	uint8_t id;
	uint8_t chksum;
	uint32_t lat;
	uint32_t lon;
	uint16_t alt;
	uint16_t speed;
	uint16_t heading;
};

struct rec_delta_t {
	uint8_t id;
	uint8_t chksum;
	struct pos_delta[4];
};

void Log_Init();
void Log_CheckFlash();
void Log_Start();
void Log_Stop();
void Log_WriteError();
void Log_Timer();


#endif

