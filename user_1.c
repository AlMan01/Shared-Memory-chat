#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

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
	const char* shm_name = "Memory";
	shmid = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
	if(shmid == -1)
	{
		perror("UNABLE TO CREATE SHARED MEMORY");
		exit(1);
	}
	ftruncate(shmid, 100);
	shmptr = (struct memory*)mmap(0, 100, PROT_READ | PROT_WRITE, MAP_SHARED, shmid, 0);
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
	if(shm_unlink(shm_name) != 0)
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

