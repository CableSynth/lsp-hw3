/* Author:  Keith Shomper
 * Date:    1 Nov 2017
 * Purpose: Rudimentary testing for the key vault implementation
 *          that is embedded in a kernel module.
 */

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>

/*
 * Ioctl definitions
 */

/* Use 'k' as magic number - please use a different 8-bit number in your code */
#define HW4MOD_IOC_MAGIC  'k'
#define HW4MOD_IOCSKEY     _IOW (HW4MOD_IOC_MAGIC,   1, char)

int main (int argc, char **argv) {
	int  fd;
	int  rc;

	if (argc != 2) {
		fprintf(stderr, "Usage:  %s \"hint-password value\"\n", argv[0]);
		return 1;
	}

   if ((fd = open ("/dev/hw4mod", O_WRONLY)) == -1) {
     perror("opening file");
     return -1;
   }
	
	printf("Setting seek_key as %s\n", argv[1]);
	ioctl(fd, HW4MOD_IOCSKEY, argv[1]);
	rc = lseek(fd, 0, 0);
	printf("lseek return code is %d\n", rc);

	if (rc) {
		write(fd, "", 1);
	} else {
		fprintf(stderr, "seek failed\n");
	}

   close(fd);

   return 0;
}
