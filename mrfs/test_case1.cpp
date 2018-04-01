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
	  char dstname[30];

	  strcpy(fname,"makefile");
	  strcpy(dstname,"make");
	  ls_myfs();

	  for(int i=0;i<12;i++)
	  {
	   	copy_pc2myfs(fname,dstname);
	    
	    if(i<10)
	    {
	    	dstname[4]=(char)(i+(int)'0');
	    	dstname[5]='\0';
	    }	
	    else 
	    {
	    	dstname[4] = '1';
	    	dstname[5] = (char)(i%10+(int)'0');
	    	dstname[6] = '\0';
	    }
	  }
	  ls_myfs();
	  printf("enter file to delete \n");
	  scanf("%s",fname);
	  rm_myfs(fname);
	  ls_myfs();
}
