#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define HW2_POP 0

extern int errno;

int main ()
{
	char *buffer;
	int data_size;
	int errnum;
	buffer = malloc(sizeof(char) * 50);
	int fd = open ("/dev/queue0", O_RDONLY); 
	data_size = ioctl (fd, HW2_POP, buffer); 
	// error handling
	if(data_size < 0){
		errnum = errno;
        fprintf(stderr, "Error calling ioctl: %s\n", strerror( errnum ));
        return 0;
	}
	printf("Popped data is : %s \n", buffer);
	printf("Popped data size is : %d \n", data_size);
	close (fd);
	free(buffer);
	return 0;
}
