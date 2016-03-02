#include <vector>
#include <algorithm>
#include <mash-goalplanning/planner.h>

using namespace Mash;
using namespace std;

class gsc_priors{
public:
  

  gsc_priors(){
}

 scalar_t get_surprise(const scalar_t obs);

 public:

scalar_t mup;
scalar_t sigma,sigmap;
scalar_t zeta;
};
