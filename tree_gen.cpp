#include <iostream>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "dist.h"
#include "tree_gen.h"

using namespace std;

char conff[300];
char dataf[300];
char parf[300];

ofstream outconf, outdata;

long ranseed = -3333;
int max_depth,fan_out,subtree,num_node,max_nodes;
vector<int> current_subtree;
vector<double> level_probs;
vector<vector<nodeptr> > parent_tree;

double totfanout=0.0;
double totdepth=0.0;
int totfanoutcalls=0;
int cur_max_depth=0;
bool ascii=false;
bool read_parent=false; //read in the parent tree from file
bool output_parent = false; //output parent tree as DB?
bool use_subtree = false; //always use root?

void Display_Help()
{
   cout << "-a : ascii output flag" << endl;
   cout << "-b : Use Random Subtree as root node" << endl;
   cout << "-d : Depth, default 5" << endl;
   cout << "-f : Fan out factor, default 5" << endl;
   cout << "-m : Total number of nodes in parent tree" << endl;
   cout << "-n : Number of items, default 10" << endl;
   cout << "-o : out file name (required)" << endl;
   cout << "-p : output parent tree as database" << endl;
   cout << "-P : read parent tree form <filename> (not implemented)" << endl;
   cout << "-s : seed for random number gen" << endl;
   cout << "-t : Number of subtrees, default 100" << endl;
}

void Parse_Parameters(int argc,char **argv)
{
   max_depth = 5;
   fan_out   = 5;
   num_node = 10;
   subtree   = 100;
   max_nodes = num_node;
   
   extern char * optarg;
   int c;

   if (argc < 2){
      Display_Help();
      exit(0);
   }
   else{
      while ((c=getopt(argc,argv,"abd:f:n:m:o:ps:t:"))!=-1){
         switch(c){
         case 'a':
            ascii = true;
            break;
         case 'b':
            use_subtree = true;
            break;
         case 'd': 
            max_depth = atoi(optarg);
            break;
         case 'f': 
            fan_out = atoi(optarg);
            break;
         case 'n': 
            num_node = atoi(optarg);
            break;  
         case 'm': 
            max_nodes = atoi(optarg);
            break;
         case 'o':
            sprintf(conff, "%s.conf", optarg);
            sprintf(dataf, "%s.data", optarg);
            break;
         case 'p':
            output_parent = true;
            break;
         case 'P':
            read_parent=true;
            sprintf(parf, "%s", optarg);
            break;
         case 's':
            ranseed = atoi(optarg);
            break;           
         case 't': 
            subtree = atoi(optarg);
            break;
         }
      }
   }
}

int Get_FanOut_Factor()
{
  static UniformDist ud; 
  return (int(ud() * 100000) % fan_out) +  2;
}

int Continue()
{
  int ret=0;
  float udi;
  static UniformDist ud; 

  udi = ud();
  if (udi < 0.2 || (udi > 0.4 && udi < 0.5) || (udi > 0.7 && udi < 0.9))
     ret = 1;
  return (ret) ;
}

float Get_Random_Number()
{

  static UniformDist ud;
  double udv = ud();
  //cout << "RAN " << udv << endl;
  return(udv);
  //return float(float((rand() % 100)) / float(100));
}

int Get_Next_Node_No()
{
   static UniformDist ud;
   //static int node_no = 1;
   //return node_no++;
   int node_no = (int)(num_node*ud());
   //cout << "NODE " << node_no << endl;
   return node_no;
}

void read_tree(nodeptr& current_node)
{
   
}


void Generate_Tree(nodeptr& current_node)
{
   static int tot_nodes = 1;   
   static int depth = 0;
   int i;

   depth++;

   //alloc storage for parent tree at current depth
   if (depth >= parent_tree.size()){
      parent_tree.push_back(vector<nodeptr>());
   }
   
   float sum=0;
   float rval;
   int k = Get_FanOut_Factor();
   current_node->child_num = k;
   for (i=0; i<k; i++)
   {
      nodeptr tmp = new struct node;
      tmp->node_no = Get_Next_Node_No();
      tmp->child_num = 0;
      rval = Get_Random_Number();
      //cout << "rval " << rval << endl;
      current_node->links[i].probability = rval;
      sum += rval;
      current_node->links[i].next_level = tmp;

      parent_tree[depth].push_back(tmp);
      
      tot_nodes++;
      
      if (tot_nodes < max_nodes && Continue() && depth < max_depth) 
         Generate_Tree(tmp); 

      if (tot_nodes >= max_nodes){
         k = i+1;
         break;
      }
   }
   depth--;
   
   current_node->child_num = k;
   
   //rval = Get_Random_Number();
   sum += rval;
   //cout<<"Probs of " << current_node->node_no<<" ";
   for (int i=0; i<k; i++)
   {
      current_node->links[i].probability /= sum;
      //cout<<current_node->links[i].probability<<" ";
   }
   //cout<<endl;

}


void Traverse_Tree(nodeptr current_node)
{
   outconf << current_node->node_no << " ";

   if (current_node->child_num == 0)
   {
      //cout << "current node  " << current_node->node_no<<" "<<k<<endl;
      return;
   }
   else {
      for (int i=0; i<current_node->child_num; i++)
      {
         outconf << " (" << current_node->links[i].probability << ") ";
         Traverse_Tree(current_node->links[i].next_level);
         outconf << "-1 ";
      }
   }
}

void Produce_SubTrees(nodeptr current_node, int depth)
{
   //cout << current_node->node_no << " ";
   if (depth > cur_max_depth) cur_max_depth = depth;
   
   current_subtree.push_back(current_node->node_no);

   if (current_node->child_num == 0)
   {
      return;
   }
   else {
            float sum_random,sum_prob;
            int i=0;
            sum_random = Get_Random_Number();
            sum_prob   = 0;
            int numnodes = 0;
  
            while (i < current_node->child_num)
            {
                sum_prob = current_node->links[i].probability;
                while (sum_random > sum_prob && i < current_node->child_num-1)
                      sum_prob   += current_node->links[++i].probability;

                if (sum_random <= sum_prob && i < current_node->child_num)
                {
                   numnodes++;                  
                   Produce_SubTrees(current_node->links[i].next_level,depth+1);
                   current_subtree.push_back(-1);
                   //cout << "-1 " ;

                   for (int f=i+1; f<current_node->child_num-1; f++)
                       current_node->links[f].probability /= (1-sum_prob);
                   
                   sum_prob   = 0;
                   sum_random = Get_Random_Number();
                }
                i++;
            }
            totfanout += numnodes;
            totfanoutcalls++;           
        }
}

void print_parent_tree()
{
   for (int i=0; i < parent_tree.size(); i++){
      cout << "depth " << i << ":";
      for (int j=0; j < parent_tree[i].size(); j++)
         cout << " " << parent_tree[i][j]->node_no;
      cout << endl;
   }
}

//assumes that generate_level_probs has been called before hand
nodeptr get_subtree_root()
{
   static UniformDist ud;
   //choose uniformly at random from the level and from nodes at that level
   double randv = ud();
   double sum=0;
   int level;

   for (int i=0; i < level_probs.size(); ++i){ 
      sum += level_probs[i];
      if (sum >= randv){ 
         level = i;
         break;
      }
      
   }
   
   //cout << "RANDV " << randv << " "<< level << endl;
   
   //int level = (int)(ud()*parent_tree.size());
   int rnode = (int) (ud()*parent_tree[level].size());
   return parent_tree[level][rnode];
}

void generate_level_probs()
{
   double sum=0;
   //the probability of picking level i is 1/(i+1); levels begins at 0
   for (int i =0; i < parent_tree.size(); i++) sum += (1.0/(i+1));
   //normalize the probabilty
   for (int i =0; i < parent_tree.size(); i++){
      cout << "LEVEL PROB " << i << " " << (1.0/((i+1)*sum)) << endl;
      level_probs.push_back((1.0/((i+1)*sum)));
   }
}


void get_parent(vector<int> &subtree, nodeptr current_node, int depth)
{
   subtree.push_back(current_node->node_no);
   if (depth > cur_max_depth) cur_max_depth = depth;
   
   if (current_node->child_num == 0) return;
   else {
      for (int i=0; i<current_node->child_num; i++)
      {
         get_parent(subtree, current_node->links[i].next_level, depth+1);
         subtree.push_back(-1);
      }
      totfanout += current_node->child_num;
      totfanoutcalls++;     
   }  
}


int main(int argc, char *argv[])
{
   Parse_Parameters(argc,argv);
   //RandSeed::new_seed();

   RandSeed::set_seed(ranseed);

   nodeptr root;
   root = new struct node;
   root->node_no = Get_Next_Node_No();
   
   vector<nodeptr> rptr;
   rptr.push_back(root);
   parent_tree.push_back(rptr);

   //if (read_parent) read_tree(root);
   //else 
   Generate_Tree(root);
   
   outconf.open(conff, ios::out);
   Traverse_Tree(root);
   outconf << endl;
   outconf.close();
   
   //print_parent_tree();
   
   if (ascii) outdata.open(dataf, ios::out);
   else outdata.open(dataf, ios::out|ios::binary);

   double totsize=0;
   
   int i,j;
   if (output_parent){
      i = 0;
      if (ascii){
         outdata << i << " " << i << " ";
      }
      else{
         outdata.write((const char*) &i, sizeof(int));
         outdata.write((const char*) &i, sizeof(int));
      }
      
      current_subtree.clear();
      cur_max_depth = 0;     
      get_parent(current_subtree, root, 1);
      totdepth += cur_max_depth;        
      j = current_subtree.size();

      if (ascii) outdata << j;
      else outdata.write((const char*) &j, sizeof(int));
      totsize += j;
     
   
      for (j=0; j < current_subtree.size(); j++){
         if (ascii) outdata << " " << current_subtree[j];
         else outdata.write((const char*) &current_subtree[j], sizeof(int));
      }
      if (ascii) outdata << endl;
   }
   else{
      generate_level_probs(); //generate level probabilites for subtree gen
      for (i=0 ; i < subtree; i++){
         current_subtree.clear();

         //use_subtree starts at some random subtree root, or else 
         //the subtree will always start at the main root.
         if (use_subtree) root = get_subtree_root();

         cur_max_depth = 0;        
         Produce_SubTrees(root,1);
         totdepth += cur_max_depth;        

         if (ascii) {
            outdata << i << " " << i << " " << current_subtree.size();
         }
         else{
            outdata.write((const char*) &i, sizeof(int));
            outdata.write((const char*) &i, sizeof(int));
            j = current_subtree.size();
            outdata.write((const char*) &j, sizeof(int));
         }
         
         totsize += j;
         
         for (j=0; j < current_subtree.size(); j++){
            if (ascii) outdata << " " << current_subtree[j];
            else outdata.write((const char*) &current_subtree[j], sizeof(int));
         }
         if (ascii) outdata << endl;
      }
   }
   
   outdata.close();

   cout << "TREEGEN";
   cout << " M = " << max_nodes << " N = " << num_node;
   cout << " D = " << max_depth << " F = " << fan_out;
   cout << " T = " << subtree;
   cout << " avg_nodes = " << totsize/subtree;
   cout << " avg_fanout = " << totfanout/totfanoutcalls;
   cout << " avg_max_depth = " << totdepth/subtree;
   cout << endl;

   //write results to summary file
   ofstream summary("summary.out", ios::app);
   summary << "TREEGEN";
   summary << " M = " << max_nodes << " N = " << num_node;
   summary << " D = " << max_depth << " F = " << fan_out;
   summary << " T = " << subtree;
   summary << " avg_nodes = " << totsize/subtree;
   summary << " avg_fanout = " << totfanout/totfanoutcalls;
   summary << " avg_max_depth = " << totdepth/subtree;
   summary << endl;   
   summary.close();
}



