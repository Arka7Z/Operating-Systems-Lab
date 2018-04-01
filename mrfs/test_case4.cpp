#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <bits/stdc++.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string>
#include <errno.h>
#include <sys/wait.h>

#include "myfs.h"

using namespace std;


int main()
{




	create_myfs(10);

	int status;

	char alpha[30];
	strcpy(alpha,"alphabets.txt");
	char mypapers[30];
	strcpy(mypapers,"mypapers");
	char myroot[30];
	strcpy(myroot,"myroot");
	char mydocs[30];
	strcpy(mydocs,"mydocs");
	char mycode[30];
	strcpy(mycode,"mycode");
	char mytext[30];
	strcpy(mytext,"mytext");

	sem_t * fs_sem = sem_open("/fs_sem", O_CREAT, 0777, 1);

	mkdir_myfs(myroot);
	chdir_myfs(myroot);
	mkdir_myfs(mydocs);
	mkdir_myfs(mycode);

	char fname[30];
	char dstname[30];
	int pid = fork();
	if(pid ==0)
	{	
		sem_t *fs_sem = sem_open("/fs_sem", 0);
		
		sem_wait(fs_sem);
		fs = (char *)shmat(shm_id,NULL,0);
		chdir_myfs(mydocs);
		mkdir_myfs(mytext);
		mkdir_myfs(mypapers);
		chdir_myfs(mytext);
		touch_myfs(alpha);
		int fd = open_myfs(alpha,'w');
		char buffer[27];
		buffer[26]='\0';
		for(int i=0;i<26;i++)
		{
			buffer[0]=(char)(i+(int)'A');
			int nwrite = write_myfs(fd,1,buffer);
			if(nwrite<0)
			{
				printf("problem\n");
				exit(-1);
			}
		}
		close_myfs(fd);
		chdir_myfs("..");
		chdir_myfs("..");
		sem_post(fs_sem);
		exit(1);

	}
	if(pid>0)
	{
		sem_wait(fs_sem);
		fs = (char *)shmat(shm_id,NULL,0);	
		chdir_myfs(mycode);
		printf("enter source file name\n");
		scanf("%s",fname);
		printf("enter destination name\n");
		scanf("%s",dstname);
		copy_pc2myfs(fname,dstname);
		chdir_myfs("..");
		sem_post(fs_sem);
		while (wait(&status)!= pid);
	}
	check();
	return 1;
}