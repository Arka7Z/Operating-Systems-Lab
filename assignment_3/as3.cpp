#include <bits/stdc++.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#define buf_size 5
#define prime_lim 10000

typedef struct
{
  int ar[buf_size];
  int cur_size;
} SHM;

#define shm_size sizeof(SHM)

using namespace std;

vector<int> primes;
int time_to_wait = 30;
int num_consumer = 10, num_producer = 8;

void SieveOfEratosthenes()
{
    // Create a boolean array "prime[0..n]" and initialize
    // all entries it as true. A value in prime[i] will
    // finally be false if i is Not a prime, else true.
    int n=prime_lim;
    bool prime[n+1];
    memset(prime, true, sizeof(prime));

    for (int p=2; p*p<=n; p++)
      {
        // If prime[p] is not changed, then it is a prime
        if (prime[p] == true)
	  {
            // Update all multiples of p
            for (int i=p*2; i<=n; i += p)
	      prime[i] = false;
	  }
      }
    
    // Print all prime numbers
    for (int p=2; p<=n; p++)
      if (prime[p])
	primes.push_back(p);
}

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
	  cout <<"Consumer "<<consumer_no<<" : "<<num<<", time: "<< t <<endl;
    	}
    }





  shmdt(shm_ptr);

}

void producer(int shm_id , key_t mem_key, int producer_no)
{
  SHM* shm_ptr = (SHM *)shmat(shm_id , NULL, 0);

  int gen;
  int tmp;

  while(true)
    {
      sleep(rand()%6);
      tmp=shm_ptr->cur_size;
      if(tmp<5)
	{
	  shm_ptr->cur_size++;
	  gen = rand()%(primes.size());
	  shm_ptr->ar[tmp]=primes[gen];
	  time_t t;
	  time(&t);
	  
	  cout <<"Producer "<<producer_no<<": "<<shm_ptr->ar[tmp]<< ",time : "<< t <<endl;
	  
	}
    }
  

  shmdt(shm_ptr);

}
int main()
{
  vector<int > pids;
  SieveOfEratosthenes();
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
      	  srand(i);
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
      	  srand(i+num_consumer);
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
