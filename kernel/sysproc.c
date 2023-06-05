#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "date.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
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
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  
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


  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
pte_t * walk(pagetable_t pagetable, uint64 va, int alloc);
int
sys_pgaccess(void)
{
  uint64 check_addr;
  int page_num;
  uint64 uesr_bitmap_addr;
  if(argaddr(0, &check_addr) < 0)
    return -1;
  if(argint(1, &page_num) < 0)
    return -1;
  if (page_num > 32) return -1;
  if(argaddr(2, &uesr_bitmap_addr) < 0)
    return -1;
  
  uint32 bitmap = 0;
  for (int i = 0; i < page_num; ++i) {
    pte_t* pte_ptr = walk(myproc()->pagetable, check_addr+i*PGSIZE, 0);
    if (pte_ptr != 0 &&  (*pte_ptr & PTE_A)) {
      *pte_ptr = (*pte_ptr & (~PTE_A)); // ? why
      bitmap |= (1L << i);
    }
  }
  copyout(myproc()->pagetable, uesr_bitmap_addr, (char*)&bitmap, sizeof(bitmap));
  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
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

