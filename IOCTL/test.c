#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "ioctl_test.h"

int main() {
	int answer;
	struct mystruct test = {4, "Khushal"};
	int dev = open("/dev/dummy", O_WRONLY);
	if(dev == -1) {
		printf("Opening was not possible!\n");
		return -1;
	}

	ioctl(dev, RD_VALUE, &answer);
	printf("The answer is %d\n", answer);	// ans = 100

	answer = 123; 

	ioctl(dev, WR_VALUE, &answer); // 123
	ioctl(dev, RD_VALUE, &answer);  // 123:wq
	printf("The answer is  now %d\n", answer);

	printf("Opening was successfull!\n");
	close(dev);
	return 0;
}

