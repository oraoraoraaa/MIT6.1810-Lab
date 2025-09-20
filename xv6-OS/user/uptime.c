#include "kernel/types.h"
#include "user/user.h"

int
main ()
{
  int time = uptime ();
  printf ("%d\n", time);
  exit (0);
}