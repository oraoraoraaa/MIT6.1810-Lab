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
      fprintf (2, "Missing arguments.\n");
      exit (1);
    }

  /* Buffer to store number */
  char num_buf[16];
  memset (num_buf, 0, sizeof (num_buf));
  int num_index = 0;

  for (int i = 1; i < argc; i++)
    {
      /* Open input file */
      int fd = open (argv[i],
                     O_RDONLY); /* argv[1] is a pointer to the file name */
      if (fd < 0)               /* open() returns -1 if failed */
        {
          fprintf (2, "Error opening file.\n");
          exit (1);
        }
      char currentCharacter = 0;
      while (read (fd, &currentCharacter, 1) == 1)
        {
          /* If encounter separator */
          if (strchr (" -\r\t\n./,", currentCharacter))
            {
              /* If buffer holds numeric contents */
              if (num_index > 0)
                {
                  int num = atoi (num_buf);
                  if (num % 5 == 0 || num % 6 == 0)
                    {
                      fprintf (1, "%d\n", atoi (num_buf));
                    }
                  /* Reset buffer and index */
                  memset (num_buf, 0, sizeof (num_buf));
                  num_index = 0;
                }
            }
          /* If is a number */
          else
            {
              if (num_index
                  < sizeof (num_buf) - 1) /* If buffer space enough */
                {
                  num_buf[num_index] = currentCharacter;
                  num_index++;
                }
              else
                {
                  fprintf (2, "Number too large, buffer out of space!\n");
                  exit (1);
                }
            }
        }
      /* Check the remaining buffer */
      /* The file may not end with separator */
      if (num_index > 0)
        {
          int num = atoi (num_buf);
          if (num % 5 == 0 || num % 6 == 0)
            {
              fprintf (1, "%d\n", atoi (num_buf));
            }
        }

      close (fd);
    }

  exit (0);
}
