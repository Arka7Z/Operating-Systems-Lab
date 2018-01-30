#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;

/*
Unix will make an exact copy of the parent's address space and give it to the child. Therefore, the parent and child processes have separate address spaces.
    Both processes start their execution right after the system call fork(). Since both processes have identical but separate address spaces, those variables initialized before the fork()    call have the same values in both address spaces. Since every process has its own address space, any modifications will be independent of the others. In other words, if the parent changes the value of its variable, the modification will only affect the variable in the parent process's address space. Other address spaces created by fork() calls will not be affected even though they have identical variable names.
*/

int main()
{
    int n_child_proc=3;
    int status;
    for(int i=0;i<n_child_proc;i++)
    {
        int pid=fork();
        if (pid<0)
            ;                               // ERROR
        if (pid==0)
        {
            //child process
            cout<<"CHILD PROCESS NUMBER: "<<(i+1)<<" PID: "<<getpid()<<endl;
            
            exit(0);
        }
        else            //positve pid
        {
            //pid gives the id of the child process just forked (non zero pid)
            
        }
    }
    while(wait(&status)>0)           // wait returns -1 once all child processes terminate
        ;
    cout<<" Parent terminating"<<endl;
}
