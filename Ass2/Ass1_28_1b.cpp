#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <bits/stdc++.h>
#include <sys/wait.h>
#include <fcntl.h>


using namespace std;


void create_proc_with_redirection(const char *line , char **cmd_args,string infile,bool in)
{
  int status;
  int a = fork();
  int file_desc=open(infile.c_str(),O_RDWR);
  if(a<0)
    {
      exit(1);
    }
  if(a==0)
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
      if(execvp(*cmd_args,cmd_args) < 0)
	    exit(1);
    }
  else
    {
        while (wait(&status) != a)
        ;
    }
}

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

vector<string> split(const char *phrase, string delimiter)
{

    vector<string> list;
    string s = string(phrase);
    size_t pos = 0;
    string token;
    while ((pos = s.find(delimiter)) != string::npos) {
        token = s.substr(0, pos);
        list.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    cout<<"before return "<<list[0]<<" "<<list[1]<<endl;
    return list;
}
void redirect_Proc()
{
  printf("shellby-$ ");
  string line;
  getchar();
  char *cmd_args[64];
  getline(cin,line);
  string s=line;

  parser(line.c_str(),cmd_args);
  string file = cmd_args[2];
  cout <<file <<"-finelname "<<endl;
  line = cmd_args[0];

  parser(line.c_str(),cmd_args);

  if(strcmp(cmd_args[0],"quit")!=0)
     create_proc_with_redirection(line.c_str(),cmd_args,file,true);
  else
     exit(1);
}

void select_Proc()
{
    printf("shellby-$ ");
  string line;
  char *cmd_args[64];
  getline(cin,line);
  if(cin.eof()==true)
     return;
  if(line.size()<2)
   {
      return;
   }
  parser(line.c_str(),cmd_args);
  if(strcmp(cmd_args[0],"cd")==0)
     chdir(cmd_args[1]);
  if(strcmp(cmd_args[0],"quit")!=0)
     create_proc(line.c_str(),cmd_args);
  else
     exit(1);
}
void out_direct_proc()
{
  printf("shellby-$ ");
  string line;
  getchar();
  char *cmd_args[64];
  getline(cin,line);
  string s=line;

  parser(line.c_str(),cmd_args);
  string file;

  for(int i = 0; i < 64 ;i++)
  {
    if(strcmp(cmd_args[i],">")==0)
    {
      file = cmd_args[i+1];
      break;
    }
    line+=cmd_args[i];
  }

  cout <<file <<"-finelname "<<endl;
  //line = cmd_args[0];

  parser(line.c_str(),cmd_args);

  if(strcmp(cmd_args[0],"quit")!=0)
     create_proc_with_redirection(line.c_str(),cmd_args,file,false);
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
                  //break;
      if(option== "B")
                  select_Proc();
                  //break;
        if(option== "C")

                  redirect_Proc();

      if(option== "D")
                  out_direct_proc();
                //  break;
      if(option== "E");

                //  break;
        if(option== "F");

                //  break;
        if(option== "G");
                  exit(1);
                //  break;
      }




    }
  return 0;
}
