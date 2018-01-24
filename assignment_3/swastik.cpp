#include <bits/stdc++.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#define buf_size 5

typedef struct
{
  int ar[buf_size];
  int cur_size;
} SHM;

#define shm_size sizeof(SHM)

using namespace std;


int time_to_wait = 30;
int num_consumer = 10, num_producer = 8;

void consumer(int shm_id , key_t mem_key,int consumer_no)
{
  SHM* shm_ptr = (SHM *)shmat(shm_id , NULL, 0);

  int tmp,num;



  while(true)
    {
      sleep(rand()%6);
      tmp=shm_ptr->cur_size;
      if(tmp>0)
    	{
    	  --(shm_ptr->cur_size);
    	  num=shm_ptr->ar[tmp-1];
        time_t t;
        time(&t);
    	 cout <<"consumer "<<consumer_no<<" number captured = "<<num<< " in process : "<< getpid() << ": time - "<< t <<endl;
    	}
    }





  shmdt(shm_ptr);

}

void producer(int shm_id , key_t mem_key, int producer_no)
{
  SHM* shm_ptr = (SHM *)shmat(shm_id , NULL, 0);

  int gen = rand()%100;
  int tmp;

  while(true)
    {
      sleep(rand()%6);
      tmp=shm_ptr->cur_size;
            if(tmp<5)
      	{
      	  shm_ptr->cur_size++;
      	  shm_ptr->ar[tmp]=gen;
          time_t t;
          time(&t);

          cout <<"Producer "<<producer_no<<" number produced = "<<gen<< " in process : "<< getpid() << ": time - "<< t <<endl;

      	}
    }


  shmdt(shm_ptr);

}
int main()
{
  vector<int > pids;
  int shm_id,id;
  key_t mem_key;
  int *shm_ptr;
  mem_key = ftok(".",'a');
  shm_id = shmget(mem_key, shm_size , IPC_CREAT | 0777);

  if(shm_id < 0)
    {
      perror("shmget failed");
      exit(-1);
    }

  for(int i=0;i<num_producer;i++)
    {
      id = fork();
      if(id==0)
      	{
      	  srand(i*i);
      	  producer(shm_id, mem_key,i+1);
      	}
            else
      	{
      	  pids.push_back(id);
      	}
    }

  for(int i=0;i<num_consumer;i++)
    {
      id=fork();
            if(id==0)
      	{
      	  srand(i*i);
      	  consumer(shm_id,mem_key,i+1);
      	}
            else
      	{
      	  pids.push_back(id);
      	}
    }

  sleep(30);

  for(int i=0;i<pids.size();i++)
    {
      kill(pids[i],SIGKILL);
    }

  shmctl(shm_id,IPC_RMID,NULL);

  return 1;
}
