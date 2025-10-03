#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main (int argc, char *argv[])
{
  int free_mem = freemem ();
  printf ("Free memory: %d bytes\n", free_mem);
  printf ("Free memory: %d KB\n", free_mem / 1024);
  printf ("Free memory: %d pages\n", free_mem / 4096);
}