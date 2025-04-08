#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "elf.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"

static int loadseg(pde_t *, uint64, struct inode *, uint, uint);

int flags2perm(int flags)
{
    int perm = 0;
    if(flags & 0x1)
      perm = PTE_X;
    if(flags & 0x2)
      perm |= PTE_W;
    return perm;
}

int
exec(char *path, char **argv)
{
  char *s, *last;
  int i, off;
  uint64 argc, sz = 0, sp, ustack[MAXARG], stackbase;
  struct elfhdr elf;
  struct inode *ip;
  struct proghdr ph;
  pagetable_t pagetable = 0, oldpagetable;
  struct proc *p = myproc();

  begin_op();

  if((ip = namei(path)) == 0){
    end_op();
    return -1;
  }
  ilock(ip);

  // Check ELF header
  if(readi(ip, 0, (uint64)&elf, 0, sizeof(elf)) != sizeof(elf))
    goto bad;
  
  printf("Loading file: %s, inode number: %d\n", path, ip->inum);
  // printf("type: %d\n", elf.type);
  // printf("machine: %d\n", elf.machine);
  // printf("version: %d\n", elf.version);
  // printf("entry: %lx\n", elf.entry);
  // printf("phoff: %lx\n", elf.phoff);
  // printf("shoff: %lx\n", elf.shoff);
  // printf("flags: %d\n", elf.flags);
  // printf("ehsize: %d\n", elf.ehsize);
  // printf("phentsize: %d\n", elf.phentsize);
  printf("phnum: %d\n", elf.phnum);
  // printf("shentsize: %d\n", elf.shentsize);
  // printf("shnum: %d\n", elf.shnum);
  // printf("shstrndx: %d\n", elf.shstrndx);

  if(elf.magic != ELF_MAGIC)
    goto bad;

  if((pagetable = proc_pagetable(p)) == 0)
    goto bad;

  // Load program into memory.
  for(i=0, off=elf.phoff; i<elf.phnum; i++, off+=sizeof(ph)){
    if(readi(ip, 0, (uint64)&ph, off, sizeof(ph)) != sizeof(ph))
      goto bad;
    
    // printf("------\n");
    // printf("Vaddr: %lx\n", ph.vaddr);
    // printf("Type: %d\n", ph.type);
    // printf("Off: %lx\n", ph.off);
    // printf("Filesz: %ld\n", ph.filesz);
    // printf("Memsz: %ld\n", ph.memsz);
    // printf("Flags: %d\n", ph.flags);
    // printf("Align: %lx\n", ph.align);
    
    if(ph.type != ELF_PROG_LOAD)
      continue;
    if(ph.memsz < ph.filesz)
      goto bad;
    if(ph.vaddr + ph.memsz < ph.vaddr)
      goto bad;
    if(ph.vaddr % PGSIZE != 0)
      goto bad;
    
    uint64 new_sz = PGROUNDUP(ph.vaddr + ph.memsz);
    if (new_sz > sz){
      sz = new_sz;
    }
    
    if (loadseg(pagetable, ph.vaddr, ip, ph.off, ph.filesz) < 0) {
      goto bad;
    } 
    
    if (ph.memsz > 0 && ph.filesz == 0) {
      uint64 bss_start = ph.vaddr + ph.filesz; // Start of .bss
      uint64 bss_size = ph.memsz - ph.filesz;  // Size of .bss
      printf("Marking .bss from VA 0x%lx to 0x%lx for demand paging\n", bss_start, bss_start + bss_size);
  
      // Mark each page in the .bss range as demand-paged
      for (uint64 va = PGROUNDDOWN(bss_start); va < PGROUNDUP(bss_start + bss_size); va += PGSIZE) {
        if (mappages(pagetable, va, PGSIZE, 0, PTE_U | PTE_D) != 0) { // Mark as demand-paged, no physical page yet
          goto bad;
        }
      }
    }
  }

  iunlockput(ip);
  end_op();
  ip = 0;

  p = myproc();
  uint64 oldsz = p->sz;

  // Allocate some pages at the next page boundary.
  // Make the first inaccessible as a stack guard.
  // Use the rest as the user stack.
  sz = PGROUNDUP(sz);
  printf("\nSize: %ld\n", sz);
  uint64 sz1;
  if((sz1 = uvmalloc(pagetable, sz, sz + (USERSTACK+1)*PGSIZE, PTE_W)) == 0)
    goto bad;
  sz = sz1;
  printf("\nStack Size: %ld\n", sz);
  uvmclear(pagetable, sz-(USERSTACK+1)*PGSIZE);
  sp = sz;
  stackbase = sp - USERSTACK*PGSIZE;

  // Push argument strings, prepare rest of stack in ustack.
  for(argc = 0; argv[argc]; argc++) {
    if(argc >= MAXARG)
      goto bad;
    sp -= strlen(argv[argc]) + 1;
    sp -= sp % 16; // riscv sp must be 16-byte aligned
    if(sp < stackbase)
      goto bad;
    if(copyout(pagetable, sp, argv[argc], strlen(argv[argc]) + 1) < 0)
      goto bad;
    ustack[argc] = sp;
  }
  ustack[argc] = 0;

  // push the array of argv[] pointers.
  sp -= (argc+1) * sizeof(uint64);
  sp -= sp % 16;
  if(sp < stackbase)
    goto bad;
  if(copyout(pagetable, sp, (char *)ustack, (argc+1)*sizeof(uint64)) < 0)
    goto bad;

  // arguments to user main(argc, argv)
  // argc is returned via the system call return
  // value, which goes in a0.
  p->trapframe->a1 = sp;

  // Save program name for debugging.
  for(last=s=path; *s; s++)
    if(*s == '/')
      last = s+1;
  safestrcpy(p->name, last, sizeof(p->name));
    
  // Commit to the user image.
  oldpagetable = p->pagetable;
  p->pagetable = pagetable;
  p->sz = sz;
  p->trapframe->epc = elf.entry;  // initial program counter = main
  p->trapframe->sp = sp; // initial stack pointer
  proc_freepagetable(oldpagetable, oldsz);

  return argc; // this ends up in a0, the first argument to main(argc, argv)

 bad:
  if(pagetable)
    proc_freepagetable(pagetable, sz);
  if(ip){
    iunlockput(ip);
    end_op();
  }
  return -1;
}

// Load a program segment into pagetable at virtual address va.
// va must be page-aligned
// and the pages from va to va+sz must already be mapped.
// Returns 0 on success, -1 on failure.
static int
loadseg(pagetable_t pagetable, uint64 va, struct inode *ip, uint offset, uint sz)
{
  uint i, n;
  uint64 pa;

  for(i = 0; i < sz; i += PGSIZE){
    // Calculate how many bytes to load - might be less than PGSIZE at the end
    if(sz - i < PGSIZE)
      n = sz - i;
    else
      n = PGSIZE;

    printf("Loading page at VA 0x%lx from file offset 0x%x, size %d\n", va + i, offset + i, n);
   
    int is_essential = ((va + i) >= TEXTBASE && (va + i) < TEXTBASE + TEXTSIZE) || 
                       ((va + i) >= USTACKTOP - PGSIZE && (va + i) < USTACKTOP);

    if (is_essential) {
      printf("Loading Eagerly at VA 0x%lx\n", va + i);
      // Load eagerly (current behavior)
      pa = (uint64)kalloc();
      if (pa == 0)
        return -1;
      memset((void *)pa, 0, PGSIZE);
     
      if (readi(ip, 0, (uint64)pa, offset + i, n) != n) {
        kfree((void *)pa);
        return -1;
      }
      if (mappages(pagetable, va + i, PGSIZE, pa, PTE_R | PTE_X | PTE_U | PTE_W) != 0) {
        kfree((void *)pa);
        return -1;
      }
    } else {
      // For demand paging, we should store:
      // 1. File offset
      // 2. Size to read
      // 3. inode number (for reopening)
      
      // Create metadata: pack file info into 64 bits
      // Format: [inum(16bits)][size(16bits)][offset(32bits)]
      uint64 metadata = ((uint64)ip->inum << 48) | ((uint64)n << 32) | (offset + i);
      printf("Demand Paging with the virtual address: %lx and metadata: %lx\n", va + i, metadata);
      
      // Mark as demand-paged (use PTE_D flag but no PTE_V)
      if (mappages(pagetable, va + i, PGSIZE, metadata, PTE_U | PTE_D) != 0) {
        return -1;
      }
    }
  }
  
  return 0;
}
