
/* Author:  Keith Shomper
 * Date:    1 Nov 2017
 * Purpose: Rudimentary testing for the password vault implementation
 *          that is embedded in a kernel module.
 */

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

int main () {
	char buf[2];
	int  fd;

   if ((fd = open ("/dev/hw4mod", O_WRONLY)) == -1) {
     perror("opening file");
     return -1;
   }
	
	strncpy(buf, "", 1);
	write(fd, buf, 1);

   close(fd);

   return 0;
}
