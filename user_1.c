#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

struct memory {
	char buff[100];
	int status, pid1, pid2;
};

struct memory* shmptr;

void handler(int signum)
{
	if(signum == SIGUSR1)
	{
		printf("Received from User2: ");
		puts(shmptr->buff);
	}
}

int main()
{
	int pid = getpid();
	int shmid;
	key_t key = ftok("progfile", 65);

	shmid = shmget(key, sizeof(struct memory), IPC_CREAT | 0666);
	if(shmid < 0)
	{
		perror("UNABLE TO CREATE SHARED MEMORY");
		exit(1);
	}
	shmptr = (struct memory*)shmat(shmid, NULL, 0);
	if(shmptr == (void*)-1)
	{
		perror("UNABLE TO ATTACH SHARED MEMORY");
		exit(1);
	}
	shmptr->pid1 = pid;
	shmptr->status = -1;
	signal(SIGUSR1, handler);

	while(1)
	{
		do
		{
			sleep(1);
			fgets(shmptr->buff, 100, stdin);
			shmptr->status = 0;
			kill(shmptr->pid2, SIGUSR2);
		}while(shmptr->status == 0);
	}

	if(shmdt((void*)shmptr) != 0)
	{
		perror("UNABLE TO DETACH");
		exit(1);
	}
	if(shmctl(shmid, IPC_RMID, NULL) != 0)
	{
		perror("UNABLE TO DELETE");
		exit(1);
	}
	return 0;
}

