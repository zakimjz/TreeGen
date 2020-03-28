#include <iostream>
#include "dist.h"
#include <values.h>

using namespace std;

//------------------------------- RandSeed -------------------------------

UniformDist *RandSeed::ran1 = new UniformDist(INIT_SEED);

void RandSeed::set_seed(long new_seed)
{
  delete ran1;
  ran1 = new UniformDist(new_seed);
  //cout << "rand_seed : " << new_seed << " " << endl;
}

long RandSeed::new_seed(void)
{
  float ans;

  ans = (*ran1)();
  ans *= (*ran1)();
  cout << "rand_seed : " << long(-MAXLONG * ans) << endl;
  return long(-MAXLONG * ans);
}


//------------------------------- Choose -------------------------------


// allows selection of k random items from the string
// 
Choose::Choose(long n, long k)
{
  UniformDist ud;
  long i, j, ival;
  float fval;

  num = new long [n];
  rval = new float [n];

  // associate a random value with each item
  // also copy item into num
  for (i = 0; i < n; i++)
    {
      rval[i] = ud();
      num[i] = i;
    }
  
  // sort num according to the values in rval
  for (i = 0; i < n; i++ )
    {
      ival = num[i]; fval = rval[i];
      for ( j = i; j > 0 && rval[j-1] > fval; j-- ) {
	  num[j] = num[j-1];
	  rval[j] = rval[j-1];
	}
      num[j] = ival;
      rval[j] = fval;
    }

  // resort first k num according to position
  for (i = 0; i < k; i++ )
    {
      ival = num[i]; 
      for ( j = i; j > 0 && num[j-1] > ival; j-- )
	num[j] = num[j-1];
      num[j] = ival;
    }
}


Choose::~Choose(void)
{
  delete [] num;
  delete [] rval;
}


