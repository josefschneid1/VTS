#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>

void init_sem(int semid)
{

    semctl(semid,0, SETVAL,1);
}

void P(int sem_num)
{
    struct sembuf ops[1];
    ops[0].sem_num = 0;
    ops[0].sem_flg = 0;
    ops[0].sem_op = -1;

    semop(sem_num, ops,1);
}

void V(int sem_num)
{
    struct sembuf ops[1];
    ops[0].sem_num = 0;
    ops[0].sem_flg = 0;
    ops[0].sem_op = 1;
    semop(sem_num, ops,1);
}

void kritischenBereichBetreten(int id)
{
      printf("%d betretet kritischen Bereich\n", id);
      sleep(1);
      printf("%d verlässt kritischen Bereich\n", id);
}

void unkritischenBereichBetreten(int id)
{
    printf("%d betretet unkrititischen Bereich\n", id);
    sleep(1);
    printf("%d verlässt unkrititischen Bereich\n", id);
}


int main(void)
{
  remove("key");//Remove file if it exists
  FILE* fp = fopen("key", "w+");//Create file
  fclose(fp);
  key_t key = ftok("key", 1);

  int semID = semget(key, 1, IPC_CREAT|0666);
  init_sem(semID);

  pid_t pid;
  pid = fork();
  if(pid == 0)
  {
    //First Child
    pid = (pid_t)getpid();
    printf("ProzessNr2 mit id: %d\n", pid);
    for(int i = 0; i < 3; ++i)
    {   
        P(semID);
        kritischenBereichBetreten(2);
        V(semID);
    }
    unkritischenBereichBetreten(2);
  }else if(pid > 0)
  {
    pid = fork();
    if(pid == 0)
    {
    //Second Child
    pid = (int)getpid();
    printf("ProzessNR3 mit id: %d\n", pid);

    }
    else if(pid > 0)
    {
    //Parent
    printf("ProzessNR1 mit id: %d\n", (int) getpid());
    for(int i = 0; i < 3; ++i)
    {
      P(semID);
      kritischenBereichBetreten(1);
      V(semID);    
    }
    unkritischenBereichBetreten(1);
    }
    else
    {
      perror("Zweiter fork liefert -1");
      exit(1);
    }
  }
  else
  {
    perror("Erster fork liefert -1");
    exit(1);
  }
  for(int i = 0; i < 2; ++i)
  {
    int res;
    wait(&res);
  }
  return EXIT_SUCCESS;
}
