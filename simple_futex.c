#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <assert.h>
#include <linux/futex.h>
#include <sys/syscall.h>





struct simplefu_semaphore {
  int avail;
  int waiters;
};

typedef struct simplefu_semaphore *simplefu;


void simplefu_wait(simplefu who)
{
  int val=0;

  syscall(__NR_futex, &who->avail, FUTEX_WAIT, val, NULL, 0, 0);

}

void simplefu_wake(simplefu who)
{
   syscall(__NR_futex, &who->avail, FUTEX_WAKE, 3, NULL, 0, 0);
}


void simplefu_req(simplefu who, int val)
{
   syscall(__NR_futex, &who->avail, FUTEX_REQUEUE, val, NULL, &who->avail, 0);
}



int main()
{
  int pid;
  int lockfile = open("ipc_lock", O_RDWR);
  assert(lockfile != -1);

  simplefu sema = mmap(NULL, sizeof(*sema), PROT_READ|PROT_WRITE, MAP_SHARED, lockfile, 0);
  assert(sema != MAP_FAILED);

  printf("avail %x\n", sema->avail);

  sema->avail=0;
  
  pid = fork();
  assert(pid != -1);

  if( pid == 0 ) { // child
    simplefu_wait(sema);
    printf("child %d done\n", getpid());
    exit(0);
  }

  pid = fork();
  assert(pid != -1);

  if( pid == 0 ) { // child
    simplefu_wait(sema);
    printf("child %d done\n", getpid());
    exit(0);
  }

  pid = fork();
  assert(pid != -1);

  if( pid == 0 ) { // child
    simplefu_wait(sema);
    printf("child %d done\n", getpid());
    exit(0);
  }


  printf("Waiting for childs...\n");
  getchar();
  simplefu_req(sema, 1);

  getchar();
  simplefu_req(sema, 1);

  getchar();
  simplefu_req(sema, 1);

  getchar();
  simplefu_req(sema, 1);

  printf("done initializing\n");
  
  return 0;
}
