#include<stdio.h>
#include<sys/types.h>
#include<unistd.h>
#include <bits/stdc++.h>

#define tsize 4
using namespace std;

int main()
{
  int p1[2],p2[2],p3[2];
  pipe(p1);
  pipe(p2);
  pipe(p3);
  int a = fork();
  if(a==0)
    {
      int b = fork();
      if(b==0)
	{
	  int ar[100];
	  srand(123);
	  for(int i=0;i<100;i++)
	    ar[i]=rand()%1000+1;
	  
	  sort(ar,ar+100);
	  for(int i=0;i<100;i++)
	    {
	      write(p1[1],&ar[i],tsize);
	    }
	}
      else
	{
	  srand(111);
	  int ar[100];
	  for(int i=0;i<100;i++)
	    ar[i]=rand()%1000+1;
	  
	  sort(ar,ar+100);
	  for(int i=0;i<100;i++)
	    {
	      write(p2[1],&ar[i],tsize);
	    }
	}
    }
  else
    {
      int c = fork();
      if(c==0)
	{
	  srand(444);
	  int ar[100];
	  for(int i=0;i<100;i++)
	    ar[i]=rand()%1000+1;
	  
	  sort(ar,ar+100);
	  for(int i=0;i<100;i++)
	    {
	      write(p3[1],&ar[i],tsize);
	    }
	}
      else
	{
	  int a_buf[100],b_buf[100],c_buf[100];
	  int tmp1, tmp2;
	  bool initial=true;
	  int a1=0,a2=0,b1=0,b2=0,c1=0,c2=0;                           //1 compares, 2 to read into
	  
	  for(int i=0;i<300 && a1<100 && b1<100 && c1<100;i++)
	    {
	      // READING ONE NUMBER AT A TIME INTO THE THREE BUFFERS
	      if(a2<100)
		{
		  read(p1[0],&a_buf[a2],tsize);
		  a2++;
		}
	      if(b2<100)
		{
		  read(p2[0],&b_buf[b2],tsize);
		  b2++;
		}
	      if(c2<100)
		{
		  read(p3[0],&c_buf[c2],tsize);
		  c2++;
		}
	      
	      // COMPARING AND PRINTING
	      if (a_buf[a1]<=b_buf[b1]&& a_buf[a1]<=c_buf[c1])
		{
		  cout<<a_buf[a1]<<endl;
		  a1++;
		}
	      else if (b_buf[b1]<=a_buf[a1]&& b_buf[b1]<=c_buf[c1])
		{
		  cout<<b_buf[b1]<<endl;
		  b1++;
		}
	      else  if (c_buf[c1]<=a_buf[a1]&& c_buf[c1]<=b_buf[b1])
		{
		  cout<<c_buf[c1]<<endl;
		  c1++;
		}
	      
	    }
	  
	  // IF ONE ARRAY GETS EXHAUSTED BUT THE OTHER TWO ARE NOT
	  if (a1>=100)
	    {
	      if (b1<100 || c1<100)
		{
		  if (b1<100 && c1<100)
		    {
		      while (b1<100 && c1<100)
			{
			  if(b_buf[b1]<c_buf[c1])
			    {
			      cout<<b_buf[b1]<<endl;
			      b1++;
			    }
			  else
			    {
			      cout<<c_buf[c1]<<endl;
			      c1++;
			    }
			}
		    }
		  
		  if(b1<100)
		    {
		      while (b1<100)
			{
			  cout<<b_buf[b1]<<endl;
			  b1++;
			}
		    }
		  
		  if(c1<100)
		    {
		      while (c1<100)
			{
			  cout<<c_buf[c1]<<endl;
			  c1++;
			}
		    }
		}
	      
	    }
	  
	  
	  if (b1>=100)
	    {
	      if (a1<100 || c1<100)
		{
		  if (a1<100 && c1<100)
		    {
		      while (a1<100 && c1<100)
			{
			  if(a_buf[a1]<c_buf[c1])
			    {
			      cout<<a_buf[a1]<<endl;
			      a1++;
			    }
			  else
			    {
			      cout<<c_buf[c1]<<endl;
			      c1++;
			    }
			}
		    }
		  
		  if(a1<100)
		    {
		      while (a1<100)
			{
			  cout<<a_buf[a1]<<endl;
			  a1++;
			}
		    }
		  
		  if(c1<100)
		    {
		      while (c1<100)
			{
			  cout<<c_buf[c1]<<endl;
			  c1++;
			}
		    }
		}
	      
	    }
	  
	  
	  if (c1>=100)
	    {
	      if (b1<100 || a1<100)
		{
		  if (b1<100 && a1<100)
		    {
		      while (b1<100 && a1<100)
			{
			  if(b_buf[b1]<a_buf[a1])
			    {
			      cout<<b_buf[b1]<<endl;
			      b1++;
			    }
			  else
			    {
			      cout<<a_buf[a1]<<endl;
			      a1++;
			    }
			}
		    }
		  
		  if(b1<100)
		    {
		      while (b1<100)
			{
			  cout<<b_buf[b1]<<endl;
			  b1++;
			}
		    }
		  
		  if(a1<100)
		    {
		      while (a1<100)
			{
			  cout<<a_buf[a1]<<endl;
			  a1++;
			}
		    }
		}
	      
	    }
	  //for (int i=0;i<100;i++)
	  //{
	  //  cout<<a_buf[i]<<"   "<<b_buf[i]<<"   "<<c_buf[i]<<endl;
	  //}
	}
    }
  
}

int min(int a, int b)
{
  return a>b?b:a;
}
