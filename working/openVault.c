/* Author:  Keith Shomper
 * Date:    1 Nov 2017
 * Purpose: Rudimentary testing for the password vault implementation
 *          that is embedded in a kernel module.
 */

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

int main() {
  int fd, result;
  char buf[20];

  if ((fd = open ("/dev/hw4mod", O_WRONLY)) == -1) {
    perror("opening file");
    return -1;
  }

  close(fd);

  return 0;
}

