#ifndef NMEA_h_
#define NMEA_h_

struct gpsdata_t {
	// time
	uint8_t hour;
	uint8_t min;
	uint8_t sec;

	// date
	uint8_t day;
	uint8_t month;
	uint8_t year;

	// position
	uint8_t valid;
	int32_t lat;
	int32_t lon;
	uint16_t alt;

	uint16_t heading;
	uint16_t speed;

	uint8_t sats;
	uint8_t hdop;
};

extern gpsdata_t gpsdata;

void NMEA_Parse(uint8_t *buf, uint8_t len);

#endif

