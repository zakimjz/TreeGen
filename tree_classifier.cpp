//USAGE:  a.out  <pattern_file> <data_file>
#include <cstdio> 
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <strstream>
#include <iterator>
#include <vector>
#include <stack>
#include <algorithm>
//#include <algo.h>
//#include <cstdlib>
#include <ext/numeric>
#include <unistd.h>
#include "dist.h"

using namespace std;

#define BranchIt -1 //-1 indicates a branch in the tree
 
long ranseed = -1111;

//four possible values of a comparison
enum comp_vals {equals, subset, superset, notequal};
int defaultvalue;
char test[100];
char train[100];
char dbname[100];
char pname[100];
double pool_test_ratio=1.0, db_test_ratio=0.5;
double pool_train_ratio=1.0, db_train_ratio=0.5;
double pool_common_ratio=0.0;

//add itemsets to other class to confuse itemset classifier
bool add_itemsets=false; 
bool add_itemsets_y=false; 
double add_iset_prob=1.0; //how many cases to add

//structure to store frequent subtrees with sup and ratio info for two classes 
class FreqIt: public vector<int>{
public:
   void print(ostream &out=cout, bool printvl=false){
      int i;
      out << (*this)[0];
      for (i=1 ; i < size(); ++i){
         out << " " << (*this)[i];
      } 
      if (printvl) out << " -- " << valid_last_pos;
      out << endl;
   }
   
   int &vlpos(){ return valid_last_pos;}
   
private: 
   int valid_last_pos; //ignore -1's in the end
};
 
typedef vector<FreqIt *> FreqAry;
 
FreqAry dbtrees;
FreqAry pool;
FreqAry testpool, trainpool;

bool count_unique=true;
 
double Get_Random_Number()
{

  static UniformDist ud;
  double udv = ud();
  //cout << "RAN " << udv << endl;
  return(udv);
}

 
void read_trees(const char *ffile, FreqAry &ary)
{
   const int lineSize=8192;
   const int wdSize=256;
 
   FreqIt *fit;
 
   char inBuf[lineSize];
   char inStr[wdSize];
   int inSize;

   ifstream fin(ffile, ios::in);
   if (!fin){
      cout << "cannot open freq seq file " << ffile << endl << flush;
      exit(-1);
   }
 
   bool skipfile = false;
   while(fin.getline(inBuf, lineSize)){
      inSize = fin.gcount();
      int it;
      istrstream ist(inBuf, inSize);
      fit = new FreqIt;
      ist >> inStr; //skip cid
      ist >> inStr; //skip tid
      ist >> inStr; //skip nitems
      
      while(ist >> inStr){
         it = atoi(inStr);
         fit->push_back(it);
      }

      int i = fit->size()-1;
      while ((*fit)[i] == BranchIt) --i;
      fit->vlpos() = i+1; //set the last valid item (other than -1)
      
      ary.push_back(fit);
      //fit->print(cout,true);
   }
   fin.close();
}

void check_subtree(FreqIt &fit, FreqIt &fit2,
                  int tpos, int ppos, int tscope,
                  stack<int> &stk, bool &foundflg)
{
   int i;
   int scope, ttscope;
   stack<int> tstk;
 
   scope = tscope;
   bool skip = false;
   if (fit[ppos] == BranchIt){
      skip = true;
      while(fit[ppos] == BranchIt){
         tstk.push(stk.top());
         stk.pop();
         ppos++;
      }
      tscope = tstk.top();
   }
 
   while (skip && scope >= tscope && tpos < fit2.vlpos()){
      if (fit2[tpos] == BranchIt) scope--;
      else scope++;
      tpos++;
   }
 
   if (skip) tscope = stk.top();
 
   for (i=tpos; i < fit2.vlpos() && !foundflg; i++){
      if (fit2[i] == BranchIt) scope--;
      else scope++;
      if (scope < tscope) break;
      if (fit2[i] == fit[ppos]){
         stk.push(scope);
 
         //for (int d = 0; d < stk.size(); d++) cout << "\t";
 
         if (ppos == fit.vlpos()-1){
            //cout << ppos << " found at " << i << " " << scope << endl;
            //fit.dbsup++;
            foundflg = true;
         }
         else{
            //cout << ppos << " recurse at " << i << " " << scope << endl;
            check_subtree(fit, fit2, i+1, ppos+1, scope, stk, foundflg);
         }
         stk.pop();
      }
 
   }
 
   while(!tstk.empty()){
      stk.push(tstk.top());
      tstk.pop();
   }
}
 
comp_vals compare(FreqIt &fit1, FreqIt &fit2)
{
   stack<int> stk;
   bool foundflg = false;
 
   comp_vals res = notequal;
 
   if (fit1.vlpos() <= fit2.vlpos()){
      check_subtree(fit1, fit2, 0, 0, 0, stk, foundflg);
      //cout << "came here " << fit1.vlpos() << " " << fit2.vlpos() << endl;
      if (foundflg){
         if (fit1.vlpos() == fit2.vlpos()) res = equals;
         else res = subset;
      }
      else res = notequal;
   }
   else{
      check_subtree(fit2, fit1, 0, 0, 0, stk, foundflg);
      if (foundflg) res = superset;
      else res = notequal;
   }
 
   return res;
}
 
//uses globals: I:pool, O:trainpool, testpool
void split_pool()
{
   int i;
   
   int commonsize = (int)(pool_common_ratio*pool.size());
   int trainsize = (int)(pool_train_ratio*pool.size());
   int testsize = (int)(pool_test_ratio*pool.size());
   int deltatrain = trainsize - commonsize;
   int deltatest = testsize - commonsize;
   
   vector<int>posary(pool.size());


   //create common and train pool
   iota(posary.begin(), posary.end(), 0); //assigns succesive vals to posary
   //for (i=0; i < pool.size(); ++i) posary.push_back(i);
   //randomly shuffle trainsize instances
   for (i=0; i < commonsize+deltatrain+deltatest; ++i){
      int rpos = (int) (Get_Random_Number()*pool.size());
      int temp = posary[i];
      posary[i] = posary[rpos];
      posary[rpos] = temp;
   }

   sort(&posary[0], &posary[commonsize]);
   
   if (commonsize > 0){
      cout << "COMMON POOL ";
      for (i=0; i < commonsize; ++i){
         cout << posary[i] << " ";
         trainpool.push_back(pool[posary[i]]);
         testpool.push_back(pool[posary[i]]);
      }
      cout << endl;
   }
   
   sort(&posary[commonsize], &posary[commonsize+deltatrain]);
   //populate trainpool
   cout << "ADD TO TRAINPOOL ";
   for (i=commonsize; i < trainsize; ++i){
      cout << posary[i] << " ";
      trainpool.push_back(pool[posary[i]]);
   }
   cout << endl;

   sort(&posary[commonsize+deltatrain], &posary[commonsize+deltatrain+deltatest]);
   //populate testpool
   cout << "ADD TO TESTPOOL ";
   for (i=trainsize; i < trainsize+deltatest; ++i){
      cout << " " << posary[i];
      testpool.push_back(pool[posary[i]]);
   }
   cout << endl;
}

void print_itemset(FreqIt &tree, ofstream &out, int &cnt, int lbl)
{
   vector<int>::iterator upos;
   vector<int> iset(tree.size());
   upos = remove_copy(tree.begin(), tree.end(), iset.begin(), BranchIt);
   sort(iset.begin(), upos);
   upos = unique(iset.begin(), upos);
   
   int nsz = upos-iset.begin();
   nsz  = 2*nsz-1;
   fill(upos, iset.begin()+nsz, BranchIt);
   //cout << "DONE ";
   //copy(iset.begin(), iset.end(), ostream_iterator<int>(cout, " "));
   //cout << "\nXXX ";
   //tree.print(cout);
   out << lbl << " " << cnt << " " << cnt << " " << nsz << " ";
   copy(iset.begin(), iset.begin()+(nsz-1), ostream_iterator<int>(out, " "));
   out << iset[nsz-1] << endl;
   ++cnt;
}


void output_db(ofstream &out, FreqAry &poolary,
               int stpos, int endpos, vector<int> &posary)
{
   const int xlbl=0, ylbl=1;
   
   int i,j,k;
   int cnt=0;
   bool flag;

   
   //the first trainsize trans go to training set
   for (i=stpos; i < endpos; ++i){
      k = posary[i];
      flag = false;
      for(j=0; j < poolary.size() && !flag; ++j){         
         //compare pat1 vs pat2
         comp_vals cv = compare (*dbtrees[k], *poolary[j]);
 
         switch (cv){
         case equals:
         case superset: 
            flag=true;
            break; 

         case notequal:
         case subset:  
            break;               
         }
      }

      //if dbtree is equal or superset of pool then class is X
      //none of pool tree matches so class is Y
      
      if (flag){
         out << xlbl << " " << cnt << " " << cnt 
             << " " << dbtrees[k]->size() << " ";
         dbtrees[k]->print(out);
         ++cnt;
         if (add_itemsets){
            //add itemsets only to other class
            if (Get_Random_Number() < add_iset_prob)
               print_itemset(*dbtrees[k], out, cnt, ylbl); //add to Y
         }
      }
      else{
         out << ylbl << " " << cnt << " " << cnt 
             << " " << dbtrees[k]->size() << " ";
         dbtrees[k]->print(out);
         ++cnt;
         if (add_itemsets_y){
            if (Get_Random_Number() < add_iset_prob)
               print_itemset(*dbtrees[k], out, cnt, xlbl); //add to X
         }
      }
   }
}

void separatefiles()
{
   ofstream trainout, testout; //train & test out
   
   trainout.open(train, ios::out);
   testout.open(test, ios::out);

   //shuffle the dbtrees
   int i;
   vector<int>posary(dbtrees.size());
   iota(posary.begin(), posary.end(), 0); //assigns succesive vals to posary

   //for (i=0; i < dbtrees.size(); ++i){
   //    int rpos = (int) (Get_Random_Number()*dbtrees.size());
   //   int temp = posary[i];
   //   posary[i] = posary[rpos];
   //   posary[rpos] = temp;
   //}

   int trainsize = (int)(db_train_ratio*dbtrees.size());
   //output training db
   output_db(trainout, trainpool, 0, trainsize, posary);
   //output testing db
   output_db(testout, testpool, trainsize, dbtrees.size(), posary);
   
   trainout.close();
   testout.close();
}
 

void parse_args(int argc, char **argv)
{
   extern char * optarg;
   int c;

   if (argc < 3){
      cerr << "usage: -d dbfile -o outfilename -p poolfile ";
      cout << "-r db_train_ratio (0.5) -s randseed ";
      cout << "-t pool_train_ratio (1.0) -T pool_test_ratio (1.0) ";
      cout << "-x add_iset_prob -y addytox\n";
      exit(-1);
   }
   else{
      while ((c=getopt(argc,argv,"c:d:o:p:r:s:t:T:x:y"))!=-1){
         switch(c){
         case 'c':
            pool_common_ratio = atof(optarg);
            break;
         case 'd':
            sprintf (dbname, "%s.data", optarg);
            break;
         case 'o':
            //output files for X and Y classes
            sprintf(test, "%s.train.asc", optarg);
            sprintf(train, "%s.test.asc", optarg);
            break;
         case 'p':
            //pool file
            sprintf (pname, "%s.data", optarg);
            break;
         case 'r':
            db_train_ratio = atof(optarg);
            db_test_ratio = 1-db_train_ratio;
            break;
         case 's':
            ranseed = atol(optarg);
            break;
         case 't':
            pool_train_ratio = atof(optarg);
            break;
         case 'T':
            pool_test_ratio = atof(optarg);
            break;
         case 'x':
            add_itemsets = true;
            add_iset_prob = atof(optarg);
            break;
         case 'y':
            add_itemsets_y = true;
            break;
         }
      }  
   }
   if ((pool_common_ratio+(pool_train_ratio-pool_common_ratio)+
        (pool_test_ratio-pool_common_ratio)) > 1){
      cout << "ERROR  in pool sizes\n";
      exit(-1);
   }
   
}

int main(int argc, char **argv)
{
   parse_args(argc,argv);
   
   RandSeed::set_seed(ranseed);
   //read trees to be classified 
   read_trees (dbname,dbtrees);
   //read pool for class X
   read_trees (pname, pool);
   
   //compare all pairs of pats, create train and test sets
   split_pool();
   separatefiles();
   exit(0);
}
