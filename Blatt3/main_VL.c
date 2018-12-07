/* C. Facchi 2.4.06 Example for Mutex; solution using IPC semaphore */

#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <unistd.h>

#define NUM_WRITER 2
#define NUM_READER 5
#define NUM_LOOPS 3

#define WRITER 0
#define READER 1
#define MUTEX 2




key_t sem_key;
int sem_id;

void V(int sem_id,int sem_num) {
  struct sembuf semaphore;

  semaphore.sem_num=sem_num;
  semaphore.sem_op=1;
  semaphore.sem_flg=~(IPC_NOWAIT|SEM_UNDO);
  if(semop(sem_id,&semaphore,1)){
    perror("Error in semop V()");
    exit(1);
  }
}

void P(int sem_id,int sem_num) {
  struct sembuf semaphore;

  semaphore.sem_num=sem_num;
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






void reader(int num)
{
    for(int i = 0; i < NUM_LOOPS; ++i)
    {
        int counter;
        P(sem_id, MUTEX);
        counter = semctl(sem_id, READER, GETVAL);
        counter++;
        if(counter == 1)
        {
            P(sem_id, WRITER);
        }
        init_sem(sem_id, READER, counter);
        V(sem_id, MUTEX);

        printf("Kritischer Bereich von Reader %d, Beginn\n", num);
        printf("Kritischer Bereich von Reader %d, Ende\n", num);

        P(sem_id, MUTEX);
        counter = semctl(sem_id, READER, GETVAL);
        counter--;
        if(counter == 0)
        {
            V(sem_id, WRITER);
        }
        init_sem(sem_id, READER, counter);
        V(sem_id, MUTEX);
        sleep(1);
        printf("Unkritischer Bereich von Reader %d, Beginn\n", num);
        printf("Unkritischer Bereich von Reader %d, Ende\n", num);
        sleep(1);
    }
}

void writer(int num)
{
    for(int i = 0; i < NUM_LOOPS; ++i)
    {
        P(sem_id, WRITER);
        printf("Kritischer Bereich von Writer %d, Beginn\n", num);
        printf("Kritischer Bereich von Writer %d, Ende\n", num);
        V(sem_id, WRITER);
        sleep(1);
        printf("Unkritischer Bereich von Writer %d, Beginn\n", num);
        printf("Unkritischer Bereich von Writer %d, Ende\n", num);
        sleep(1);
    }     
}


int main(){

  int process;		

  if ((sem_key = ftok (".",'1'))<0){
    perror("Error in ftok");
    exit(1);
  }

  if ((sem_id = semget( sem_key,3, IPC_CREAT|0666))<0){
    perror("Error in semget");
    exit(1);
  }
  init_sem(sem_id, WRITER, 1);
  init_sem(sem_id, READER, 0);
  init_sem(sem_id, MUTEX, 1);

  for(int i = 0; i < NUM_WRITER; ++i)
  {
    pid_t pid = fork();
    if(pid == 0)
    {
        writer(i);
        exit(1);        

    }else if(pid == -1)
    {
        perror("Fehler bei fork\n");    
        exit(1);
    }
  }

  for(int i = 0; i < NUM_READER; ++i)
  {
    pid_t pid = fork();
    if(pid == 0)
    {
        reader(i);
        exit(1);        

    }else if(pid == -1)
    {
        perror("Fehler bei fork\n");
    }
  }

  for(int i = 0; i < NUM_READER + NUM_WRITER; ++i)
  {
    int result;
    wait(&result);
  }
   
  return 0;
}

