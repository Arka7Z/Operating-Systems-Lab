#include <bits/stdc++.h>
#include <random>
#include <iostream>
#define L 3
#define loop(i, n) for(int i = 0; i < n; ++i)


using namespace std;

float rr(int n, vector<int> burst_time,vector<int> arrival_time,int timeslice)
{
  vector<int> waiting_time(n,0),turnaround_time(n,0),remaining_time(burst_time.begin(),burst_time.end());


    // RUN SCHEDULER
    int m,nextval;
    queue<int> Queue;
    nextval=0;
    Queue.push(0);
    int i=1;
     while(!Queue.empty())
     {
        m=Queue.front();
        Queue.pop();
        //cout<<burst_time[m]<<endl;

        if(burst_time[m]>timeslice)
         nextval=nextval+timeslice;
        else
         nextval=nextval+burst_time[m];
        if(burst_time[m]>timeslice)
         burst_time[m]=burst_time[m]-timeslice;
        else
         burst_time[m]=0;
        while(i<n &&arrival_time[i]<=nextval && burst_time[i]>0 &&i!=m)
        {

                Queue.push(i);
                i++;
        }
        if(burst_time[m]>0)
          Queue.push(m);
        if(burst_time[m]<=0)
          {
            turnaround_time[m]=nextval-arrival_time[m];

          }
      }

    int total_TA_time=0;
    for (auto& n : turnaround_time)
      total_TA_time += n;
    return ((float)total_TA_time/(float)n);

}

float fcfs(int n, vector<int> burst_time,vector<int> arrival_time)
{
  vector<int> waiting_time(n,0),turnaround_time(n,0);
  loop(i,n)
        waiting_time[i] =  burst_time[i-1] + waiting_time[i-1];
  loop(i,n)
        turnaround_time[i] = burst_time[i] + waiting_time[i];
  int total_TA_time=0;
  for (auto& n : turnaround_time)
    total_TA_time += n;
  return ((float)total_TA_time/(float)n);
}

float psjf(int n, vector<int> burst_time,vector<int> arrival_time)
{
  vector<int> remaining_time(burst_time.begin(),burst_time.end()),turnaround_time(n);
  int complete = 0, t = 0, minimum = INT_MAX;
  int shortest_process = 0;

  while (complete != n)
    {
              loop(j,n)
              {
                  if ((arrival_time[j] <= t) && (remaining_time[j] < minimum) && (remaining_time[j] > 0))
                  {
                      minimum = remaining_time[j];
                      shortest_process = j;
                  }
              }

              remaining_time[shortest_process]--;
              minimum=remaining_time[shortest_process];
              if(minimum==0)
                minimum=INT_MAX;

              if (remaining_time[shortest_process] == 0)
                {
                  complete++;
                  turnaround_time[shortest_process]=(t+1)-arrival_time[shortest_process];
                }
              t++;

    }
    int total_TA_time=0;

    for (auto& n : turnaround_time)
      total_TA_time += n;
    return ((float)total_TA_time/(float)n);

}

int main()
{
  int N;
  cin>>N;
  vector<int> fcfs_atn(10),sjf_atn(10),rr_1_atn(10),rr_2_atn(10),rr_5_atn(10);

  for(int count=0;count<10;count++)
  {
      vector<int> process_ID(N),burst_time(N),arrival_time(N);

      std::random_device                  rand_dev;
      std::mt19937                        generator(rand_dev());
      std::uniform_real_distribution<> distr(0.0, 1.0);
      arrival_time[0]=0;
      for(int i=1;i<N;i++)
      {
        double U=distr(generator);

        float R=exp(-1.0*L*1.0)-(U*((exp(-1.0*L*1.0))-(exp(-1.0*L*10.0))));
        arrival_time[i]=arrival_time[i-1]+((int)(((-1.0 /  L) * (log(R)))));
      }
      std::uniform_int_distribution<int>  distr2(1,20);
      for(int i=0;i<N;i++)
      {
        int U=0;
        while(U==0)
        {
           U=distr2(generator);
        }
        burst_time[i]=U;
      }


      fcfs_atn[count]=fcfs(N,burst_time,arrival_time);

      sjf_atn[count]=psjf(N,burst_time,arrival_time);

      rr_1_atn[count]=rr(N,burst_time,arrival_time,1);
      rr_2_atn[count]=rr(N,burst_time,arrival_time,2);
      rr_5_atn[count]=rr(N,burst_time,arrival_time,5);

  }
  int n=N;

  int total_TA_time=0;
  for (auto& n :fcfs_atn)
    total_TA_time += n;
  cout<<((float)total_TA_time/(float)n)<<endl;

  total_TA_time=0;
  for (auto& n :sjf_atn)
    total_TA_time += n;
  cout<<((float)total_TA_time/(float)n)<<endl;

  total_TA_time=0;
  for (auto& n :rr_1_atn)
    total_TA_time += n;
  cout<<((float)total_TA_time/(float)n)<<endl;

  total_TA_time=0;
  for (auto& n :rr_2_atn)
    total_TA_time += n;
  cout<<((float)total_TA_time/(float)n)<<endl;

  total_TA_time=0;
  for (auto& n :rr_5_atn)
    total_TA_time += n;
  cout<<((float)total_TA_time/(float)n)<<endl;


}
