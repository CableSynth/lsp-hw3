
/* Author:  Keith Shomper
 * Date:    1 Nov 2017
 * Purpose: Rudimentary testing for the password vault implementation
 *          that is embedded in a kernel module.
 */

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include "pwd_vault.h"

#define  MAX_USERS 20
#define  BUF_SIZE  80

int main () {
	char buf[BUF_SIZE];
   char hint[MAX_HINT_SIZE] = "";
   char pwd[MAX_PWD_SIZE];
	int  fd;
	int  count = BUF_SIZE;

   if ((fd = open ("/dev/hw4mod", O_WRONLY)) == -1) {
     perror("opening file");
     return -1;
   }

   while (strcmp(hint, "q") != 0) {
      
      printf("Enter hint-password pair:  ");
      scanf("%s %s", hint, pwd);

		sprintf(buf, "%s %s", hint, pwd);

		if (strcmp(hint, "q") == 0) continue;

		int rc = write(fd, buf, count);
   }

   close(fd);

   return 0;
}
