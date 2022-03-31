#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#define SIZE 100

struct memory
{
	char buff[SIZE];
	int status, pid1, pid2;
};

struct memory* shmptr;

void handler(int signum)
{
	if(signum == SIGUSR2)
	{
		printf("Received from User1: ");
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
	shmptr = (struct memory*)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmid, 0);
	if(shmptr == (void*)-1)
	{
		perror("UNABLE TO ATTACH SHARED MEMORY");
		exit(1);
	}
	shmptr->pid2 = pid;
	shmptr->status = -1;
	signal(SIGUSR2, handler);

	while(1)
	{
		sleep(1);
		fgets(shmptr->buff, SIZE, stdin);
		shmptr->status = 0;
		kill(shmptr->pid1, SIGUSR1);
		while(shmptr->status == 1)
			continue;
	}
	if(shm_unlink(shm_name) != 0)
	{
		perror("UNABLE TO DETACH");
		exit(1);
	}
	return 0;
}
