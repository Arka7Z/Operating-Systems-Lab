#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;

/*
        write( file descriptor, &(thing to write) ie pointer to object to write, sizeof(the object to write) or number of bytes to write)
        read ( file descriptor, &(thing to read into) , number of bytes to read)
*/


int main()
{
    int fd[2];              // CREATING PIPE
    pipe(fd);

    int pid=fork();
    if(pid==0)
    {
         // WRITER PROCESS

        srand(123);
        int arr[3]={1,2,3};

        close(fd[0]);                   // CLOSE UNUSED(READING END)
        for(int i=0;i<3;i++)
            write(fd[1],&arr[i],sizeof(int));
        close(fd[1]);                   // CLOSE WRITING END AFTER WRITING SO AS READ GETS THE EOF
    }
    else
    {
        // READER PROCESS

        int arr[5];
        
        int  i=0;

        close(fd[1]);                   // CLOSE UNUSED(WRITING END)
        while(int n_bytes=read(fd[0],&arr[i],sizeof(int))>0)        // READIN IN A LOOP UNTIL END
            i++;
        close(fd[0]);                   // CLOSE READING END after reading
        for(int j=0;j<i;j++)
            cout<<arr[j]<<endl;
    }
}
