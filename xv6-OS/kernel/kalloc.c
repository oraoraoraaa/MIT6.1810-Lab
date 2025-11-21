// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange (void *pa_start, void *pa_end);
void freerange_super (void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run
{
  struct run *next;
};

struct run_super
{
  struct run_super *next;
};

struct
{
  struct spinlock lock;
  struct run *freelist;
} kmem;

struct
{
  struct spinlock lock;
  struct run_super *freelist;
} kmem_super;

void
kinit ()
{
  initlock (&kmem.lock, "kmem");
  initlock (&kmem_super.lock, "kmem_super");
  freerange (end, (void *)SUPERSTART);
  freerange_super ((void *)SUPERSTART, (void *)PHYSTOP);
}

void
freerange (void *pa_start, void *pa_end)
{
  char *p;
  p = (char *)PGROUNDUP ((uint64)pa_start);
  for (; p + PGSIZE <= (char *)pa_end; p += PGSIZE)
    kfree (p);
}

void
freerange_super (void *pa_start, void *pa_end)
{
  char *p;
  int debug_counter = 0;
  p = (char *)SUPERPGROUNDUP ((uint64)pa_start);
  for (; p + SUPERPGSIZE <= (char *)pa_end; p += SUPERPGSIZE)
    {
      kfree_super (p);
      debug_counter++;
    }
  printf ("DEBUG: kalloc.c:freerange_super(): %d superpage(s) from pa %p to "
          "%p successfully added to "
          "freelist.\n",
          debug_counter, pa_start, pa_end);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree (void *pa)
{
  struct run *r;

  if (((uint64)pa % PGSIZE) != 0 || (char *)pa < end
      || (uint64)pa >= SUPERSTART)
    panic ("kfree");

  // Fill with junk to catch dangling refs.
  memset (pa, 1, PGSIZE);

  r = (struct run *)pa;

  acquire (&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release (&kmem.lock);
}

void
kfree_super (void *pa)
{
  struct run_super *rs;

  if (((uint64)pa % SUPERPGSIZE) != 0 || (uint64)pa < SUPERSTART
      || (uint64)pa >= PHYSTOP)
    panic ("kfree");

  // Fill with junk to catch dangling refs.
  memset (pa, 2, SUPERPGSIZE);

  rs = (struct run_super *)pa;

  acquire (&kmem_super.lock);
  rs->next = kmem_super.freelist;
  kmem_super.freelist = rs;
  release (&kmem_super.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc (void)
{
  struct run *r;

  acquire (&kmem.lock);
  r = kmem.freelist;
  if (r)
    kmem.freelist = r->next;
  release (&kmem.lock);

  if (r)
    memset ((char *)r, 5, PGSIZE); // fill with junk
  return (void *)r;
}

// Allocate one 2-MB page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc_super (void)
{
  struct run_super *rs;

  acquire (&kmem_super.lock);
  rs = kmem_super.freelist;
  if (rs)
    kmem_super.freelist = rs->next;
  release (&kmem_super.lock);

  if (rs)
    memset ((char *)rs, 6, SUPERPGSIZE);
  return (void *)rs;
}