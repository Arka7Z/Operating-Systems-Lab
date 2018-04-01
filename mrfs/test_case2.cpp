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

#include "myfs.h"

using namespace std;

int main()
{
	create_myfs(10);
	char fname[30];
  char mytext[30];
  strcpy(fname,"mytext.txt");
  strcpy(mytext,"mytext.txt");
  touch_myfs(mytext);
  // printf("touch done\n");
  // return;
  int fd = open_myfs(mytext,'w');
  int fd2;
  char tmp_buf[10];
  int N;

  printf("enter N\n");
  scanf("%d",&N); 

  int fsize=0;
    for(int i=0;i<100;i++)
    {
      int r = rand()%1000007;
      int dig = int_to_char(r,tmp_buf);
      // printf("generated = %d\n",r);

      int nwrite = write_myfs(fd,dig,tmp_buf);
      // printf("current file size = %d\n",fdt[fd].in->file_size);
      // printf("dig = %d\n", dig);
      fsize += nwrite;
    }
  close_myfs(fd);
  // printf("fsize = %d\n",fsize);
  char buffer[fsize+100];
  for(int i=0;i<N;i++)
  {
    sprintf(fname,"mytext-%d.txt",i+1);
    touch_myfs(fname);
    fd2 = open_myfs(fname,'w');
    fd = open_myfs(mytext,'r');
    int nread =0;
    if((nread =read_myfs(fd,fsize,buffer))<0)
      {
        perror("error in reading mytext.txt\n");
        exit(-1);
      }
    printf("nread = %d\n", nread);
    write_myfs(fd2,nread,buffer);
    close_myfs(fd2);
    close_myfs(fd);
  }

  char dump_name[30];
  strcpy(dump_name,"mydump-28.backup");
  dump_myfs(dump_name);
  ls_myfs();
 }