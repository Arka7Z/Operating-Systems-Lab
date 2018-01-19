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

void parser(const char *line , char **cmd_args);
vector<string> split(const string &s, char delim);

vector<string> split(const string &s, char delim)
{
    stringstream ss(s);
    string item;
    vector<string> tokens;
    while (getline(ss, item, delim))
    {
        tokens.push_back(item);
    }
    return tokens;
}


int spawn_proc (int in, int out, char **cmd)
{
  pid_t pid;

  if ((pid = fork ()) == 0)
    {
      if (in != 0)
        {
          dup2 (in, 0);
          close (in);
        }

      if (out != 1)
        {
          dup2 (out, 1);
          close (out);
        }

      return execvp (*cmd, cmd);
    }

  return pid;
}


int pipe_Command()
{
  printf("shellby-$ ");
  string line,file;
  getchar();
  char *cmd_args[64];
  getline(cin,line);
  vector<string> commands=split(line,'|');
  //list<char* cmd_args[64] > command_list(commands.size());
  //  for(int i=0;i<commands.size();i++)
  //   cout<<commands[i]<<endl;

  //////
  int n=commands.size();
  int i;
  pid_t pid;
  int in, fd [2];

  /* The first process should get its input from the original file descriptor 0.  */
  in = 0;

  /* Note the loop bound, we spawn here all, but the last stage of the pipeline.  */
  for (i = 0; i < n - 1; ++i)
    {
      pipe (fd);
      char* cmd_args[64];
      parser(commands[i].c_str(),cmd_args);
      /* f [1] is the write end of the pipe, we carry `in` from the prev iteration.  */
      spawn_proc (in, fd [1], cmd_args);

      /* No need for the write end of the pipe, the child will write here.  */
      close (fd [1]);

      /* Keep the read end of the pipe, the next child will read from there.  */
      in = fd [0];
    }

  /* Last stage of the pipeline - set stdin be the read end of the previous pipe
     and output to the original file descriptor 1. */
  if (in != 0)
    dup2 (in, 0);

  /* Execute the last stage with the current process. */

  parser(commands[i].c_str(),cmd_args);
  return execvp (*cmd_args, cmd_args);

}
void create_proc(const char *line , char **cmd_args,bool directed=false,bool in=true,string filename="",bool background=false)
{
  int status;
  int a = fork();
  int file_desc;
  if (directed)
  {
      if(!in)
      {
         file_desc=open(filename.c_str(),O_CREAT,S_IWGRP | S_IRUSR | S_IWUSR |S_IXUSR	);
         close(file_desc);
      }
      file_desc=open(filename.c_str(),O_RDWR);
  }
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
      if(!background)
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



void select_Proc(bool directed=false,bool in=true,bool background=false)
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
     create_proc(line.c_str(),cmd_args,directed,in,file,background);
  else
     exit(1);
}

int main()
{
  string option;
  while(1)
    {
      cout<<"Option: ";
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
                select_Proc(false,true,true);
          else   if(option== "F")
                pipe_Command();
          else   if(option== "G")
                exit(1);
      }
    }
  return 0;
}
