#ifndef MYFS
#define MYFS


#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <string>
#include <errno.h>
#include <semaphore.h>





#define BLOCK_SIZE  256
#define MAX_NUM_BLOCK 1024*64*4 //64 mb
#define MAX_INODES 32
#define MAX_FS_SIZE 1024*1024*64


// #define SHIFT  5 // 32 = 2^5
// #define MASK  0x1F // 11111 in binary 


#define inode_bm_size MAX_INODES+1
#define data_block_bm_size MAX_NUM_BLOCK+1


using namespace std;



typedef struct
{
  int directory;
  int num_files;
  int file_size;
  int inode_num;
  // int soft_link;
  char fname[30];
  mode_t access_permission;
  int direct_pointer[8];
  char owner[30];
  int indirect_pointer;
  int doubly_ind_pointer;
  time_t last_modified;
  time_t last_read;

}inode;

typedef struct
{
  int fs_size;
  int max_inodes;
  int max_data_blocks;
  int inodes_used;
  int data_blocks_used;
  //int inode_bm[MAX_INODES/32+1];
  bitset<inode_bm_size> inode_bm;
  bitset<data_block_bm_size> data_block_bm;
  int redundant[10];

  //int data_block_bm[(MAX_NUM_BLOCK+31)/32 + 1];
  int superblock_blocks;
  int pwd ;
  int block_per_inode;
  
}superblock;


typedef struct
{
  char file_name[30];
  inode * in;
  int byte_offset;
  int mode;  // 0 read, 1 write
  
}fdt_entry;

// function prototype
int rmdir_recursive(inode *, inode* );


key_t mem_key = ftok("mrfs",'a');
int shm_id = shmget(mem_key, MAX_FS_SIZE , IPC_CREAT | 0777);




//sem_t fs_mutex;
// char *fs = (char *)malloc(MAX_FS_SIZE);
char *fs = (char *)shmat(shm_id,NULL,0);

char *file_detail = (char *)malloc(32);
char * buf = (char *) malloc(sizeof(char)*BLOCK_SIZE);
fdt_entry fdt[100];
bitset<100> fdt_bitmap;

//int last_file = -1;


void inode_setter(bitset <inode_bm_size> bset , int i ,int val)
{
  bset.set(i,val);
}
int inode_test(bitset <inode_bm_size> bset, int i)
{
  return bset.test(i);
}

int available_fd()
{
  for(int i=0;i<100;i++)
  {
    if(!fdt_bitmap.test(i))
    {
      return i;
    }
  }
  return -1;
}

void data_block_setter(bitset <data_block_bm_size> bset, int i, int val)
{
  bset.set(i,val);
}

int data_block_test(bitset <data_block_bm_size> bset, int i)
{
  return bset.test(i);
}



// void setter(int a[],int i)     {        a[i>>SHIFT] |=  (1<<(i&MASK));}
// void clr(int a[],int i)     {        a[i>>SHIFT] &= ~(1<<(i&MASK));}
// int  test(int a[],int i)    { return (a[i>>SHIFT] &   (1<<(i&MASK)));}




void print_inode_bitmap(bitset<inode_bm_size> b, int size)
{
  printf("printing bitmap , size = %d ----- \n",size);
  
  cout<<b.to_string()<<endl;
  
  printf("\n");
}

void print_data_block_bitmap(bitset <data_block_bm_size> b, int size)
{
  printf("printing data block bitmap === \n");
  for(int i=0;i<size;i++)
  {
    if(data_block_test(b,i))
      printf("%d\n",i );
  }
  printf("data block printing complete\n");
}

void pretty_permission(int permission, int directory)
{
  char str[11];
  for(int i=0;i<10;i++)
    str[i]='-';
  str[10]='\0';

  if(directory)
    str[0]='d';

  for(int i=0;i<3;i++)
  {
    int p=permission%10;
    permission/=10;
    bitset<3>bset(p);

    string s = bset.to_string();

    if(s[0]=='1')
    {
      str[i*3+1]='r';
    }
    if(s[1]=='1')
    {
      str[i*3+2]='w';
    }
    if(s[2]=='1')
    {
      str[i*3+3]='x';
    }

  }

printf("%-13s",str);

}

int get_empty_block()
{
  
  superblock* tmp = (superblock*)fs;

  for(int i=0;i<tmp->max_data_blocks;i++)
  {
    if(! data_block_test(tmp->data_block_bm,i))
      return i;
  }
  return -1;
}

int get_empty_inode()
{
  
  superblock *tmp= (superblock *)fs;

  for(int i=1;i<tmp->max_inodes;i++)
  {
    if(inode_test(tmp->inode_bm,i)==0)
      return i;
  }
  return -1;
}



void write_superblock(superblock *t)
{
  memcpy(fs,t,sizeof(superblock));
}


void print_superblock_info()
{
  superblock *t = (superblock *)fs;
  printf("-------------------------------------------------------\n");
  printf("fs_size = %d\n",t->fs_size);
  printf("max_inodes = %d \n",t->max_inodes );
  printf("max_data_blocks = %d\n",t->max_data_blocks );
  printf("inodes_used = %d\n",t->inodes_used );
  printf("data_blocks_used = %d\n",t->data_blocks_used );
  printf("superblock_blocks =%d\n",t->superblock_blocks );
  printf("blocks per inode = %d\n",t->block_per_inode );
  printf("pwd = %d\n", t->pwd );
  
  printf("-------------------------------------------------------\n");
}

int get_inode_offset(int inode)
{
  superblock * tmp = (superblock *) fs;
  int off = (tmp->superblock_blocks + inode*tmp->block_per_inode )*BLOCK_SIZE;
  return off;
}

long get_file_size(int fd)
{
  struct stat file_stat;
  int rc = fstat(fd, &file_stat);
  if(rc==0)
    return file_stat.st_size;

  printf("error in reading from file - %s\n",strerror(errno));
  return -1;
}

int get_data_block_offset(int block)
{
  superblock * tmp = (superblock *) fs;
  int off = (tmp->superblock_blocks + MAX_INODES*tmp->block_per_inode + block)*BLOCK_SIZE;
  return off;
}

int insert_file_detail(inode * pwd, int file_inode , char* file_name)
{
  if(pwd->num_files<64)
  {
    if((pwd->num_files%8)!=0)
    {
      int block_no = pwd->direct_pointer[pwd->num_files/8];
      int file_detail_offset = get_data_block_offset(block_no) + 32*(pwd->num_files%8) ;
      //char * file_detail = (char *) malloc(sizeof(char )*32);
      memcpy(file_detail,file_name,30);
      *(file_detail+30) = (short int) file_inode;
       memcpy(fs +  file_detail_offset, file_detail, 32);
       return 0;
     }
     else
     {
      printf("assigning new block while inserting file detail \n");
      int new_block = get_empty_block();
      superblock * sup = (superblock *)fs;
      sup->data_block_bm.set(new_block,1);
      sup->data_blocks_used++;
      pwd->direct_pointer[pwd->num_files/8] = new_block;

      int file_detail_offset = get_data_block_offset(new_block) + 32*(pwd->num_files%8) ;
      //char * file_detail = (char *) malloc(sizeof(char )*32);
      memcpy(file_detail,file_name,30);
      *(file_detail+30) = (short int) file_inode;
      memcpy(fs +  file_detail_offset, file_detail, 32);
      return 0;
     }

  }
  else
    return -1;

}

int get_data_block_num(inode * file_inode , int block_no) //block no is not the global block number, it is the interpreted as the ith (i>=0) block of a file
{
  int ret;
  if(block_no<8)
  {
    return file_inode->direct_pointer[block_no];
  }
  else if(block_no>=8 && block_no<72)  // indirect pointer handling
  {
    block_no-=8;
    memcpy(&ret , (fs + get_data_block_offset(file_inode->indirect_pointer) + block_no*sizeof(int) ), 4);
    return ret;
  }
  else
  {
    int db_left = block_no-72;
    int double_p = file_inode->doubly_ind_pointer;
    int idp_pos = (db_left)/64;
    // printf("idp_pos = %d\n",idp_pos);
    int idp;
    memcpy(&idp, (fs + get_data_block_offset(double_p) + sizeof(int)* idp_pos),sizeof(int));
    // printf("idp get_data_block_num = %d\n",idp);
    memcpy(&ret , (fs + get_data_block_offset(idp) +sizeof(int) * (db_left%64)), sizeof(int));
    return ret;
  }

  return -1;

}

inode * search_pwd(inode * pwd_inode, char * name)
{
  int num_db = (pwd_inode->num_files + 7)/8;
  int file_left = pwd_inode->num_files;
  inode * file_inode;
  int found =0;

  for(int i=0; i<num_db && file_left >0 ;)  /// direct pointer search
  {
    int curr_block_offset = get_data_block_offset(pwd_inode->direct_pointer[i]);

    int curr_file = 0;
    while(curr_file<8 && file_left > 0)
    {
      int file_offset=(curr_block_offset + 32* curr_file);
      memcpy(file_detail, (fs+file_offset),32);

      printf("searching -- %s \n",file_detail);
      if(strcmp(file_detail,name )==0)
      {
        found=1;
        short int * inode_num = (short int *)(file_detail + 30);
        file_inode = (inode *) (fs + get_inode_offset(*inode_num));
        break;
      }

      file_left--;
      curr_file++;
    }

    if(found)
    {
      break;
    }
    i++;
  }

  if(found)
  {
    return file_inode;
  }

  ///// TODO indirect and double indirect pointer search left


  return NULL;

}
void decrease_num_files(inode * dir_inode)
{
  int curr_num_block = (dir_inode->num_files+7)/8;
  int curr_last_block;
  superblock* sup = (superblock *) fs;

  if(curr_num_block<=8 && curr_num_block>0)
  {
    curr_last_block=dir_inode->direct_pointer[curr_num_block-1];
  }  
  else
  {
    perror("handle indirect pointer in decrease_num_files\n");
    exit(-1);
  }
  
  dir_inode->num_files--;
  
  int new_num_block = (dir_inode->num_files+7)/8;
  if(new_num_block<curr_num_block)
  {
    sup->data_block_bm.set(curr_last_block,0);
    sup->data_blocks_used--;
  }
  dir_inode->file_size = new_num_block*BLOCK_SIZE;
}

int get_file_pos(inode * dir_inode, char *filename)
{
  int num_db = (dir_inode->num_files + 7)/8;

  int file_left = dir_inode->num_files;

  int found =0;
  int file_pos = 0;
  for(int i=0; i<num_db && file_left >0 ;)  /// direct pointer search
  {
    int curr_block_offset = get_data_block_offset(dir_inode->direct_pointer[i]);

    int curr_file = 0;
    while(curr_file<8 && file_left > 0)
    {
      int file_offset=(curr_block_offset + 32* curr_file);
      memcpy(file_detail, (fs+file_offset),32);

      printf("searching -- %s \n",file_detail);
      if(strcmp(file_detail,filename )==0)
      {
        found=1;
        break;
      }

      file_pos ++;
      file_left--;
      curr_file++;
    }

    if(found)
    {
      break;
    }
    i++;
  }

  if(found)
  {
    return file_pos;
  }


  return -1;

}

char *get_file_detail_from_dir(inode * dir_inode, int pos, char * buff)   /// pos is the position where file detail is present
{
  int data_block = dir_inode->direct_pointer[pos/8];
  int offset = get_data_block_offset(data_block) + (pos%8)*32;

  memcpy(buff,(fs + offset),32);

  return buff;
 
}

void put_file_detail_to_dir(inode * dir_inode, int pos, char * buff)
{
  int data_block = dir_inode->direct_pointer[pos/8];
  int offset = get_data_block_offset(data_block) + (pos%8)*32;

  memcpy((fs+offset),buff,32);

}
int status_myfs()
{
  //sem_wait(&fs_mutex);
  superblock * sup = (superblock *)fs;
  printf("-------------------------------------------------------\n");
  printf("fs_size = %d Bytes\n",sup->fs_size);
  printf("max_inodes = %d \n",sup->max_inodes );
  printf("max_data_blocks = %d\n",sup->max_data_blocks );
  printf("inodes_used = %d\n",sup->inodes_used );
  printf("data_blocks_used = %d\n",sup->data_blocks_used );
  printf("superblock_blocks =%d\n",sup->superblock_blocks );
  printf("blocks per inode = %d\n",sup->block_per_inode );
  printf("pwd = %d\n", sup->pwd );
  
  printf("-------------------------------------------------------\n");
  
  //sem_post(&fs_mutex);
}
int rm_myfs(char *filename)
{
  //sem_wait(&fs_mutex);
  superblock * sup = (superblock *) fs;
  inode * pwd_inode = (inode *)(fs + get_inode_offset(sup->pwd));
  inode * file_inode = search_pwd(pwd_inode, filename);
  if(file_inode == NULL)
  {

    perror("file not present \n");
    //sem_post(&fs_mutex);
    return -1;
  }
  if(file_inode->directory==1)
  {
    perror("is a directory, use rmdir instead \n");
    //sem_post(&fs_mutex);
    return -1;
  }
  /// removing the data blocks taken up by file
  int num_data_blocks = (file_inode->file_size + BLOCK_SIZE -1)/BLOCK_SIZE;

  for(int i=0;i<num_data_blocks;i++)
  {
    int db = get_data_block_num(file_inode, i);
    printf("db = %d\n",db);
    sup->data_block_bm.set(db,0);
    sup->data_blocks_used--;
  }

  // removing file from inode list

  sup->inode_bm.set(file_inode->inode_num,0);
  sup->inodes_used--;

  /// removing the file inode from directory file list

  int pos = get_file_pos(pwd_inode , filename);

  printf("file pos = %d",pos);

  char tmp_detail[32];

  for(int i = pos;i < (pwd_inode->num_files -1) ;i++)
  {
    get_file_detail_from_dir(pwd_inode, i+1 , tmp_detail);

    put_file_detail_to_dir(pwd_inode, i, tmp_detail);
  }
  decrease_num_files(pwd_inode);

  return 1;
}



int rm_recursive(inode * dir_inode, char * filename)
{
  //sem_wait(&fs_mutex);
  superblock * sup = (superblock *) fs;
  inode * file_inode = search_pwd(dir_inode, filename);
  if(file_inode == NULL)
  {
    perror("file not present \n");
    //sem_post(&fs_mutex);
    return -1;
  }
  if(file_inode->directory==1 && file_inode->inode_num == dir_inode->inode_num)
  {
    perror(".. encountered\n");
    //sem_post(&fs_mutex);
    return 0;
  }
  else if(file_inode->directory==1)
  {
    rmdir_recursive(dir_inode, file_inode);
    sup->inode_bm.set(file_inode->inode_num,0);
    sup->inodes_used--;
    /// removing the file inode from directory file list

    int pos = get_file_pos(dir_inode , filename);
    printf("file pos = %d",pos);
    char tmp_detail[32];
    for(int i = pos;i < (dir_inode->num_files -1) ;i++)
    {
      get_file_detail_from_dir(dir_inode, i+1 , tmp_detail);
      put_file_detail_to_dir(dir_inode, i, tmp_detail);
    }
    decrease_num_files(dir_inode);

    //sem_post(&fs_mutex);
    return 0;
  }
  /// removing the data blocks taken up by file
  int num_data_blocks = (file_inode->file_size + BLOCK_SIZE -1)/BLOCK_SIZE;

  for(int i=0;i<num_data_blocks;i++)
  {
    int db = get_data_block_num(file_inode, i);
    sup->data_block_bm.set(db,0);
    sup->data_blocks_used--;
  }

  // removing file from inode list

  sup->inode_bm.set(file_inode->inode_num,0);
  sup->inodes_used--;

  /// removing the file inode from directory file list

  int pos = get_file_pos(dir_inode , filename);
  printf("file pos = %d",pos);
  char tmp_detail[32];
  for(int i = pos;i < (dir_inode->num_files -1) ;i++)
  {
    get_file_detail_from_dir(dir_inode, i+1 , tmp_detail);
    put_file_detail_to_dir(dir_inode, i, tmp_detail);
  }
  decrease_num_files(dir_inode);
  //sem_post(&fs_mutex);
  return 0;
}


int rmdir_recursive(inode * parent_dir_inode, inode* dir_inode)
{
  printf("inside rmdir_recursive\n");
  superblock * sup = (superblock *) fs;
  
  if(!dir_inode->directory)
  {
    perror("not a directory , use rm_myfs instead (rmdir_recursive)\n");
    //sem_post(&fs_mutex);
    return -1;
  }
  char tmp_detail[32];
  while(dir_inode->num_files>1)
  {
    get_file_detail_from_dir(dir_inode,dir_inode->num_files-1, tmp_detail);
    rm_recursive(dir_inode,tmp_detail);
  }
  sup->data_block_bm.set(dir_inode->direct_pointer[0],0);
  sup->data_block_bm.set(dir_inode->inode_num,0);
  int pos = get_file_pos(parent_dir_inode, dir_inode->fname);
  for(int i=pos;i < parent_dir_inode->num_files-1;i++)
  {
    get_file_detail_from_dir(parent_dir_inode, i+1,tmp_detail);
    put_file_detail_to_dir(parent_dir_inode, i, tmp_detail);
  }
  decrease_num_files(parent_dir_inode);
  //sem_post(&fs_mutex);
  return 1;

}


int rmdir_myfs(char *filename)
{
  //sem_wait(&fs_mutex);
  superblock * sup = (superblock *) fs;
  inode * pwd_inode = (inode *)(fs + get_inode_offset(sup->pwd));
  inode * dir_inode = search_pwd(pwd_inode, filename);  
  if(dir_inode == NULL)
  {
    perror("directory not found in present directory\n");
    //sem_post(&fs_mutex);
    return -1;
  }
  if(!dir_inode->directory)
  {
    perror("not a directory , use rm_myfs instead (rmdir_myfs)\n");
    //sem_post(&fs_mutex);
    return -1;
  }
  char tmp_detail[32];
  while(dir_inode->num_files>1)
  {
    get_file_detail_from_dir(dir_inode,dir_inode->num_files-1, tmp_detail);
    rm_recursive(dir_inode,tmp_detail);
  }
  sup->data_block_bm.set(dir_inode->direct_pointer[0],0);
  sup->data_block_bm.set(dir_inode->inode_num,0);
  int pos = get_file_pos(pwd_inode, filename);
  for(int i=pos;i < pwd_inode->num_files-1;i++)
  {
    get_file_detail_from_dir(pwd_inode, i+1,tmp_detail);
    put_file_detail_to_dir(pwd_inode, i, tmp_detail);
  }
  decrease_num_files(pwd_inode);
  //sem_post(&fs_mutex);
  return 1;
}

int dump_myfs(char *filename)
{
  //sem_wait(&fs_mutex);
  FILE *f;
  f = fopen(filename, "ab");
  superblock * sup = (superblock *) fs;
  int nwrite = fwrite(fs,sizeof(char),MAX_FS_SIZE,f);

  if(nwrite!=MAX_FS_SIZE)
  {
    fclose(f);
    perror("error in dumping \n");
    return -1;
  }
  fclose(f);
  //sem_post(&fs_mutex);
  return 1;
}

int restore_myfs(char *filename)
{
  //sem_wait(&fs_mutex);
  int fd = open(filename, O_RDONLY);
  if(fd<0)
  {
    printf("error in opening file - %s",strerror(errno));
    return -1;
  }
  int size = get_file_size(fd);

  printf("restore myfs size = %d\n",size);
  int nread = read(fd,fs,size);
  if(nread != size)
  {
    perror("error in reading from backup\n");
    return -1;
  }
  close(fd);
  //sem_post(&fs_mutex);
  return 1;
}

int open_myfs(char *filename, char mode)
{
  //sem_wait(&fs_mutex);
  superblock * sup = (superblock *) fs;
  inode * pwd_inode = (inode *) ( fs + get_inode_offset(sup->pwd));
  inode * file_inode = search_pwd(pwd_inode, filename);

  if(file_inode==NULL)
  {
    perror("file not present in present working directory\n");
    return -1;
  }
  else if(file_inode->directory)
  {
    perror("file is a directory \n");
    return -1;
  }

  int fd = available_fd();
  fdt_bitmap.set(fd,1);
  strcpy(fdt[fd].file_name, filename);
  fdt[fd].in = file_inode;
  fdt[fd].byte_offset = 0;
  fdt[fd].mode = mode;
  //sem_post(&fs_mutex);
  return fd;
}

int close_myfs(int fd)
{
  //sem_wait(&fs_mutex);
  if(!fdt_bitmap.test(fd))
  {
    perror("file not open\n");
    return -1;
  }
  fdt_bitmap.set(fd,0);
  //sem_post(&fs_mutex);
  return 1;
}

int read_myfs(int fd, int nbytes, char * buff)
{
  // TODO decide later if write mode allows read

  //sem_wait(&fs_mutex);
  if(fd > 99 || !fdt_bitmap.test(fd))
  {
    perror("invalid fd\n");
    //sem_post(&fs_mutex);
    return -1;
  }

  // if(fdt[fd].in->file_size < nbytes)
  // {
  //   perror("nbytes greater than file size\n");
  //   return -1;
  // }

  superblock * sup = (superblock*) fs;
  printf("byte offset = %d\n",fdt[fd].byte_offset);

  int start_block = (fdt[fd].byte_offset/BLOCK_SIZE);
  printf("start block = %d\n",start_block);
  
  int remaining_size = min(nbytes, (fdt[fd].in->file_size - fdt[fd].byte_offset));
  printf("remaining_size = %d\n",remaining_size);
  int end_block = (fdt[fd].byte_offset + remaining_size )/BLOCK_SIZE;
  printf("end_block=%d\n",end_block);
  int nread = 0;

  int curr_pos = fdt[fd].byte_offset;

  char tmp[BLOCK_SIZE];
  for(int i=start_block;i<=end_block; i++)
  {
    if(i==start_block)
    {
      int data_block_num = get_data_block_num(fdt[fd].in,i);
      memcpy(tmp,(fs + get_data_block_offset(data_block_num)),BLOCK_SIZE);
      int start_point = (fdt[fd].byte_offset)%BLOCK_SIZE;
      memcpy(buff,(tmp + start_point ),min(remaining_size, BLOCK_SIZE - start_point ));
      nread = min(remaining_size, BLOCK_SIZE - start_point);
      buff += nread;
      remaining_size -= nread;
      continue;

    }
    else if(i==end_block)
    {
      int data_block_num = get_data_block_num(fdt[fd].in, i);
      memcpy(tmp, (fs + get_data_block_offset(data_block_num)) , remaining_size);
      int end_point = remaining_size;
      if(remaining_size>BLOCK_SIZE)
      {
        perror("end_block problem");
        exit(-1);

      }
      memcpy(buff,tmp, end_point);
      remaining_size -= end_point;
      nread += end_point;
      continue;
    }
    else
    {
      int data_block_num = get_data_block_num(fdt[fd].in,i);
      memcpy(tmp, (fs + get_data_block_offset(data_block_num)), BLOCK_SIZE);
      memcpy(buff, tmp,BLOCK_SIZE);
      remaining_size -= BLOCK_SIZE;
      buff += BLOCK_SIZE;
      nread += BLOCK_SIZE;
    }
  }

  fdt[fd].byte_offset += nread;
  if(remaining_size==0)
  {
    //sem_post(&fs_mutex);
    return nread;
  }

  perror("remaining_size not equals 0, do math properly\n");
  //sem_post(&fs_mutex);
  return -1;
}

void write_db_to_pointers(inode * file_inode , int i, int db)
{

  superblock * sup = (superblock *) fs;

  if(i<8)
    {
      file_inode->direct_pointer[i]=db;
    }
  else if(i>=8 && i<72)
    {
      int db_used = i;
      db_used -=8;
      if(i==8)
      {
        file_inode->indirect_pointer = get_empty_block();
        sup->data_block_bm.set(file_inode->indirect_pointer,1);
        sup->data_blocks_used ++;
      }
      int offset = get_data_block_offset(file_inode->indirect_pointer) + sizeof(int)*db_used;
      memcpy((fs+offset), &db,sizeof(int));
    }
    // else
    // {
    //   int db_used = i;
    //   db_used -= 72;

    //   if(i==72)
    //   {
    //     file_inode->doubly_ind_pointer = get_empty_block();
    //     sup->data_block_bm.set(file_inode->doubly_ind_pointer,1);
    //     sup->data_blocks_used ++;
    //   }

    //   if(db_used%64 == 0)
    //   {
    //     int new_block = get_empty_block();
    //     sup->data_block_bm.set(new_block,1);
    //     sup->data_blocks_used ++;

    //     memcpy((fs + get_data_block_offset(file_inode->doubly_ind_pointer) + sizeof(int)*(db_used%64)),&new_block,sizeof(int));

    //   }

    //   int double_p_offset = get_data_block_offset(file_inode->doubly_ind_pointer);
    //   int ind_p;
    //   memcpy(&ind_p, (fs + double_p_offset + sizeof(int)*(db_used%64)),sizeof(int) );




        // else handle double indirect pointers
    // }
}

int write_myfs(int fd, int nbytes, char * buff)
{
  //sem_wait(&fs_mutex);
  if(fd > 99 || !fdt_bitmap.test(fd))
  {
    perror("invalid file descriptor \n");
    //sem_post(&fs_mutex);
    return -1;
  }
  if(fdt[fd].in->directory)
  {
    perror("is a directory \n");
    //sem_post(&fs_mutex);
    return -1;
  }
  superblock * sup = (superblock *) fs;
  // TODO for first write call, delete the data blocks first, for now just assigning new blocks

  if(fdt[fd].byte_offset==0)
  {
    fdt[fd].in->file_size = 0;
  }
  int start_block = (fdt[fd].byte_offset/BLOCK_SIZE);
  // printf("start block = %d\n",start_block);
  int remaining_size = nbytes;
  // printf("remaining_size = %d\n",remaining_size);
  int end_block = (fdt[fd].byte_offset + remaining_size )/BLOCK_SIZE;
  // printf("end_block=%d\n",end_block);
  int nwritten=0;
  char tmp_buf[BLOCK_SIZE];

  for(int i=start_block; i <= end_block; i++)
  {
    if(i==start_block)
    {
      if(fdt[fd].byte_offset%BLOCK_SIZE == 0) /// new block required
      {
        int db = get_empty_block();
        sup->data_block_bm.set(db,1);
        sup->data_blocks_used ++;
        write_db_to_pointers(fdt[fd].in, i, db);
        // printf("write_db_to_pointers = %d\n",db);
      }
      int db = get_data_block_num(fdt[fd].in, start_block);
      // printf("get_data_block_num returns = %d\n",db);
      memcpy(tmp_buf,(fs + get_data_block_offset(db)), BLOCK_SIZE);
      // printf("tmp_buf contains --- %s\n",tmp_buf);
      memcpy(tmp_buf+(fdt[fd].byte_offset % BLOCK_SIZE),buff + 0,min(nbytes,(BLOCK_SIZE - (fdt[fd].byte_offset % BLOCK_SIZE)))); // +0 since this is the starting block
      // printf("modified tmp_buf -- %s\n",tmp_buf);
      memcpy((fs + get_data_block_offset(db)), tmp_buf, BLOCK_SIZE);

      nwritten += min(nbytes,(BLOCK_SIZE - (fdt[fd].byte_offset % BLOCK_SIZE)));
      remaining_size -= nwritten;
      fdt[fd].byte_offset += nwritten;
      fdt[fd].in->file_size += nwritten;
    }
    else
    {
      int db = get_empty_block();
      sup->data_block_bm.set(db,1);
      sup->data_blocks_used ++;
      write_db_to_pointers(fdt[fd].in, i, db);
      int to_be_written = min(BLOCK_SIZE, remaining_size);
      memcpy((fs + get_data_block_offset(db)), buff+nwritten, to_be_written);
      nwritten += to_be_written;
      remaining_size -= to_be_written;
      fdt[fd].in->file_size += to_be_written;

    }
  }
  time_t curr_time;
  time(&curr_time);

  fdt[fd].in->last_modified = curr_time;
  // printf("remaining_size = %d\n", remaining_size);
  // printf("nbytes = %d\n",nbytes );
  // printf("nwritten = %d\n",nwritten);
  //sem_post(&fs_mutex);
  return nwritten;
}

int eof_myfs(int fd)
{
  //sem_wait(&fs_mutex);
  if(!fdt_bitmap.test(fd))
  {
    perror("invalid file descriptor \n");
    //sem_post(&fs_mutex);
    return -1;
  }
  if(fdt[fd].in->file_size == fdt[fd].byte_offset)
  {
    //sem_post(&fs_mutex);
    return 1;
  }
  else if(fdt[fd].in->file_size > fdt[fd].byte_offset)
  {
    //sem_post(&fs_mutex);
    return 0;
  }
  else
  {
    perror("error in  eof_myfs\n");
    //sem_post(&fs_mutex);
    exit(-1);
  }
}

int copy_pc2myfs(char *source, char *dest)
{
  //sem_wait(&fs_mutex);
  superblock * sup = (superblock *) fs;
  inode * pwd_inode = (inode *) (fs + get_inode_offset (sup->pwd));

  int new_size =0;
  if(search_pwd(pwd_inode,dest)!=NULL)
  {
    perror("file with same name already present \n");
    return -1;
  }

  int fd = open(source, O_RDONLY);
  
  if(fd==-1)
  {
    //sem_post(&fs_mutex);
    return -1;
  }

  int new_inode = get_empty_inode();
  if(new_inode==-1)
  {
    //sem_post(&fs_mutex);
    return -1;
  }

  sup->inode_bm.set(new_inode,1);
  sup->inodes_used ++;

  int file_size = get_file_size(fd);

  if(file_size==-1)
  {
    //sem_post(&fs_mutex);
    return -1;
  }

  inode * file_inode = (inode *)(fs + get_inode_offset(new_inode));
  file_inode->inode_num = new_inode;

  //printf("testing 0th datablock = %d\n",test(sup->data_block_bm,0));
  int no_blocks = (file_size + BLOCK_SIZE -1 )/BLOCK_SIZE ;
  int remaining_size = file_size;
  

  insert_file_detail(pwd_inode,new_inode,dest);

  pwd_inode->num_files++;

  file_inode->file_size = file_size;
  strcpy(file_inode->owner,"basher666");
  file_inode->access_permission = 777;
  file_inode->directory = 0;
  file_inode->num_files=0;

  printf("file size = %d\n",file_size);



  int dp=0;
  while(remaining_size>0 && dp<8)  // filling the direct pointers
  {
    int new_block = get_empty_block();
    //setter(sup->data_block_bm,new_block);
    sup->data_block_bm.set(new_block,1);
    sup->data_blocks_used++;

    file_inode->direct_pointer[dp] = new_block;
    int block_offset = get_data_block_offset(new_block);

    int read_size;
    if((read_size = read(fd,buf,min(remaining_size, BLOCK_SIZE))) <=0)
    {
      perror("read problem in copy_pc2myfs\n");
      //sem_post(&fs_mutex);
      return -1;
    }
    new_size += read_size;
    remaining_size-=read_size;

    printf("new_block =%d\n",new_block);
    memcpy(fs+block_offset,buf,read_size);
    
    dp++;

    
  }



  //// indirect pointer handling
  if(remaining_size>0)
  {
    int idp_block = get_empty_block();
    sup->data_block_bm.set(idp_block,1);
    sup->data_blocks_used++;

    file_inode->indirect_pointer = idp_block;


    int no_direct_blocks = BLOCK_SIZE/sizeof(int);

    int idp = 0;
    
    int idp_block_offset = get_data_block_offset(idp_block);
    int nread;

    while(remaining_size>0 && idp<no_direct_blocks)
    {

      int dp_block = get_empty_block();
      sup->data_block_bm.set(dp_block,1);
      sup->data_blocks_used++;
      
      memcpy((fs + idp_block_offset + sizeof(int)*idp ),&dp_block,sizeof(int));

      if((nread = read(fd,buf,min(remaining_size, BLOCK_SIZE))) <=0)
      {
        perror("read problem in copy_pc2myfs\n");
        return -1;
      }
      int dp_block_offset = get_data_block_offset(dp_block);
      memcpy((fs+dp_block_offset), buf, nread);
      // write(1,buf, nread);
      // fflush(stdout);

      new_size += nread;
      remaining_size-=nread;
      idp++;
    }


  }



  if(remaining_size>0)    // double indirect pointer filling
  {

    int d_ind_block = get_empty_block();
    sup->data_block_bm.set(d_ind_block,1);
    sup->data_blocks_used++;

    file_inode->doubly_ind_pointer = d_ind_block;

    int more_blocks_req = (remaining_size + BLOCK_SIZE -1)/BLOCK_SIZE;

    int curr_idp_block_pos = 0;
    while(more_blocks_req>0 && remaining_size >0 && curr_idp_block_pos < 64)
    {
      int idp_block = get_empty_block();
      sup->data_block_bm.set(idp_block,1);
      sup->data_blocks_used++;
      memcpy((fs + get_data_block_offset(d_ind_block) + sizeof(int)*curr_idp_block_pos), &idp_block,sizeof(int));
      int nread;
      int curr_dp_pos = 0;
      // printf("idp_block = %d\n",idp_block);
      // status_myfs();
      while(remaining_size > 0 && curr_dp_pos < 64 && more_blocks_req >0)
      {
        int dp_block = get_empty_block();
        more_blocks_req --;
        sup->data_block_bm.set(dp_block,1);
        sup->data_blocks_used++;

        // printf("dp_block = %d\n",dp_block);

        memcpy((fs + get_data_block_offset(idp_block) + sizeof(int)*curr_dp_pos),&dp_block, sizeof(int));

        if((nread = read(fd,buf,min(remaining_size, BLOCK_SIZE))) <=0)
        {
          perror("read problem in copy_pc2myfs\n");
          return -1;
        }
        // printf("nread=%d\n",nread);
        // write(1,buf, nread);copy_pc
        // fflush(stdout);

        memcpy((fs + get_data_block_offset(dp_block) ),buf,nread);
        
        new_size += nread;
        remaining_size -=nread;
        curr_dp_pos ++;
      }
      curr_idp_block_pos ++;
    }    

  }


  if(remaining_size>0)
  {
    perror("not possible to store in the possible fs\n");
    return -1;
  }

  time_t curr_time;
  time(&curr_time);



  file_inode->last_read = curr_time;
  file_inode->last_modified = curr_time;

  close(fd);

  printf("new size = %d\n",new_size);

  pwd_inode->file_size = ((pwd_inode->num_files+7)/8)*BLOCK_SIZE;
  //sem_post(&fs_mutex);
  return EXIT_SUCCESS;
}


int chdir_myfs(char * dirname)
{
  //sem_wait(&fs_mutex);
  superblock * sup = (superblock *)fs;
  inode * pwd_inode = (inode *) (fs + get_inode_offset(sup->pwd));
  inode * dir_inode = search_pwd(pwd_inode, dirname);

  if(dir_inode==NULL)
  {
    perror("directory not present\n");
    return -1;
  }
  if(dir_inode->directory!=1)
  {
    perror("not a directory\n");
    return -1;
  }

  sup->pwd = dir_inode->inode_num;

  time_t curr_time;
  time(&curr_time);
  dir_inode->last_read = curr_time;

  //sem_post(&fs_mutex);
  return EXIT_SUCCESS;

}

int mkdir_myfs(char *dirname)
{
  //sem_wait(&fs_mutex);
  superblock * sup = (superblock *) fs;
  inode * pwd_inode = (inode *) (fs + get_inode_offset(sup->pwd));

  int dir_inode_num = get_empty_inode();
  sup->inodes_used++;
  insert_file_detail(pwd_inode, dir_inode_num, dirname);
  pwd_inode->num_files++;
  sup->inode_bm.set(dir_inode_num);

  inode * dir_inode = (inode *)(fs + get_inode_offset(dir_inode_num));
  dir_inode->inode_num = dir_inode_num;

  dir_inode->directory = 1;
  dir_inode->num_files = 0;
  dir_inode->access_permission = 666;
  strcpy(dir_inode->fname, dirname);
  strcpy(dir_inode->owner, "basher666");

  dir_inode->file_size = BLOCK_SIZE ; 
  // int dir_data_block = get_empty_block();
  // sup->data_block_bm.set(dir_data_block,1);
  // sup->data_blocks_used++;
  // dir_inode->direct_pointer[0] = dir_data_block;

  char parent_dir[30];
  strcpy(parent_dir,"..");
  insert_file_detail(dir_inode, sup->pwd , parent_dir);
  dir_inode->num_files++;

  time_t curr_time;
  time(&curr_time);

  dir_inode->last_modified = curr_time;
  dir_inode->last_read = curr_time;

  pwd_inode->file_size = ((pwd_inode->num_files+7)/8)*BLOCK_SIZE;

  //sem_post(&fs_mutex);
  return EXIT_SUCCESS;


}

int showfile_myfs( char * filename)
{
  //sem_wait(&fs_mutex);
  superblock * sup = (superblock *)fs;
  inode * pwd_inode = (inode *)(fs + get_inode_offset(sup->pwd));

  inode * file_inode = search_pwd(pwd_inode, filename);



  if(file_inode==NULL)
  {
    perror("file not present in the present working directory \n");
    //sem_post(&fs_mutex);
    return -1;
  }

  if(file_inode->directory)
  {
    perror("entered file name is a directory\n");
    //sem_post(&fs_mutex);
    return -1;
  }


  int dp = 0;

  int remaining_size = file_inode->file_size;
  while(remaining_size>0 && dp<8)
  {
    int block_offset = get_data_block_offset(file_inode->direct_pointer[dp]);
   
    int nread = min(remaining_size,BLOCK_SIZE);
    memcpy(buf,(fs + block_offset), nread);
    
    write(1,buf, nread);
    fflush(stdout);
    remaining_size -= nread;
    dp++;
  }


  if(remaining_size>0)
  {
      int idp_block = file_inode->indirect_pointer;


      int no_direct_blocks = BLOCK_SIZE/sizeof(int);

      int idp = 0;
      
      int idp_block_offset = get_data_block_offset(idp_block);
      
      int nread;

      while(remaining_size>0 && idp<no_direct_blocks)
      {
        int dp_block;

        memcpy(&dp_block,(fs + idp_block_offset + sizeof(int)*idp ),sizeof(int));
        int dp_block_offset = get_data_block_offset(dp_block);

        int nread = min(remaining_size, BLOCK_SIZE);
        memcpy(buf, (fs + dp_block_offset) , nread );
        write(1, buf, nread);
        fflush(stdout);
        remaining_size-=nread;
        idp++;
      }


  }


  if(remaining_size>0) // double indirect pointers
  {
    int d_ind_block = file_inode->doubly_ind_pointer;

    int more_blocks_req = (remaining_size + BLOCK_SIZE -1)/BLOCK_SIZE;

    int curr_idp_block_pos = 0;
    while(more_blocks_req>0 && remaining_size >0 && curr_idp_block_pos < 64)
    {
      int idp_block;
      memcpy(&idp_block,(fs + get_data_block_offset(d_ind_block) + sizeof(int)*curr_idp_block_pos),sizeof(int));
      printf("idp_block=%d\n",idp_block);
      int curr_dp_pos = 0;
      while(remaining_size > 0 && curr_dp_pos < 64 && more_blocks_req >0)
      {
        int dp_block;
        more_blocks_req --;
        memcpy(&dp_block,(fs + get_data_block_offset(idp_block) + sizeof(int)*curr_dp_pos), sizeof(int));
        printf("dp_block=%d\n",dp_block);
        int nread = min(remaining_size, BLOCK_SIZE);
        memcpy(buf, (fs + get_data_block_offset(dp_block)) , nread);

        write(1, buf, nread);
        fflush(stdout);
        
        remaining_size -= nread;
        curr_dp_pos ++;
      }
      curr_idp_block_pos ++;
    }    
  }


  if(remaining_size>0)
  {
    perror("not possible in current fs\n");
    return (-1);
  }

  //// TODO double indirect pointer required

  return 1;
//sem_post(&fs_mutex);

}

int copy_myfs2pc (char *source, char *dest)
{
  superblock * sup = (superblock *) fs;  

  inode * pwd_inode = (inode *) ( fs + get_inode_offset(sup->pwd));
  inode * file_inode = search_pwd(pwd_inode, source);
  if(file_inode==NULL)
  {
    perror("file not found in current directory \n");
    return -1;
  }

  printf("file found in pwd \n");
  FILE *f;
  f = fopen(dest, "ab");

  /// direct pointers 

  int dp = 0;

  int remaining_size = file_inode->file_size;

  printf("file_size again = %d\n",remaining_size);
  while(remaining_size>0 && dp<8)
  {
    int block_offset = get_data_block_offset(file_inode->direct_pointer[dp]);
    int nwrite = fwrite((fs+block_offset), sizeof(char), min(remaining_size, BLOCK_SIZE),f);
    remaining_size-=nwrite;
    dp++;
  }



  /// indirect pointer handling

  if(remaining_size>0)
  {
      int idp_block = file_inode->indirect_pointer;


      int no_direct_blocks = BLOCK_SIZE/sizeof(int);

      int idp = 0;
      
      int idp_block_offset = get_data_block_offset(idp_block);
      int nwrite;
      while(remaining_size>0 && idp<no_direct_blocks)
      {

        int dp_block;
        
        memcpy(&dp_block,(fs + idp_block_offset + sizeof(int)*idp ),sizeof(int));

        int dp_block_offset = get_data_block_offset(dp_block);

        nwrite = fwrite((fs + dp_block_offset ), sizeof(char ), min(remaining_size, BLOCK_SIZE), f);

        remaining_size-=nwrite;
        idp++;
      }


  }



if(remaining_size>0) // double indirect pointers
  {
    int d_ind_block = file_inode->doubly_ind_pointer;

    int more_blocks_req = (remaining_size + BLOCK_SIZE -1)/BLOCK_SIZE;

    int curr_idp_block_pos = 0;
    while(more_blocks_req>0 && remaining_size >0 && curr_idp_block_pos < 64)
    {
      int idp_block;
      memcpy(&idp_block,(fs + get_data_block_offset(d_ind_block) + sizeof(int)*curr_idp_block_pos),sizeof(int));
      printf("idp_block=%d\n",idp_block);
      int curr_dp_pos = 0;
      while(remaining_size > 0 && curr_dp_pos < 64 && more_blocks_req >0)
      {
        int dp_block;
        more_blocks_req --;
        memcpy(&dp_block,(fs + get_data_block_offset(idp_block) + sizeof(int)*curr_dp_pos), sizeof(int));
        printf("dp_block=%d\n",dp_block);
        int nwrite = min(remaining_size, BLOCK_SIZE);
        memcpy(buf, (fs + get_data_block_offset(dp_block)) , nwrite);

        nwrite = fwrite((fs + get_data_block_offset(dp_block)), sizeof(char ), min(remaining_size, BLOCK_SIZE), f);
        
        remaining_size -= nwrite;
        curr_dp_pos ++;
      }
      curr_idp_block_pos ++;
    }    
  }





  // if(remaining_size>0) // double indirect pointers
  // {
  //   int d_ind_block = get_empty_block();
  //   sup->data_block_bm.set(d_ind_block,1);
  //   sup->data_blocks_used++;

  //   file_inode->doubly_ind_pointer = d_ind_block;

  //   int more_blocks_req = (remaining_size + BLOCK_SIZE -1)/BLOCK_SIZE;

  //   int curr_idp_block_pos = 0;
  //   while(more_blocks_req>0 && remaining_size >0 && curr_idp_block_pos < 64)
  //   {
  //     int idp_block = get_empty_block();
  //     sup->data_block_bm.set(d_ind_block,1);
  //     sup->data_blocks_used++;
  //     memcpy((fs + get_data_block_offset(file_inode->doubly_ind_pointer) + sizeof(int)*curr_idp_block_pos),&idp_block,sizeof(int));
  //     int nwrite;
  //     int curr_dp_pos = 0;
  //     while(remaining_size > 0 && curr_dp_pos < 64)
  //     {
  //       int dp_block = get_empty_block();
  //       more_blocks_req --;
  //       sup->data_block_bm.set(dp_block,1);
  //       sup->data_blocks_used++;

  //       memcpy((fs + get_data_block_offset(idp_block) + sizeof(int)*curr_dp_pos),&dp_block, sizeof(int));


  //       nwrite = fwrite((fs + get_data_block_offset(dp_block)), sizeof(char ), min(remaining_size, BLOCK_SIZE), f);

  //       if(nwrite <=0)
  //       {
  //         perror("read problem in copy_myfs2pc\n");
  //         return -1;
  //       }

  //       remaining_size -= nwrite;
  //       curr_dp_pos ++;
  //     }
  //     curr_idp_block_pos ++;
  //   }    
  // }
  if(remaining_size>0)
  {
    perror("not possible to store in current fs\n");
    return -1;
  }


  time_t curr_time;
  time(&curr_time);
  file_inode->last_read = curr_time;


  fclose(f);
}



int ls_myfs()
{

  superblock * tmp = (superblock *) fs;
  int pwd=tmp->pwd;
  inode * pwd_inode;
  print_superblock_info();

  int pwd_inode_offset = get_inode_offset(pwd) ;

  pwd_inode = (inode *)(fs + pwd_inode_offset);

  printf("%s\n",pwd_inode->owner);
  
  int name_offset = BLOCK_SIZE*(tmp->superblock_blocks + MAX_INODES + pwd_inode->direct_pointer[0]);
  
  char *name = (char *) malloc(30);
  
  // printf("hahah\n");

  print_data_block_bitmap(tmp->data_block_bm, tmp->max_data_blocks);

  
  memcpy(name, fs+name_offset , 30);
  
  printf(" current directory -- %s \t total files = %d\n",pwd_inode->fname,pwd_inode->num_files);

  cout <<setfill('-') << setw(35)  << endl ;
  int remaining_files = pwd_inode->num_files;

  int block_it = 0;

  printf("%-13s", "permission");
  printf("%-20s","owner");
  printf("%-15s","size");
  printf("%-26s","modified" );
  printf("%-30s", "name");
  cout <<endl;
  cout <<"---------------------------------------------------------------------------------------------"<<endl;


  for(;remaining_files>0;)
  {

    int block_offset = get_data_block_offset(pwd_inode->direct_pointer[block_it]);
    for(int curr_file = 0;remaining_files>0 && curr_file< 8;)
    {
      memset(name,0,sizeof(name));


      int file_offset = block_offset + curr_file*32;

      // printf("curr_file = %d\n",curr_file);

      memcpy(name,fs+file_offset,30);

      short int *file_inode_num = (short int *) (fs+file_offset + 30);


      inode * file_inode = (inode *)(fs + get_inode_offset(*file_inode_num));

      tm * ptm = std::localtime(&(file_inode->last_modified));
      char time_buf[26];
      strftime(time_buf, 26, "%a, %d.%m.%Y %H:%M:%S", ptm);

      pretty_permission(file_inode->access_permission, file_inode->directory );
      printf("%-20s",file_inode->owner);
      printf("%-15d", file_inode->file_size);
      printf("%-26s",time_buf);
      printf("%-30s\n",name);

      remaining_files--;
      curr_file++;
    }
    if(remaining_files<=0)
      break;

    block_it++;

  }



  free(name);


}


int create_myfs(int mb_size)
{
  int size = mb_size;
  size=size*1024*1024;

  //fs=(char *)malloc(sizeof(size));
  
  if(fs==NULL)
    return -1;

  superblock t;// = (superblock *)fs;

  t.fs_size = size;

  t.max_inodes = MAX_INODES;

  t.max_data_blocks = ceil(size/256.0) - ceil(sizeof(superblock)/256.0) - MAX_INODES*(ceil(sizeof(inode)/256.0))-1;

  //t.inode_bm = (int *)malloc((((MAX_INODES+31)/32)+10)*sizeof(int));

  //t.data_block_bm = (int *) malloc(sizeof(int)*((MAX_NUM_BLOCK+31)/32 + 10));

  t.block_per_inode = (int) ceil(sizeof(inode)/256.0);
  // t.inode_bm = (int * )malloc(sizeof(int)*t.max_inodes);

  // t.data_block_bm = (int *)malloc(sizeof(int)* t.max_data_blocks);

  t.data_blocks_used = 0;

  t.inodes_used = 0;
  t.pwd = 0;
  t.superblock_blocks = (int) ceil(sizeof(superblock)/256.0);

  //t.inode_bm = create_bitvec(t.max_inodes);

  //printf("t.inode_bm->size = %d\n\n",t.inode_bm.size);
  //t.data_block_bm = create_bitvec(t.max_data_blocks);


  for(int i=0;i<t.max_inodes;i++)
  {
    t.inode_bm.set(i,0);
    //inode_setter(t.inode_bm,i,0);
  }

  // print_bitmap(t.inode_bm,t.max_inodes);

  for(int i=0;i<t.max_data_blocks;i++)
  {
    t.data_block_bm.set(i,0);
    // data_block_setter(t.data_block_bm,i,0);
  }

  // print_data_block_bitmap(t.data_block_bm,t.max_data_blocks);

  // printf("bbeb\n");
  

  write_superblock(&t);

  
  // printf("sizeof(superblock)=%d\n",sizeof(superblock));



  // printf("writing to superblock done\n");
  superblock *tt = (superblock *) fs;

  // print_superblock_info();


  inode root;
  strcpy(root.fname,"root");
  strcpy(root.owner,"root");
  root.access_permission = 777;
  root.directory = 1;
  root.num_files=0;
  root.file_size = BLOCK_SIZE;


  int block_pos = get_empty_block();


  // printf("block_pos = %d \n", block_pos);

  // printf("superblock size = %d",sizeof(t));
  // printf("superblock_blocks = %d",t.superblock_blocks);
  int offset = (t.superblock_blocks + MAX_INODES + block_pos) * BLOCK_SIZE + 32*root.num_files;
  // printf("root self file offset = %d\n",offset );
  // printf("fs size = %d\n",size);
  char * dp = fs + offset;


  strcpy(file_detail,"/.");
  short int  *inode_ptr = (short int *)(file_detail + 30);

  *inode_ptr =(short int) 0;
  

  

  memcpy(dp,file_detail,sizeof(file_detail));
  root.num_files ++;

  // printf("sizeof (inode ) = %d\n",sizeof(inode) );
  for(int i=0;i<8;i++)
  {
    root.direct_pointer[i]=-1;
  }

  root.direct_pointer[0]=block_pos;

  time_t curr_time;
  time(&curr_time);

  root.last_modified = curr_time;
  root.last_read = curr_time;
  root.inode_num = 0;


  memcpy(fs + t.superblock_blocks*BLOCK_SIZE ,&root,sizeof(root));




  superblock * sb_ptr = (superblock *) fs;

  sb_ptr->inodes_used ++;
  sb_ptr->data_blocks_used += 1;
  // print_superblock_info();
  // printf("block_pos = %d\n",block_pos);
  
  //data_block_setter(sb_ptr->data_block_bm,block_pos,1);
  //inode_setter(sb_ptr->inode_bm,0,1);
  sb_ptr->data_block_bm.set(block_pos,1);
  sb_ptr->inode_bm.set(0,1);
  // print_superblock_info();
  // printf("printing inode_bm \n");
  // print_inode_bitmap(sb_ptr->inode_bm,sb_ptr->max_inodes);
  // printf("printing data_block_bm\n");
  // print_data_block_bitmap(sb_ptr->data_block_bm, sb_ptr->max_data_blocks);

  // free(file_detail);
  return EXIT_SUCCESS;

}

int chmod_myfs(char *filename , int mode)
{
  superblock * sup = (superblock *) fs;
  inode * pwd_inode = (inode *)( fs + get_inode_offset(sup->pwd));
  inode * file_inode = search_pwd(pwd_inode, filename);
  if(file_inode == NULL)
  {
    perror("file/directory not present in present working directory\n");
    return -1;
  }

  file_inode->access_permission = mode;
  return 1;
}

int touch_myfs(char * filename)
{
  superblock * sup = (superblock *) fs;
  inode * pwd_inode = (inode *)( fs + get_inode_offset(sup->pwd));
  inode * file_inode = search_pwd(pwd_inode, filename);

  if(file_inode != NULL)
  {
    printf("file present \n");
    return 1;
  }

  int new_inode = get_empty_inode();
  printf("new inode = %d",new_inode);
  insert_file_detail(pwd_inode,new_inode,filename);
  sup->inode_bm.set(new_inode,1);
  sup->inodes_used ++;
  file_inode = (inode *)(fs + get_inode_offset(new_inode));
  file_inode->inode_num = new_inode;
  file_inode->num_files = 1;
  file_inode->file_size = 0;
  file_inode->access_permission = 666;
  file_inode->directory =0;
  pwd_inode->num_files++;
  time_t curr_time;
  time(&curr_time);
  file_inode->last_modified = curr_time;
  file_inode->last_read = curr_time;

  strcpy(file_inode->owner,"basher666");
  
  return 1;

}

void test_case1()
{
  char fname[30];
  char dstname[30];
  strcpy(fname,"makefile");
  strcpy(dstname,"make0");

  ls_myfs();
  for(int i=0;i<14;i++)
  {
    copy_pc2myfs(fname,dstname);
    if(i<10)
      dstname[4]=(char)(i+(int)'0');
    else
    {
      dstname[4]='1';
      dstname[5]=(char)(i%10+(int)'0');
      dstname[6]='\0';
    }
  }
  ls_myfs();
  printf("enter file to delete \n");
  scanf("%s",fname);
  rm_myfs(fname);
  ls_myfs();
}
int int_to_char(int n,char *buf)
{
  int i=0;
  while(n>0)
  {
    buf[i] =(char)((n%10) + '0');
    n /= 10;
    i++;
  }
  buf[i]=',';
  i++;
  return i;
}
void test_case2()
{
  char fname[30];
  char mytext[30];
  strcpy(fname,"mytext.txt");
  strcpy(mytext,"mytext.txt");
  touch_myfs(mytext);
  // printf("touch done\n");
  // return;
  int fd = open_myfs(mytext,'w');
  int fd2;
  char tmp_buf[10];
  int N;

  printf("enter N\n");
  scanf("%d",&N); 

  int fsize=0;
    for(int i=0;i<100;i++)
    {
      int r = rand()%1000007;
      int dig = int_to_char(r,tmp_buf);
      // printf("generated = %d\n",r);

      int nwrite = write_myfs(fd,dig,tmp_buf);
      // printf("current file size = %d\n",fdt[fd].in->file_size);
      // printf("dig = %d\n", dig);
      fsize += nwrite;
    }
  close_myfs(fd);
  // printf("fsize = %d\n",fsize);
  char buffer[fsize+100];
  for(int i=0;i<N;i++)
  {
    sprintf(fname,"mytext-%d.txt",i+1);
    touch_myfs(fname);
    fd2 = open_myfs(fname,'w');
    fd = open_myfs(mytext,'r');
    int nread =0;
    if((nread =read_myfs(fd,fsize,buffer))<0)
      {
        perror("error in reading mytext.txt\n");
        exit(-1);
      }
    // printf("nread = %d\n", nread);
    write_myfs(fd2,nread,buffer);
    close_myfs(fd2);
    close_myfs(fd);
  }

  char dump_name[30];
  strcpy(dump_name,"mydump-28.backup");
  dump_myfs(dump_name);
  
}

void test_case3()
{
  char mytext[30];
  char sorted[30];
  char backup_name[30];
  strcpy(mytext,"mytext.txt");
  strcpy(sorted,"sorted.txt");
  strcpy(backup_name, "mydump-28.backup");
  restore_myfs(backup_name);
  ls_myfs();
  
  char * token;
  int fd = open_myfs(mytext,'r');
  int fsize = fdt[fd].in->file_size + 1;
  char buffer[fsize];
  read_myfs(fd,fsize,buffer);
  close_myfs(fd);
  buffer[fsize-1]='\0';

  token = strtok(buffer,",");

  int ar[100];
  int i=0;
  while(token!=NULL)
  {
    int num = atoi(token);
    ar[i++]=num;
    token = strtok(NULL,",");
  }
  sort(ar,ar+i);
  char num_buf[10];
  touch_myfs(sorted);
  fd = open_myfs(sorted,'w');
  for(int j=0;j<i;j++)
  {
    int dig = int_to_char(ar[j],num_buf);
    write_myfs(fd,dig,num_buf);

  }
  close_myfs(fd);


}



int check()
{
  mode_t mode=0777;
  int size_mb=10;
  char * file_name=(char *)malloc(30*sizeof(char));
  char * dest_name =(char *)malloc(30*sizeof(char));
  //sem_init(&fs_mutex, 0, 1);
  // for(int i=0;i<100;i++)
    // fdt_bitmap.set(i,0);

  int choice;
  while(1)
  {
    cout<<"1-->create_myfs\n2-->ls_myfs\n3-->copy_pc2myfs\n4-->copy_myfs2pc\n5-->mkdir_myfs\n6-->chdir_myfs\n7-->showfile_myfs\n8-->open file in file system\n9-->read from file system\n10-->remove file\n11-->dump file system\n12-->restore file system\n13-->remove directory from pwd\n14-->test case 1\n15-->exit\n16-->status of file system\n17-->test case 2\n18-->test case 3\n19-->chmod_myfs"<<endl;
    scanf("%d",&choice);
    int fd;
    switch(choice)
    {
      case 1: create_myfs(size_mb);
              printf(" MRFS of size %d MB created ------\n",size_mb);
              break;
      case 2: ls_myfs();
              break;
      case 3: printf("enter source file name\n");
              scanf("%s",file_name);
              printf("enter destination file name\n");
              scanf("%s",dest_name);
              copy_pc2myfs(file_name,dest_name);
              break;
      case 4: printf("enter source file name\n");
              scanf("%s",file_name);
              printf("enter destination file name\n");
              scanf("%s",dest_name);
              copy_myfs2pc(file_name,dest_name);
              break;
      case 5: printf("enter directory name \n");
              scanf("%s",file_name);
              mkdir_myfs(file_name);
              break;
      case 6: printf("enter directory name \n");
              scanf("%s",file_name);
              chdir_myfs(file_name);
              break;
      case 7: printf("enter text file name \n");
              scanf("%s",file_name);
              showfile_myfs(file_name);
              break;
      case 8:   printf("enter file name \n");
                scanf("%s",file_name);
                fd = open_myfs(file_name,'r');
                break;

      case 9:   printf("10 bytes of makefile\n");
                char buff[100];
                read_myfs(fd,100,buff);
                printf("%s\n",buff);
                break;
      case 10:  printf("enter name of file \n");
                scanf("%s",file_name);
                rm_myfs(file_name);
                break;
      case 11:  printf("enter dump file name\n");
                scanf("%s",file_name);
                dump_myfs(file_name);
                break;
      case 12:  printf("enter backup file to restore\n");
                scanf("%s",file_name);
                restore_myfs(file_name);
                break;
      case 13:  printf("enter directory name to delete\n");
                scanf("%s",file_name);
                rmdir_myfs(file_name);
                break;
      case 14:  test_case1();
                break;
      case 15:  exit(EXIT_SUCCESS);
      case 16:  status_myfs();
                break;
      case 17:  test_case2();
                break;
      case 18:  test_case3();
                break;
      case 19:  printf("enter directory/file name to change the permission\n");
                scanf("%s",file_name);
                int mode;
                printf("enter the permission\n");
                scanf("%d",&mode);
                chmod_myfs(file_name,mode);
                break;
      case 20: create_myfs(10);
                char myroot[30];
                strcpy(myroot,"myroot");
                char mydocs[30];
                strcpy(mydocs,"mydocs");
                char mycode[30];
                strcpy(mycode,"mycode");
                char mytext[30];
                strcpy(mytext,"mytext");
                mkdir_myfs(myroot);
                chdir_myfs(myroot);
                mkdir_myfs(mydocs);
                mkdir_myfs(mycode);
                chdir_myfs(mydocs);
                mkdir_myfs(mytext);
                break;
  
      default:
              break;

    }
  }
  return 1;

}


#endif