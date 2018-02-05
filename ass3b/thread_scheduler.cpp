#include <bits/stdc++.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

using namespace std;

int * status;

void sigusr1_handler(int signum)
{
  signal(SIGUSR1,sigusr1_handler);
  signal(SIGUSR2,sigusr2_handler);
  pause();
}

void sigusr2_handler(int signum)
{
  signal(SIGUSR1,sigusr1_handler);
  signal(SIGUSR2,sigusr2_handler);
  cout<<"back to work"<<endl;
}

void* worker_func(void * unused)
{
  usleep(1000);
  signal(SIGUSR1,sigusr1_handler);
  signal(SIGUSR2,sigusr2_handler);

  int ar[1000];

  srand(time(NULL));

  for(int i=0;i<1000;i++)
    {
      ar[i]=rand()%10000;
    }
  sort(ar,ar+1000);

  sleep(rand()%11+1);
  pthread_exit();
}


void sigint_handler(int signum)
{
  cout <<" caught SIGINT"<<endl;
  if(signum==SIGINT)
    cout<<"HEHE"<<endl;
  signal(SIGINT,sigint_handler);
  
}

void* scheduler_func(pthread_t *workers,int &n)
{
  queue <int > ready;
  int terminated= n;
  
  for(int i=0;i<n;i++)
    {
      ready.push_back(i);
      pthread_kill(workers[i],SIGUSR1);
    }
  for(int i=0;i<n;i++)
    {
      int t = ready.pop();

      
      
      


      
  
}
void* reporter_func(int* n)
{
  
}
int main()
{
  int n;
  cout << "enter N " << endl;
  cin >> n;

  pthread_attr_t *attr=NULL;
  status = (int *)malloc(sizeof(int)*n);
  memset(status,0,n*sizeof(int));
  
  pthread_t workers[n];

  for(int i=0;i<n;i++)
    {
      pthread_create(&workers[i],attr,worker_func,NULL);
    }
  signal(SIGINT,sigint_handler);
  
  return EXIT_SUCCESS;
}
