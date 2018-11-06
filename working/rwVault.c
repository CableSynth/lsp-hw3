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
#include "pwd_vault.h"

#define  BUF_SIZE  80

#define HW4MOD_IOC_MAGIC  'k'
#define HW4MOD_IOCSKEY     _IOW (HW4MOD_IOC_MAGIC,   1, char)

int main () {
	char buf[BUF_SIZE];
   char hint[MAX_HINT_SIZE] = "";
   char pwd[MAX_PWD_SIZE];
	int  fd;
	int  count = BUF_SIZE;

   if ((fd = open ("/dev/hw4mod", O_RDWR)) == -1) {
     perror("opening file");
     return -1;
   }

	printf("Printing passwords in vault:\n");
	int i  = 1;
	int rc;
	while (rc = read(fd, buf, count)) {
		if (rc > 0) {
			printf("Password %2d:  %s\n", i, buf);
			if (i == 1) {
				sscanf(buf, "%s %s", hint, pwd);
			}
			i++;
		} else {
			perror("read");
			break;
		}
	}

	/* seek to first password */
	sprintf(buf, "%s %s", hint, pwd);
	ioctl(fd, HW4MOD_IOCSKEY, buf);
	rc = lseek(fd, 0, 0);
	if (rc) fprintf(stderr, "Resetting to password '%s'\n", buf);

	/* delete odd passwords, print even passwords */
	i  = 1;
	while (rc = read(fd, buf, count)) {
		if (rc > 0) {
			if (i % 2) {
				ioctl(fd, HW4MOD_IOCSKEY, buf);
				rc = lseek(fd, 0, 0);
				if (rc) write(fd, "", 1);
			} else {
				printf("Key %2d:  %s\n", i, buf);
			}
			i++;
		} else {
			perror("read");
			break;
		}
	}

   close(fd);

   return 0;
}
