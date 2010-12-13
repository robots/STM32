#include "platform.h"
#include "stm32f10x.h"

#include "nmea.h"

gpsdata_t gpsdata;

static uint32_t NMEA_atoi(char *p)
{
	uint32_t out = 0;

	while ((*p >= '0' && *p <= '9') || *p == '.')
	{
		if (*p == '.') {
			p++;
			continue;
		}
		out *= 10;
		out += *p - '0';
		p++;
	}

	return out;
}

void NMEA_Parse(uint8_t *buf, uint8_t len)
{
	uint8_t *p;
	uint32_t tmp;

	if(!strncmp(buf, "$GPGGA", 6))
	{	
		p = buf;
		p += 7;

		gpsdata.hour = (p[0] - '0') * 10 + p[1] - '0';
		gpsdata.min = (p[2] - '0') * 10 + p[3] - '0';
		gpsdata.sec = (p[4] - '0') * 10 + p[5] - '0';

/*		if (p[6] == '.')
			gpsdata.hsec = (p[7] - '0') * 10 + p[8] - '0';
		else
			gpsdata.hsec = 0;
*/
		p = strstr(p, ",") + 1;
		// parse lat
		tmp = NMEA_atoi(p);
		p = strstr(p, ",") + 1;
		if (p[0] == 'S')
			tmp = -tmp;
		gpsdata.lat = tmp;

		p = strstr(p, ",")+1;
		// parse lon
		tmp = NMEA_atoi(p);
		p=strstr(p, ",") + 1;
		if (p[0] == 'W')
			tmp = -tmp;
		gpsdata.lon = tmp;

		p = strstr(p, ",") + 1;
		gpsdata.valid = (p[0] - '0')?1:0;

		p = strstr(p, ",") + 1;
		gpsdata.sats = (p[0] - '0') * 10 + p[1] - '0';

		p = strstr(p, ",") + 1;
		// HDOP
		gpsdata.hdop = NMEA_atoi(p);

		p = strstr(p, ",") + 1;
		// alt
		gpsdata.alt = NEMA_atoi(p);
	} else if(!strncmp(buf, "$GPRMC", 6)) {	
		p = buf;
		p += 7;
		gpsdata.hour = (p[0] - '0') * 10 + p[1] - '0';
		gpsdata.min = (p[2] - '0') * 10 + p[3] - '0';
		gpsdata.sec = (p[4] - '0') * 10 + p[5] - '0';
		
//		if(p[6] == '.')
//			gpsdata.hsec = (p[7]-'0')*10 + p[8]-'0';
//		else
//			gpsdata.hsec=0;

		p = strstr(p, ",") + 1;
		gpsdata.valid = (p[0] == 'A')?1:0;

		p = strstr(p, ",") + 1;
		// parse lat
		tmp = NMEA_atoi(p);
		p = strstr(p, ",") + 1;
		
		if (p[0] == 'S')
			tmp = -tmp;

		gpsdata.lat = tmp;

		p = strstr(p, ",")+1;
		// parse lon
		tmp = NMEA_atoi(p);
		p=strstr(p, ",") + 1;
		if (p[0] == 'W')
			tmp = -tmp;
		gpsdata.lon = tmp;

		p = strstr(p, ",") + 1;
		// speed
		gpsdata.speed = NMEA_atoi(p);

		p = strstr(p, ",") + 1;
		// heading
		gpsdata.heading = NMEA_atio(p);

		p = strstr(p, ",") + 1;
		// date
		gpsdata.day = (p[0] - '0') * 10 + p[1] - '0';
		gpsdata.month = (p[2] - '0') * 10 + p[3] - '0';
		gpsdata.year = (p[4] - '0') * 10 + p[5] - '0';
	}

}
