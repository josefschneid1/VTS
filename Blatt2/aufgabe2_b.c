/* C. Facchi 2.4.06 Example for Mutex; solution using IPC semaphore */

#include <stdio.h>
#include <stdlib.h>

# include <sys/types.h>
# include <sys/ipc.h>
# include <sys/sem.h>

#include <unistd.h>



#define NUMBER_OF_PROCESSES	5
#define NUMBER_OF_SEMAPHORES 5



key_t sem_key;
int sem_id;


void V(int sem_id,int num) {
  struct sembuf semaphore;

  semaphore.sem_num = num;

  semaphore.sem_op=1;

  semaphore.sem_flg=~(IPC_NOWAIT|SEM_UNDO);

  if(semop(sem_id,&semaphore,1)){
    perror("Error in semop V()");
    exit(1);
  }
}

void P(int sem_id, int num)
{
  struct sembuf semaphore;

  semaphore.sem_num = num;

  semaphore.sem_op=-1;

  semaphore.sem_flg=~(IPC_NOWAIT|SEM_UNDO);
   
  if(semop(sem_id,&semaphore,1)){
    perror("Error in semop P()");
    exit(1);
  }
}



void init_sem(int sem_id,int sem_num,int val) {
  union semnum {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
  }argument;
  argument.val=val;

  if(semctl(sem_id,sem_num,SETVAL,argument)<0){
    perror("Error in semctl");
    exit(1);
  }
}

void denken(long time, int num)
{
    printf("Philosoph %d beginnt zu denken!\n", num);
    sleep(time);
    printf("Philosoph %d hört auf zu denken!\n", num);
}

void essen(long time, int l , int r, int num)
{
       
    if(r < l)
    {
        P(sem_id, r);
        P(sem_id, l);
    }else
    {
        P(sem_id, l);
        P(sem_id, r);
    }
    printf("Philosoph %d hat Gabel %d und %d!\n", num, l,r);
    sleep(time);
    printf("Philosoph %d legt Gabel %d und %d zurück!\n", num, l,r);
    V(sem_id, r);
    V(sem_id, l);
}

void child_loop(int process){
#define NUM_CHILD_LOOPS 3

for(int i = 0; i < NUM_CHILD_LOOPS; ++i)
{
    int denkenTime = rand() % 5 + 1;
    int essenTime = rand() % 5 + 1;
    
    int lSemaphore = process;
    int rSemaphore = (process+4) % 5;
    
    denken(denkenTime, process);
    essen(essenTime, lSemaphore, rSemaphore, process);
}      
}


int main(){

  int process;		

  if ((sem_key = ftok (".",'1'))<0){
    perror("Error in ftok");
    exit(1);
  }


  if ((sem_id = semget( sem_key,NUMBER_OF_SEMAPHORES, IPC_CREAT|0666))<0){
    perror("Error in semget");
    exit(1);
  }

  for(int i = 0; i < NUMBER_OF_SEMAPHORES; ++i)
  {
    init_sem( sem_id, i, 1);
  }

  for(process=0; process< NUMBER_OF_PROCESSES; process++) {
    switch(fork()) {
      case -1:	perror("FORK failed");
    		break;
    
      case 0:	/*child*/
  		child_loop(process);
  		exit(0);
    
      default:	/*father*/
  				;
    }
  }
	
}

