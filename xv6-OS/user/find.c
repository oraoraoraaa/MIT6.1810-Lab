#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

char *fmtname (char *path);
void find (char *path, char *name);

int
main (int argc, char *argv[])
{
  find (argv[1], argv[2]);
  exit (0);
}

char *
fmtname (char *path)
{
  char *p = path + strlen (path);
  /* Find the first character after the last slash */
  for (; p >= path && *p != '/'; p--)
    ;
  p++;

  return p;
}

void
find (char *path, char *name)
{
  if (strlen (name) > DIRSIZ)
    {
      printf ("find: target name longer than file name limit.\n");
      exit (1);
    }

  char buf[512];
  char *p;
  int fd;
  struct dirent de;
  struct stat st;

  if ((fd = open (path, O_RDONLY)) < 0)
    {
      printf ("find: failed to open %s\n", path);
      return;
    }
  if (fstat (fd, &st) < 0)
    {
      printf ("find: failed to stat %s\n", path);
      return;
    }

  switch (st.type)
    {
    case T_DEVICE:
    case T_FILE:
      if (strcmp (fmtname (path), name) == 0)
        {
          printf ("%s\n", path);
        };
      break;

    case T_DIR:
      if (strlen (path) + 1 /* for '/' */ + DIRSIZ + 1 /* for '\0' */
          > sizeof (buf))
        {
          printf ("find: path too long: %s\n", path);
          return;
        }
      strcpy (buf, path);
      p = buf + strlen (buf);
      *p++ = '/';
      while (read (fd, &de, sizeof (de)) == sizeof (de))
        {
          if (de.inum == 0 || strcmp (de.name, ".") == 0
              || strcmp (de.name, "..") == 0)
            {
              continue;
            }

          memmove (p, de.name, DIRSIZ);
          p[DIRSIZ] = 0;
          find (buf, name);
        }
      break;
    default:
      printf ("find: failed to identify st.type in %s.\n", path);
      return;
      break;
    }

  close (fd);
}