#include "gsc_priors.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <math.h>
using namespace std;

scalar_t gsc_priors::get_surprise(const scalar_t obs){

scalar_t mup2,sigmap2;
scalar_t bs_surprise;

mup2 = ((zeta*mup/(sigmap*sigmap)) + (obs/(sigma*sigma)))/((zeta/(sigmap*sigmap)) + (1/(sigma*sigma)));
sigmap2 = sqrt(((sigma*sigma)*(sigmap*sigmap))/((zeta*sigma*sigma)+sigmap*sigmap));

bs_surprise = (sigmap*sigmap)/(2*sigma*sigma) + (sigmap*sigmap)*(mup-obs)*(mup-obs)/(2*(sigma*sigma)*((sigma*sigma)+(sigmap*sigmap)));
mup=mup2;
sigmap=sigmap2;
return bs_surprise;
}

            
