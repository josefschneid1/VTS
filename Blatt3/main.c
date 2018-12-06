#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <errno.h>
#include <wait.h>

#define NUM_WRITERS 2
#define NUM_READERS 5

#define SEM_WRITER 0
#define MUTEX_COUNTER 1
#define SEM_COUNTER 2
#define LOOPS 3

extern int errno;

int sem_id;

void init_sem(int sem_num, int val)
{
    if(semctl(sem_id, sem_num, SETVAL, val) == -1)
    {
        perror("Fehler bei semctl!\n");
        exit(3);
    }
}

void P(int sem_num)
{
    struct sembuf s = {sem_num, -1, SEM_UNDO};
    if(semop(sem_id, &s, 1) == -1)
    {
        perror("Fehler bei semop!\n");
        exit(4);
    }
}

void V(int sem_num)
{
    struct sembuf s = {sem_num, 1, SEM_UNDO};
    if(semop(sem_id, &s, 1) == -1)
    {
        perror("Fehler bei semop!\n");
        exit(4);
    }
}

int P_non_blocking(int sem_num)
{
    struct sembuf s = {sem_num, -1, SEM_UNDO | IPC_NOWAIT};
    int ret = semop(sem_id, &s, 1);
    if(ret == -1 && errno != EAGAIN)
    {
        perror("Fehler bei semop!\n");
        exit(5);
    }else
    {
        return ret;
    }

};

void reader()
{
    for(int i = 0; i < LOOPS; ++i)
    {
        sleep(1);
        P(MUTEX_COUNTER);
        if (P_non_blocking(SEM_COUNTER) == -1) {
            P(SEM_WRITER);
        } else {
            V(SEM_COUNTER);
        }
        V(SEM_COUNTER);
        V(MUTEX_COUNTER);
        /*kritischer Bereich Reader*/
        P(MUTEX_COUNTER);
        P(SEM_COUNTER);
        if (P_non_blocking(SEM_COUNTER) == -1) {
            V(SEM_WRITER);
        } else {
            V(SEM_COUNTER);
        }
        V(MUTEX_COUNTER);

    }
}

void writer() {
    for (int i = 0; i < LOOPS; ++i)
    {
        P(SEM_WRITER);
        /*Kritischer Bereich Writer*/
        V(SEM_WRITER);
    }
}

int main() {
    key_t key;
    if((key = ftok(".", 1)) == -1)
    {
        perror("Fehler bei ftok!\n");
        exit(1);
    }

    if((sem_id = semget(key, 3, IPC_CREAT | 0666)) == -1)
    {
        perror("Fehler bei semget!\n");
        exit(2);
    }

    init_sem(0, 1);
    init_sem(1, 1);
    init_sem(2, 0);

    for(int i = 0; i < NUM_READERS; ++i)
    {
        pid_t pid = fork();
        if(pid != 0)
        {
            reader();
            exit(0);
        }
    }

    for(int i = 0; i < NUM_WRITERS; ++i)
    {
        pid_t pid = fork();
        if(pid != 0)
        {
            writer();
            exit(0);
        }
    }

    for(int i = 0; i < NUM_WRITERS + NUM_READERS; ++i)
    {
        int status;
        wait(&status);
    }

    return 0;
}