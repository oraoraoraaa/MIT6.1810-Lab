#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

int
main (int argc, char *argv[])
{
  /* Argument check */
  if (argc == 1)
    {
      fprintf (2, "Usage: sixfive <file1> [file2] ...\n");
      exit (1);
    }

  for (int i = 1; i < argc; i++)
    {
      /* Buffer and index must be reset for each file */
      char num_buf[16];
      memset (num_buf, 0, sizeof (num_buf));
      int num_index = 0;
      int errorflag = 0;

      /* Open input file */
      int fd = open (argv[i], O_RDONLY);
      if (fd < 0)
        {
          fprintf (2, "sixfive: cannot open %s\n", argv[i]);
          exit (1);
        }

      char currentCharacter;
      while (read (fd, &currentCharacter, 1) == 1)
        {
          /* If encounter separator */
          if (strchr (" -\r\t\n./,", currentCharacter))
            {
              if (num_index > 0)
                {
                  int num = atoi (num_buf);
                  if (num % 5 == 0 || num % 6 == 0)
                    {
                      fprintf (1, "%d\n", num); // Use the variable
                    }
                  /* Reset buffer and index */
                  memset (num_buf, 0, sizeof (num_buf));
                  num_index = 0;
                }
            }
          /* If is a digit */
          else
            {
              if (num_index < sizeof (num_buf) - 1)
                {
                  num_buf[num_index++] = currentCharacter;
                }
              else
                {
                  /* Error and continue to next file (next for loop) */
                  fprintf (2, "Number too large in file %s\n", argv[i]);
                  errorflag = 1;
                  break;
                }
            }
        }

      /* Check the remaining buffer for the current file */
      if (!errorflag && num_index > 0)
        {
          int num = atoi (num_buf);
          if (num % 5 == 0 || num % 6 == 0)
            {
              fprintf (1, "%d\n", num);
            }
        }

      close (fd);
    }

  exit (0);
}
