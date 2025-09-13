#include "kernel/types.h"
#include "user/user.h"

void
pingpong ()
{
  int pipeParent[2];
  int pipeChild[2];
  pipe (pipeParent);
  pipe (pipeChild);

  if (fork () == 0)
    {
      /* Child process: Pong -- write to parent process */
      char bufChild[1];
      close (pipeParent[1]);
      close (pipeChild[0]);

      /* Wait for ping */
      if (read (pipeParent[0], bufChild, 1) != 1 || bufChild[0] != 'A')
        {
          fprintf (2, "Read from parent process error.\n");
          exit (1);
        }
      fprintf (1, "%d: received ping.\n", getpid ());

      /* Pong */
      write (pipeChild[1], "B", 1);
      exit (0);
    }
  else
    {
      /* Parent process: Ping -- write to child process */
      char bufParent[1];

      close (pipeParent[0]);
      close (pipeChild[1]);

      /* Ping */
      write (pipeParent[1], "A", 1);

      /* Waiting for pong */
      if (read (pipeChild[0], bufParent, 1) != 1 || bufParent[0] != 'B')
        {
          fprintf (2, "Read from child process error.\n");
          exit (1);
        }
      wait (0);
      fprintf (1, "%d: received pong.\n", getpid ());

      close (pipeParent[1]);
      close (pipeChild[0]);

      // exit (0);
      /* To run the code once and not trigger examination,
         uncomment the line. */
    }
}

/* A quiet version to avoid the effect of fprintf on speed */
void
pingpong_quiet ()
{
  int pipeParent[2];
  int pipeChild[2];
  pipe (pipeParent);
  pipe (pipeChild);

  if (fork () == 0)
    {
      /* Child process: Pong -- write to parent process */
      char bufChild[1];
      close (pipeParent[1]);
      close (pipeChild[0]);

      /* Wait for ping */
      if (read (pipeParent[0], bufChild, 1) != 1 || bufChild[0] != 'A')
        {
          fprintf (2, "Read from parent process error.\n");
          exit (1);
        }

      /* Pong */
      write (pipeChild[1], "B", 1);
      exit (0);
    }
  else
    {
      /* Parent process: Ping -- write to child process */
      char bufParent[1];

      close (pipeParent[0]);
      close (pipeChild[1]);

      /* Ping */
      write (pipeParent[1], "A", 1);

      /* Waiting for pong */
      if (read (pipeChild[0], bufParent, 1) != 1 || bufParent[0] != 'B')
        {
          fprintf (2, "Read from child process error.\n");
          exit (1);
        }
      wait (0);

      close (pipeParent[1]);
      close (pipeChild[0]);

      // exit (0);
      /* To run the code once and not trigger examination,
         uncomment the line. */
    }
}

void
examinePerformance ()
{
  fprintf (1, "VERBOSE EXAMINATION-------------\n\n");
  for (int i = 9; i > 0; i--)
    {
      fprintf (1, "Examining program starting in %d...\n", i);
      pause (10);
      if (i == 1)
        {
          fprintf (1, "Examining program starting NOW.\n");
          pause (10);
        }
    }
  int startTime = uptime ();
  for (int i = 0; i < 1000; i++)
    {
      pingpong ();
    }
  int endTime = uptime ();
  fprintf (1, "Exchange amount: 1000\nTime spent: %d\n", endTime - startTime);
  fprintf (1,
           "Exchange rate (rounded towards zero): %d exchanges per second\n\n",
           1000 * 10 / (endTime - startTime));
}

void
examinePerformance_quiet ()
{
  fprintf (1, "QUIET EXAMINATION-------------\n\n");
  for (int i = 9; i > 0; i--)
    {
      fprintf (1, "Examining program starting in %d...\n", i);
      pause (10);
      if (i == 1)
        {
          fprintf (1, "Examining program starting NOW.\n");
          pause (10);
        }
    }
  int startTime = uptime ();
  for (int i = 0; i < 1000; i++)
    {
      pingpong_quiet ();
    }
  int endTime = uptime ();
  fprintf (1, "Exchange amount: 1000\nTime spent: %d\n", endTime - startTime);
  fprintf (1,
           "Exchange rate (rounded towards zero): %d exchanges per second\n",
           1000 * 10 / (endTime - startTime));
}

int
main ()
{
  pingpong ();
  fprintf (1, "---------------------------------\nExamination is enabled!\n");
  fprintf (1, "View source code line 54 to see the disabling "
              "method.\n---------------------------------\n");
  pause (15);

  examinePerformance ();
  examinePerformance_quiet ();
  exit (0);
}