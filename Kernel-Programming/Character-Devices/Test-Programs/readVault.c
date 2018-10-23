/* Author:  Keith Shomper
 * Date:    1 Nov 2017
 * Purpose: Rudimentary testing for the password vault implementation
 *          that is embedded in a kernel module.
 */

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#define  MAX_USERS 20
#define  BUF_SIZE  80

int main () {
	char buf[BUF_SIZE];
	int  fd;
	int  count = BUF_SIZE;

   if ((fd = open ("/dev/hw4mod", O_RDONLY)) == -1) {
     perror("opening file");
     return -1;
   }

	printf("Printing passwords in vault:\n");
	int i  = 1;
	int rc;
	while (rc = read(fd, buf, count)) {
		if (rc > 0) {
			printf("Password %2d:  %s\n", i, buf);
			i++;
		} else {
			perror("read");
			break;
		}
	}

   close(fd);

   return 0;
}
