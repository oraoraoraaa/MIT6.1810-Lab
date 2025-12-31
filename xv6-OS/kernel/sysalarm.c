#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"

uint64
sys_sigalarm (void)
{
  int tick;
  uint64 handler;
  struct proc *p = myproc ();

  argint (0, &tick);
  argaddr (1, &handler);

  if (tick == 0 && handler == 0)
    {
      // disable alarm
      p->alarm_tick = -1;
      p->alarm_handler = -1;
      p->alarm_tick_remaining = -1;
      return 0;
    }

  p->alarm_tick = tick;
  p->alarm_tick_remaining = tick;
  p->alarm_handler = handler;

  return 0;
}

uint64
sys_sigreturn (void)
{
  return 0;
}