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
	char mytext[30];
  char sorted[30];
  char backup_name[30];
  strcpy(mytext,"mytext.txt");
  strcpy(sorted,"sorted.txt");
  strcpy(backup_name, "mydump-28.backup");
  restore_myfs(backup_name);
  ls_myfs();
  
  char * token;
  int fd = open_myfs(mytext,'r');
  int fsize = fdt[fd].in->file_size + 1;
  char buffer[fsize];
  read_myfs(fd,fsize,buffer);
  close_myfs(fd);
  buffer[fsize-1]='\0';

  token = strtok(buffer,",");

  int ar[100];
  int i=0;
  while(token!=NULL)
  {
    int num = atoi(token);
    ar[i++]=num;
    token = strtok(NULL,",");
  }
  sort(ar,ar+i);
  char num_buf[10];
  touch_myfs(sorted);
  fd = open_myfs(sorted,'w');
  for(int j=0;j<i;j++)
  {
    int dig = int_to_char(ar[j],num_buf);
    write_myfs(fd,dig,num_buf);

  }
  close_myfs(fd);

  ls_myfs();

}