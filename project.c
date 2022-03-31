#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>

char path[50];
pthread_t vthr[10];
int index_thr;

void add_process(pid_t pid)
{
	char buff[20];
	int procs = open(path, O_RDWR | O_APPEND);

	sprintf(buff, "%d\n", pid);
	write(procs, buff, strlen(buff));
	close(procs);
}

void delete_process(pid_t pid)
{
	int current, nr = 0;
	int v[100];
	int f = open(path, O_RDONLY);
	char procs[255];
	read(f, procs, 255);
	char *lin = strtok(procs, "\n");

	while(lin != NULL)
	{
		current = atoi(lin);
		if(current != pid)
			v[nr++] = current;
		
		lin = strtok(NULL, "\n");
	}
	close(f);

	f = open(path, O_RDWR | O_TRUNC, S_IRWXU);
	for(int i = 0; i < nr; i++)
	{
		char buff[20];
		sprintf(buff, "%d\n", v[i]);
		write(f, buff, strlen(buff));
	}
	close(f);
}

void* ls(void* v)
{
	pid_t pid = fork();
		
	if(pid == 0)
	{
		int pid2 = getpid();

		add_process(pid2);

		char *argv[2];
		argv[0] = "ls";
		argv[1] = NULL;
		
		printf("Process %d is loading...\n", index_thr);
		sleep(10);
		printf("Process %d: ", index_thr);
		fflush(NULL);

		int a = execve("/bin/ls", argv, NULL);
	}
	else if(pid > 0)
	{
		wait(NULL);
		delete_process(pid);
		return NULL;
	}
}

void* cat(void* v)
{
	pid_t pid = fork();
	
	if(pid == 0)
	{
		int pid2 = getpid();

		add_process(pid2);

		char *argv[3];
		argv[0] = "cat";
		argv[1] = path;
		argv[2] = NULL;
		
		printf("Process %d is loading...\n", index_thr);
		printf("Process %d: \n", index_thr);

		int a = execve("/bin/cat", argv, NULL);
	}
	else if(pid > 0)
	{
		wait(NULL);
		delete_process(pid);
		return NULL;
	}
}

void* nru(void* v)
{
	pid_t pid = fork();
	
	if(pid == 0)
	{
		int pid2 = getpid();

		add_process(pid2);

		printf("Process %d is loading...\n", index_thr);
		printf("Process %d: \n", index_thr);
		printf("Number of active users: ");
		fflush(NULL);

		system("/bin/ls -lA | egrep -c \'^d\'");	
	}
	else if(pid > 0)
	{
		wait(NULL);
		delete_process(pid);
		return NULL;
	}
}

void exit_program()
{
	printf("Waiting for the rest of the processes...\n");
	for(int i = 0; i < index_thr; i++)
		pthread_join(vthr[i], NULL);
	
	char procs_path[50] = "./";
	strcat(procs_path, path);

	remove(procs_path);

	char* dir = strtok(path, "/");	
	char dir_path[50] = "./";
	strcat(dir_path, dir);
	
	remove(dir_path);

	exit(0);
}


int main()
{
	int u = open("users.txt", O_RDONLY);
	bool ok = false;
	char user[20], password[20], users_pass[255];
	char* user_name;
	
	read(u, users_pass, 255);
	
	while(!ok)
	{
		printf("Username: ");
		scanf("%s", user);
		printf("\nPassword: ");
		scanf("%s", password);

		// read from file
		read(u, users_pass, 255);

		// check data
		user_name = strtok(users_pass, ":\n");
		while(!ok && user_name != NULL)
		{
			char* pass = strtok(NULL, ":\n");
			if(strcmp(user_name, user) == 0 && strcmp(pass, password) == 0)
					ok = true;
			
			user_name = strtok(NULL, ":\n");
		}

		if(ok == false)
		{
			printf("Username or password wrong\n");
			lseek(u, 0, SEEK_SET);
		}
	}

	printf("You logged in! Available commands: ls, cat, nru, exit\n");
	lseek(u, 0, SEEK_SET);
	
	// folder for active user
 	mkdir(user, S_IRWXU);
	close(u);

	// create procs file and write parent's pid
	int id = getpid();
	strcpy(path, user);
	strcat(path, "/procs.txt");
	int procs = open(path, O_RDWR|O_CREAT|O_APPEND, S_IRWXU);
	char buff[20];
	sprintf(buff, "%d\n", id);
	write(procs, buff, strlen(buff));
	close(procs);
	
	// interpret commands
	char cmd[100];
	int cnt = 1;

	while(cnt > 0)
	{
		if(cnt > 1)
			printf(">>");
		fgets(cmd, 100, stdin);
		cnt++;

		if(strcmp(cmd,"exit\n") == 0)
			exit_program();
		else if(strcmp(cmd, "ls\n") == 0)
		{
			if (pthread_create(&vthr[index_thr++], NULL, ls, NULL))
			{
				printf("Error at ls!\n");
				return errno;
			}
		}
		else if(strcmp(cmd, "cat\n") == 0)
		{
			if (pthread_create(&vthr[index_thr++], NULL, cat, NULL))
			{
				printf("Error at cat!\n");
				return errno;
			}
		}
		else if(strcmp(cmd, "nru\n") == 0)
		{
			if (pthread_create(&vthr[index_thr++], NULL, nru, NULL))
			{
				printf("Error at nru!\n");
				return errno;
			}
		}
		else if(strcmp(cmd, "\n") == 0)
			continue;
		else
			printf("Invalid command!\n");
	}

	return 0;
}
