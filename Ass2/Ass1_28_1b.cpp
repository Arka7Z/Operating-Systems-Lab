#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <bits/stdc++.h>
#include <sys/wait.h>
#include <fcntl.h>

#define REMOVE_SPACES(x) x.erase(std::remove(x.begin(), x.end(), ' '), x.end())
using namespace std;


void create_proc(const char *line , char **cmd_args,bool directed=false,bool in=true,string filename="")
{
  int status;
  int a = fork();
  int file_desc;
  if (directed)
    file_desc=open(filename.c_str(),O_RDWR);
  if(a<0)
    {
      exit(1);
    }
  if(a==0)
    {
      if(directed)
      {
          if(in)
          {
              close(0);
              dup(file_desc);
          }
          else
          {
            close(1);
            dup(file_desc);
          }
      }
      if(execvp(*cmd_args,cmd_args) < 0)
	       exit(1);
    }
  else
    {
        while (wait(&status) != a)
        ;
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
      i++;

    }
  cmd_args[i]='\0';
}



void select_Proc(bool directed=false,bool in=true)
{
  printf("shellby-$ ");
  string line,file;
  getchar();
  char *cmd_args[64];
  getline(cin,line);
  string line_tmp=line;
  if(cin.eof()==true)
     exit(1);
  if(line.size()<2)
   {
      return;
   }
   if (directed)
   {
     if(in)
     {
       line=line_tmp.substr(0,line_tmp.find("<"));
       file=line_tmp.substr(line_tmp.find("<")+1,line_tmp.length());
       REMOVE_SPACES(file);
     }
     else
     {
       line=line_tmp.substr(0,line_tmp.find(">"));
       file=line_tmp.substr(line_tmp.find(">")+1,line_tmp.length());
       REMOVE_SPACES(file);
     }
   }
  parser(line.c_str(),cmd_args);
  if(strcmp(cmd_args[0],"cd")==0)
     chdir(cmd_args[1]);
  if(strcmp(cmd_args[0],"quit")!=0)
     create_proc(line.c_str(),cmd_args,directed,in,file);
  else
     exit(1);
}

int main()
{
  string option;
  while(1)
    {
      cin>>option;
      {
          if(option== "A")
              select_Proc();
          else if(option== "B")
              select_Proc();
          else  if(option== "C")
              select_Proc(true,true);
          else if(option== "D")
              select_Proc(true,false);
          else if(option== "E")
                ;
          else   if(option== "F")
                ;
          else   if(option== "G")
                ;
      }
    }
  return 0;
}
