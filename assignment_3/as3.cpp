#include <bits/stdc++.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

typedef struct
{
  int ar[5];
  int id;
} SHM;

#define shm_size sizeof(SHM)*4

using namespace std;

int time_to_wait = 30;
int num_consumer, num_producer;



int main()
{
  int shm_id;
  key_t mem_key;
  int *shm_ptr;
  mem_key = ftok(".",'a');
  shm_id = shmget(mem_key, shm_size , IPC_CREAT | 0777);
  
  if(shm_id < 0)
    {
      perror("shmget failed");
      exit(-1);
    }
 }
 
