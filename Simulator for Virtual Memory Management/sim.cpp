#include <bits/stdc++.h>

#define map_cost 250
#define unmap_cost 250
#define page_in_cost 3000
#define page_out_cost 3000
#define mem_acc_cost 1

#define virtual_AS 64

using namespace std;

int num_frames;
std::vector< bitset<32> > pageTable(64,0);
list<int> page_fifo;

int page_fault=0;
int page_trans=0;
int total_cost=0;

void init(list<int>& free_frames)
{
  free_frames.resize(num_frames);
  page_fault=0;
  page_trans=0;
  total_cost=0;
  std::iota(free_frames.begin(),free_frames.end(),0);
  for(int i=0;i<64;i++)
    pageTable[i].reset();
}

vector<string> split(const string &s, char delim)
{
    stringstream ss(s);
    string item;
    vector<string> tokens;
    while (getline(ss, item, delim))
    {
        tokens.push_back(item);
    }
    return tokens;
}


void print_pageTable()
{
  for(int i=0;i<pageTable.size();i++)
  {
    cout<<"i "<<i<<" Pt[i] "<< pageTable[i]<<endl;
  }
}

bool is_valid(int virtual_page)
{
  if(pageTable[virtual_page][29]==0)
    return false;
  else
    return true;
}
bool is_dirty(int virtual_page)
{
  if(pageTable[virtual_page][30]==0)
    return false;
  else
    return true;
}

void set_referenced_bit(int  virtual_page)
{
  pageTable[virtual_page][31]=1;
}

void set_dirty_bit(int  virtual_page)
{
  pageTable[virtual_page][30]=1;
}

void reset_dirty_bit(int  virtual_page)
{
  pageTable[virtual_page][30]=0;
}

void map_table(int virtual_page,int frame)
{
  bitset<29> tmp(frame);
  for(int i=0;i<29;i++)
    pageTable[virtual_page][i]=tmp[i];
}
int get_frame(int virtual_page)
{
  bitset<29> tmp;
  for(int i=0;i<29;i++)
    tmp[i]=pageTable[virtual_page][i];
  int frame = (int)(tmp.to_ulong());
  return frame;
}

void handle_instruction_fifo(int instr_num,int rw, int virtual_page, list<int>& free_frames)
{

  unordered_set<int> frame_in_memory;

  if(is_valid(virtual_page))
  {
    // Valid
    set_referenced_bit(virtual_page);
    if(rw)
      set_dirty_bit(virtual_page);
    total_cost+=mem_acc_cost;
  }
  else
  {
    // Page Fault
    page_fault++;
    if(free_frames.size()>0)
    {
      // Memory not full
      int frame=free_frames.front();
      free_frames.pop_front();

      // load page virtual_page into frame
      cout<<instr_num<<": "<<"IN"<<" "<<virtual_page<<" "<<frame<<endl;
      total_cost+=page_in_cost;
      page_trans++;
      page_fifo.push_back(virtual_page);

      // map virtual_page to frame
      total_cost+=map_cost;
      map_table(virtual_page,frame);
      cout<<instr_num<<": "<<"MAP"<<" "<<virtual_page<<" "<<frame<<endl;

      // read write
      set_referenced_bit(virtual_page);
      if(rw)
        set_dirty_bit(virtual_page);
      total_cost+=mem_acc_cost;

    }
    else
    {
        int victim_page=page_fifo.front();               // SELECT VICTIM PAGE
        page_fifo.pop_front();

        int victim_frame=get_frame(victim_page);

        // UNMAP
        cout<<instr_num<<": "<<"UNMAP"<<" "<<victim_page<<" "<<victim_frame<<endl;
        total_cost+=unmap_cost;

        if(is_dirty(victim_page))
        {
          // write to memory
          reset_dirty_bit(virtual_page);
          cout<<instr_num<<": "<<"OUT"<<" "<<victim_page<<" "<<victim_frame<<endl;
          total_cost+=page_out_cost;
          page_trans++;
        }

        // load page virtual_page into frame
        cout<<instr_num<<": "<<"IN"<<" "<<virtual_page<<" "<<victim_frame<<endl;
        total_cost+=page_in_cost;
        page_trans++;

        // map virtual_page to frame
        total_cost+=map_cost;
        cout<<instr_num<<": "<<"MAP"<<" "<<virtual_page<<" "<<victim_frame<<endl;
        page_fifo.push_back(virtual_page);
        map_table(virtual_page,victim_frame);

        // read write
        total_cost+=mem_acc_cost;
        set_referenced_bit(virtual_page);
        if(rw)
          set_dirty_bit(virtual_page);

    }

  }
  cout<<endl;
}

void handle_instruction_random(int instr_num,int rw, int virtual_page, list<int>& free_frames,vector<int>& frame_to_page)
{

  if(is_valid(virtual_page))
  {
    // Valid
    set_referenced_bit(virtual_page);
    if(rw)
      set_dirty_bit(virtual_page);
    total_cost+=mem_acc_cost;
  }
  else
  {
    // Page Fault
    page_fault++;
    if(free_frames.size()>0)
    {
      // Memory not full
      int frame=free_frames.front();
      free_frames.pop_front();

      // load page virtual_page into frame
      cout<<instr_num<<": "<<"IN"<<" "<<virtual_page<<" "<<frame<<endl;
      total_cost+=page_in_cost;
      page_trans++;
      page_fifo.push_back(virtual_page);


      // map virtual_page to frame
      map_table(virtual_page,frame);
      frame_to_page[frame]=virtual_page;
      cout<<instr_num<<": "<<"MAP"<<" "<<virtual_page<<" "<<frame<<endl;
      total_cost+=map_cost;

      // read write
      set_referenced_bit(virtual_page);
      if(rw)
        set_dirty_bit(virtual_page);
      total_cost+=mem_acc_cost;
    }
    else
    {


        int victim_frame=rand()%num_frames;
        int victim_page=frame_to_page[victim_frame];

        // UNMAP
        total_cost+=unmap_cost;
        cout<<instr_num<<": "<<"UNMAP"<<" "<<victim_page<<" "<<victim_frame<<endl;
        if(is_dirty(victim_page))
        {
          // write to memory
          reset_dirty_bit(virtual_page);
          cout<<instr_num<<": "<<"OUT"<<" "<<victim_page<<" "<<victim_frame<<endl;
          total_cost+=page_out_cost;
          page_trans++;
        }

        // load page virtual_page into frame
        cout<<instr_num<<": "<<"IN"<<" "<<virtual_page<<" "<<victim_frame<<endl;
        total_cost+=page_in_cost;
        page_trans++;

        // map virtual_page to frame
        total_cost+=map_cost;
        cout<<instr_num<<": "<<"MAP"<<" "<<virtual_page<<" "<<victim_frame<<endl;

        map_table(virtual_page,victim_frame);
        frame_to_page[victim_frame]=virtual_page;

        // read write
        set_referenced_bit(virtual_page);
        if(rw)
          set_dirty_bit(virtual_page);
        total_cost+=mem_acc_cost;

    }

  }
  cout<<endl;
}


void handle_instruction_LRU(int instr_num,int rw, int virtual_page, list<int>& free_frames,vector<int>& page_use)
{

  page_use[virtual_page]=instr_num;

  if(is_valid(virtual_page))
  {
    // Valid
    set_referenced_bit(virtual_page);
    if(rw)
      set_dirty_bit(virtual_page);
    total_cost+=mem_acc_cost;
  }
  else
  {
    // Page Fault
    page_fault++;
    if(free_frames.size()>0)
    {
      // Memory not full
      int frame=free_frames.front();
      free_frames.pop_front();

      // load page virtual_page into frame
      cout<<instr_num<<": "<<"IN"<<" "<<virtual_page<<" "<<frame<<endl;
      total_cost+=page_in_cost;
      page_trans++;
      page_fifo.push_back(virtual_page);


      // map virtual_page to frame
      map_table(virtual_page,frame);

      cout<<instr_num<<": "<<"MAP"<<" "<<virtual_page<<" "<<frame<<endl;
      total_cost+=map_cost;

      // read write
      set_referenced_bit(virtual_page);
      if(rw)
        set_dirty_bit(virtual_page);
      total_cost+=mem_acc_cost;
    }
    else
    {


        int lru=INT_MAX,victim_page;
        for(int i=0;i<virtual_AS;i++)
          if(page_use[i]<lru)
          {
            lru=page_use[i];
            victim_page=i;
          }

        int victim_frame=get_frame(victim_page);
        // UNMAP
        total_cost+=unmap_cost;
        cout<<instr_num<<": "<<"UNMAP"<<" "<<victim_page<<" "<<victim_frame<<endl;
        page_use[victim_page]=INT_MAX;
        if(is_dirty(victim_page))
        {
          // write to memory
          reset_dirty_bit(virtual_page);
          cout<<instr_num<<": "<<"OUT"<<" "<<victim_page<<" "<<victim_frame<<endl;
          total_cost+=page_out_cost;
          page_trans++;
        }

        // load page virtual_page into frame
        cout<<instr_num<<": "<<"IN"<<" "<<virtual_page<<" "<<victim_frame<<endl;
        total_cost+=page_in_cost;
        page_trans++;

        // map virtual_page to frame
        total_cost+=map_cost;
        cout<<instr_num<<": "<<"MAP"<<" "<<virtual_page<<" "<<victim_frame<<endl;

        map_table(virtual_page,victim_frame);


        // read write
        set_referenced_bit(virtual_page);
        if(rw)
          set_dirty_bit(virtual_page);
        total_cost+=mem_acc_cost;

    }

  }
  cout<<endl;
}

int main()
{
  cout<<"Enter number of frames in memory"<<endl;
  cin>>num_frames;
  list< int> free_frames(num_frames);
  std::iota(free_frames.begin(),free_frames.end(),0);
  int instr_num=1;

  ifstream file("test.txt");


  cout<<"Page Replacement Algorithm: FIFO"<<endl;
  for (std::string line; std::getline(file, line); )
  {
    if(line[0]=='#')
      continue;
    std::vector<string> line_vec=split(line,' ');
    int rw=stoi(line_vec[0]);
    int virtual_page=stoi(line_vec[1]);
    handle_instruction_fifo(instr_num,rw,virtual_page,free_frames);
    instr_num++;
  }
  cout<<"Page faults: "<<page_fault<<". Page transfers: "<<page_trans<<". Total cost: "<<total_cost<<endl<<"-----------------------------------"<<endl;
  file.close();


  init(free_frames);
  vector<int> frame_to_page(num_frames);
  cout<<"Page Replacement Algorithm: Random"<<endl;
  ifstream file_random("test.txt");
  instr_num=1;
  for (std::string line; std::getline(file_random, line); )
  {
    if(line[0]=='#')
      continue;
    std::vector<string> line_vec=split(line,' ');
    int rw=stoi(line_vec[0]);
    int virtual_page=stoi(line_vec[1]);
    handle_instruction_random(instr_num,rw,virtual_page,free_frames,frame_to_page);
    instr_num++;
  }
  cout<<"Page faults: "<<page_fault<<". Page transfers: "<<page_trans<<". Total cost: "<<total_cost<<endl<<"-----------------------------------"<<endl;
  file_random.close();


  init(free_frames);
  vector<int> page_use(virtual_AS,INT_MAX);
  cout<<"Page Replacement Algorithm: LRU"<<endl;
  ifstream file_LRU("test.txt");
  instr_num=1;
  for (std::string line; std::getline(file_LRU, line); )
  {
    if(line[0]=='#')
      continue;
    std::vector<string> line_vec=split(line,' ');
    int rw=stoi(line_vec[0]);
    int virtual_page=stoi(line_vec[1]);
    handle_instruction_LRU(instr_num,rw,virtual_page,free_frames,page_use);
    instr_num++;
  }
  cout<<"Page faults: "<<page_fault<<". Page transfers: "<<page_trans<<". Total cost: "<<total_cost<<endl<<"-----------------------------------"<<endl;
  file_LRU.close();

}
