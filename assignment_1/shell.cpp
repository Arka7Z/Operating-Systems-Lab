#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <bits/stdc++.h>
#include <sys/wait.h>

using namespace std;

void create_proc(const char *line , char **cmd_args)
{
  int status;
  int a = fork();
  if(a<0)
    {
      exit(1);
    }
  if(a==0)
    {
      //int i=0;
      /*while(cmd_args[i])
      {
        printf("%s ",cmd_args[i]);
	i++;
      }
      */
      
      if(execvp(*cmd_args,cmd_args) < 0)
	exit(1);
    }
  else
    {
      
      while (wait(&status) != a);
      //wait(NULL);
      //printf("child complete\n");
    }
}

void parser(const char *line , char **cmd_args)
{
  char *token;

  token = (char *) malloc(100);
  token = strtok((char *) line," ");

  int i=0;

  while(token!=NULL)
    {
      cmd_args[i] =(char *) malloc(strlen(token)+2);

      strcpy(cmd_args[i],token);

      cmd_args[i][strlen(cmd_args[i])]='\0';
      
      token = strtok(NULL," ");
      //cmd_args++;
      i++;

    }
  cmd_args[i]='\0';
}


int main()
{
  string line;
  char *cmd_args[64];
  while(1)
    {
      printf("shellby-$ ");
      //scanf("%[^\n]%*c",line);
      getline(cin,line);
      if(cin.eof()==true)
	break;
      
      //printf("%s \n",line);
      //printf("%d \n",strlen(line));
      if(line.size()<2)
	{
	  //printf("chech\n");
	  continue;
	}
      parser(line.c_str(),cmd_args);
      if(strcmp(cmd_args[0],"cd")==0)
	 chdir(cmd_args[1]);
      if(strcmp(cmd_args[0],"quit")!=0)
	create_proc(line.c_str(),cmd_args);
      else
	exit(1);
      
    }
  return 0;
}

