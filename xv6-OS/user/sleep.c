/* Lab Utilities -- Sleep */
#include "kernel/types.h"

#include "kernel/stat.h"
#include "user/user.h"

int
main (int argc, char *argv[]) /* argv[] stores pointers! */
{
  if (argv[1] == 0)
    {
      fprintf (2, "Missing arguments.\n");
      exit (1);
    }

  int n = atoi (argv[1]);
  pause (n);
  exit (0);
};