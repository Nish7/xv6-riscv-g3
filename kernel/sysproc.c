#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

extern struct proc proc[NPROC];

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// report process status
// list all porcess
uint64
sys_ps(void)
{
  static char *states[] = {
      [UNUSED] = "unused",
      [USED] = "used",
      [SLEEPING] = "sleep",
      [RUNNABLE] = "runble",
      [RUNNING] = "running",
      [ZOMBIE] = "zombie"};

  uint64 dst;
  int max;
  argaddr(0, &dst); // sys_call arg; the user-space pointer
  argint(1, &max);  // sys_call arg; capacity of the array

  struct proc *p;
  struct uproc u;
  int count = 0;

  for (p = proc; p < &proc[NPROC] && count < max; p++)
  {
    acquire(&p->lock);
    if (p->state == UNUSED)
    {
      release(&p->lock);
      continue;
    }

    if (p->state >= 0 && p->state < NELEM(states) && states[p->state])
    {
      u.pid = p->pid;
      u.state = p->state;
      safestrcpy(u.name, p->name, sizeof(u.name));
      if (copyout(myproc()->pagetable, dst + count * sizeof(u), (char *)&u, sizeof(u)) < 0)
      {
        release(&p->lock);
        return -1;
      }
      count++;
    }

    release(&p->lock);
  }

  return count;
}
