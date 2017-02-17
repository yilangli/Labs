#include <stdio.h>
#include <fcntl.h>

int main(){
	int fd;
	fd= open("/dev/kyouko3",O_RDWR);
	printf("main start");
	getchar();
	close(fd);

	return 0;
}
