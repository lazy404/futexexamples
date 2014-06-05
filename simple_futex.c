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

#define FUTEX_WAIT              0
#define FUTEX_WAKE              1
#define FUTEX_FD                2
#define FUTEX_REQUEUE           3
#define FUTEX_CMP_REQUEUE       4
#define FUTEX_WAKE_OP           5
#define FUTEX_LOCK_PI           6
#define FUTEX_UNLOCK_PI         7
#define FUTEX_TRYLOCK_PI        8
#define FUTEX_WAIT_BITSET       9
#define FUTEX_WAKE_BITSET       10
#define FUTEX_WAIT_REQUEUE_PI   11
#define FUTEX_CMP_REQUEUE_PI    12



void simplefu_wait(simplefu who)
{
  int val=0;
  syscall(__NR_futex, &who->avail, FUTEX_WAIT, val, NULL, 0, 0);
}

int cmp=0;

void simplefu_req(simplefu who)
{
//  futex(int *uaddr, int op, int val, *timeout, int *uaddr2, int val3);
  syscall(__NR_futex, &who->avail, FUTEX_CMP_REQUEUE_PI, 1, NULL, &who->avail, 0);
}


void wake(simplefu who)
{
   syscall(__NR_futex, &who->avail, FUTEX_WAKE, 3, NULL, 0, 0);
}


int main()
{
  int pid;
  int lockfile = open("ipc_lock", O_RDWR);
  assert(lockfile != -1);

  simplefu sema = mmap(NULL, sizeof(*sema), PROT_READ|PROT_WRITE, MAP_SHARED, lockfile, 0);
  assert(sema != MAP_FAILED);

  sema->avail=0;
  printf("avail2 %x\n", sema->avail);
  
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

  sleep(1);
  printf("Waiting for childs...\n");
  //getchar();

  simplefu_req(sema);

  wake(sema);
  printf("done initializing\n");
  
  return 0;
}
