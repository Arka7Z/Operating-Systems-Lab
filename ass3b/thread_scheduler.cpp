#include <bits/stdc++.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

using namespace std;

int * status;
typedef struct
{
  pthread_t *w;
  int n;
} sched_args;

void sigusr2_handler(int signum)
{
  //signal(SIGUSR1,sigusr1_handler);
  signal(SIGUSR2,sigusr2_handler);
  cout<<"back to work - "<<pthread_self()<<endl;
}
void sigusr1_handler(int signum)
{
  signal(SIGUSR1,sigusr1_handler);
  signal(SIGUSR2,sigusr2_handler);
  pause();
}



void* worker_func(void * tmp)
{
  int w_id = *((int *)tmp);
  usleep(100000);
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
  status[w_id]=2;
  while(1);
}


void sigint_handler(int signum)
{
  cout <<" caught SIGINT"<<endl;
  if(signum==SIGINT)
    cout<<"HEHE"<<endl;
  signal(SIGINT,sigint_handler);
  
}

void* scheduler_func(void * args)
{
  sched_args * util = (sched_args *) args;
  pthread_t * workers = util->w;
  int n = util->n;
  
  queue <int > ready;
  int running_n = n;
  
  for(int i=0;i<n;i++)
    {
      pthread_kill(workers[i],SIGUSR1);
    }
  for(int i=0;i<n;i++)
    {
      ready.push(i);
    }
  while(1)
    {
      int w_id = ready.front();
      pthread_kill(workers[w_id],SIGUSR2);
      status[w_id]=1;
      sleep(1);
      if(status[w_id]==2)
	{
	  pthread_kill(workers[w_id],SIGKILL);
 	  ready.pop();
	  running_n--;
	  if(running_n==0)
	    break;
	}
      else
	{
	  pthread_kill(workers[w_id],SIGUSR1);
	  status[w_id]=0;
	  int t = ready.front();
	  ready.pop();
	  ready.push(t);
	}
      
    }
  pthread_exit(0);
}
void* reporter_func(void *nn)
{
  int n = *((int *) nn);
  int prev_running=-1;

  int terminated=0;
  while(terminated!=n)
    {
      terminated=0;
      for(int i=0;i<n;i++)
	{
	  if(status[i]==1)
	    {
	      if(prev_running>=0 && prev_running!=i) 
		{
		  cout<<"worker - "<<i<<" has paused "<<endl;
		  cout<<"worker - "<<i<<" is running"<<endl;
		  prev_running=i;
		}
	    }
	  else if(status[i]==2)
	    {
	      terminated++;
	    }
	}
    }
  int i=0;
  pthread_exit(0);
	  
}
int main()
{
  signal(SIGINT,sigint_handler);

  int n;
  cout << "enter N " << endl;
  cin >> n;

  pthread_attr_t *attr=NULL;
  status = (int *)malloc(sizeof(int)*n);
  memset(status,0,n*sizeof(int));
  
  pthread_t workers[n];
  pthread_t sched_t;
  pthread_t report_t;

  for(int i=0;i<n;i++)
    {
      pthread_create(&workers[i],attr,worker_func,(void *)&i);
    }
  
  sched_args util;
  util.w = &workers[0];
  util.n = n;
  pthread_create(&sched_t,attr,scheduler_func,(void *)&util);

  pthread_create(&report_t,attr,reporter_func,(void *)&n);
  void ** ret;
  pthread_join(sched_t,ret);
  pthread_join(report_t,ret);
  return EXIT_SUCCESS;
}
