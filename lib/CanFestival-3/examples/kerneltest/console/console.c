#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include "console.h"


void showhelp(void) {
	printf("\nCanFestival kernel test example console\n\n");

	printf("start - start example\n");
	printf("stop  - stop example\n");
	printf("quit  - quit console\n");
	printf("\n");
}

int main(int argc,char *argv[])
{
	int canf_ktest_dev, cmd;
	char command[10];
	char device_path[20] = "/dev/";

	// create absolute path name for device
	strcat (device_path, DEVICE_NAME);

	canf_ktest_dev = open (device_path, O_WRONLY);
	
	if (canf_ktest_dev == -1) {
		perror ("Opening device");
		return 1;
	}

	showhelp();
    
	while (1) {
		printf("> ");
		scanf ("%s", &command);

		if (strcmp(command,"start") == 0)
			cmd = CMD_START;
		
		else if (strcmp(command,"stop") == 0)
			cmd = CMD_STOP;
		
		else if (strcmp(command,"quit") == 0)
			break;
		
		else {
			printf("Bad command\n");
			continue;
		}
		
		write(canf_ktest_dev, &cmd, sizeof(cmd));
	}
    
	close(canf_ktest_dev);
	return 0;
}
