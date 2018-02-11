#include <bits/stdc++.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#define bufsize 1000

using namespace std;

int * status;
typedef struct
{
  pthread_t *w;
  int n;
} sched_args;

bool gdb=false;

void sigusr1_handler(int);
void sigusr2_handler(int);

void sigusr2_handler(int signum)
{
  
  //cout<<"back to work - "<<pthread_self()<<endl;
  // signal(SIGUSR1,sigusr1_handler);
  signal(SIGUSR2,sigusr2_handler);
  if(gdb)
    cout<<"returning from sigusr2_handler()"<<endl;
}

void sigusr1_handler(int signum)
{
  if(signum==SIGUSR1)
    {
      //cout<<"sigusr1 handled"<<endl;
      signal(SIGUSR1,sigusr1_handler);
      // signal(SIGUSR2,sigusr2_handler);
      //cout<<"thread - "<<pthread_self()<<" pausing"<<endl;
      pause();
      if(gdb)
	cout<<"returning from sigusr1_handler()"<<endl;
    }
}

void sigint_handler(int signum)
{
  cout <<" caught SIGINT"<<endl;
  if(signum==SIGINT)
    cout<<"HEHE"<<endl;
  signal(SIGINT,sigint_handler);
}

void* worker_func(void * tmp)
{
  //signal(SIGUSR1,sigusr1_handler);
  //signal(SIGUSR2,sigusr2_handler);

  //pause();
  usleep(1000);
  
  int * w_id = (int *)tmp;
  //usleep(100);

  int ar[bufsize];

  srand(time(NULL));

  for(int i=0;i<bufsize;i++)
    {
      //ar[i]= i > 0 ? (ar[i-1] * 23) % 10007 : *w_id;
      ar[i]=(rand()%10007+1);
    }
  //printf("generated in worker in %d\n",*w_id);
  fflush(stdout);

  int min_no=INT_MIN;
  int min_pos=-1;
  int swap;
  for(int i=0;i<bufsize-1;i++)
    {
      min_no=ar[i];
      min_pos=i;
      for(int j=i+1;j<bufsize;j++)
	{
	  if(min_no>ar[j])
	    {
	      min_no=ar[j];
	      min_pos=j;
	    }
	}
      if(min_no<ar[i])
	{
	  swap=ar[min_pos];
	  ar[min_pos]=ar[i];
	  ar[i]=swap;
	}
      
    }

  if(gdb)
    printf("sorted in worker in %d\n",*w_id);
  fflush(stdout);
  
  int t = rand()%11+1;
  
  while(t>1)
    {
      if(gdb)
	printf("worker %d sleeping for %d seconds\n",*w_id,t-1);
      t=sleep(t);
    }
  if(gdb)
    printf("worker_func() %d completed\n",*w_id);
  status[*w_id]=2;

  pthread_exit(0);
  //while(1);
}

void* scheduler_func(void * args)
{
  cout << "scheduler thread started"<<endl;
  sched_args * util = (sched_args *) args;
  pthread_t * workers = util->w;
  int n = util->n;
  
  queue <int > ready;
  int running_n = n;
  
  for(int i=0;i<n;i++)
    {
      ready.push(i);
    }
  struct timespec ts;  
  int s;
  while(!ready.empty())
    {
      int w_id;
      if(ready.size()!=1)
	{
	  w_id=ready.front();
	  pthread_kill(workers[w_id],SIGUSR2);
	  //printf("running worker %d\n",w_id);
	  status[w_id]=1;
	  
	  
	  //if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
	  //{
	  //  printf("clock_gettime() error");
	  /* Handle error */
	  //}

	  //ts.tv_sec += 1;
	  
	  //s = pthread_timedjoin_np(workers[w_id], NULL, &ts);
	  //if (s != 0)
	  //{
	  /* Handle error */
	  //}
	  sleep(1);

	}
      else
	{
	  w_id=ready.front();
	  pthread_kill(workers[w_id],SIGUSR2);
	  status[w_id]=1;

	  pthread_join(workers[w_id],NULL);
	  break;

	}
      
      
      
      if(status[w_id]==2)
	{
	  
	  
	  //pthread_kill(workers[w_id],SIGKILL);
	  if(gdb)
	    printf("finished thread %d \n",w_id);
	  pthread_cancel(workers[w_id]);
 	  ready.pop();
	  running_n--;
	  if(gdb)
	    {
	      cout<<"finished removing thread from queue"<<endl;
	      printf("ready size = %d\n",ready.size());
	    }
	  //printf("running = %d\n",running_n);
	  if(running_n == 0 || ready.empty())
	    break;
	}
      else if(status[w_id]==1)
	{

	  pthread_kill(workers[w_id],SIGUSR1);

	  status[w_id]=0;
	  if(gdb)
	    {
	      printf("worker %d is pausing\n",w_id);
	      //printf("ready size = %d\n",ready.size());
	    }
	  int t = ready.front();
	  ready.pop();
	  ready.push(t);
	  
	}
      else
	cout<<"strange error \n";
      
    }
}

void* reporter_func(void *nn)
{
  cout<<"reporter thread started"<<endl;
  int n = *((int *) nn);
  int prev_running=-1;
  int ack[n];
  memset(status,0,n*sizeof(int));
  
  int terminated=0;
  while(terminated!=n)
    {
      //cout<<"polling"<<endl;
      terminated=0;
      for(int i=0;i<n;i++)
	{
	  if(status[i]==1)
	    {
	      //printf("found a running thread\n");
	      if(prev_running!=i) 
		{
		  if(prev_running>=0)
		    {
		      printf("worker - %d has paused\n",prev_running);
		    }
		      printf("worker - %d is running\n",i);
		    
		  prev_running=i;
		}
	    }
	  else if(status[i]==2)
	    {
	      if(ack[i]==0)
		{
		  ack[i]=1;
		  printf("worker - %d has terminated\n",i);
		}
	      terminated++;
	    }
	}
    }
  //pthread_exit(0)
}

int main()
{
  //signal(SIGINT,sigint_handler);

  signal(SIGUSR1,sigusr1_handler);
  signal(SIGUSR2,sigusr2_handler);

  int n;
  cout << "enter N " << endl;
  cin >> n;

  
  pthread_attr_t *attr=NULL;
  status = (int *)malloc(sizeof(int)*n);
  memset(status,0,n*sizeof(int));
  
  pthread_t workers[n];
  pthread_t sched_t;
  pthread_t report_t;
  int id[n];
  for(int i=0;i<n;i++)
    {
      id[i]=i;
      pthread_create(&workers[i],attr,worker_func,(void *)&id[i]);
      pthread_kill(workers[i],SIGUSR1);
    }

  sched_args util;
  util.w = &workers[0];
  util.n = n;
  pthread_create(&sched_t,attr,scheduler_func,(void *)&util);

  //cout<<"check1 in main()"<<endl;
  pthread_create(&report_t,attr,reporter_func,(void *)&n);
  void ** ret;
  pthread_join(sched_t, NULL);

  cout<<"all threads terminated\n";
  cout<<"scheduler finished"<<endl;
  pthread_cancel(report_t);
  
  cout <<"reporter finished"<<endl;
  return EXIT_SUCCESS;
}
