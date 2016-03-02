/*******************************************************************************
* The MASH Framework contains the source code of all the servers in the
* "computation farm" of the MASH project (http://www.mash-project.eu),
* developed at the Idiap Research Institute (http://www.idiap.ch).
*
* Copyright (c) 2016 Idiap Research Institute, http://www.idiap.ch/
* Written by Leonidas Lefakis (leonidas.lefakis@idiap.ch)
*
* This file is part of the MASH Framework.
*
* The MASH Framework is free software: you can redistribute it and/or modify
* it under the terms of either the GNU General Public License version 2 or
* the GNU General Public License version 3 as published by the Free
* Software Foundation, whichever suits the most your needs.
*
* The MASH Framework is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public Licenses
* along with the MASH Framework. If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/


/** Author: Leonidas Lefakis (leonidas.lefakis@idiap.ch)

    TODO: Write a description of your goal-planner
*/

#include <mash-goalplanning/planner.h>
#include <vector>
#include <algorithm>
#include <math.h>
#include "gsc_priors.h"

using namespace Mash;
using namespace std;


//------------------------------------------------------------------------------
// Declaration of the goal-planner class
//------------------------------------------------------------------------------
class bs_cluster_planner: public Planner
{
   typedef vector<scalar_t> scalar2D_t;
   typedef vector<scalar2D_t> scalar3D_t;
   typedef vector<gsc_priors> vec_gsp;
   typedef vector<vec_gsp> mat_gsp;
   typedef vector<mat_gsp> multi_mat_gsp;
public:
    bs_cluster_planner();
    virtual ~bs_cluster_planner();


    //_____ Methods to implement __________
public:

    virtual bool setup(const tExperimentParametersList& parameters);
    virtual bool learn(ITask* task);
    virtual unsigned int chooseAction(IPerception* perception);
    virtual bool reportFeaturesUsed(tFeatureList &list);
    virtual bool init(ITask* task);
    virtual bool computeState(IPerception* perception);

    
  virtual bool loadModel(PredictorModel &model);
  virtual bool saveModel(PredictorModel &model);
  

    //_____ Attributes __________
protected:
    vector<scalar2D_t> current_state;
    vector<scalar3D_t> previous_states;
    mat_gsp vec_gsp_grid;
    multi_mat_gsp clusters_mat_gsp;

    unsigned int num_features;
    unsigned int h_c,w_c;
    vector<coordinates_t> grid_coordinates; 
};


//------------------------------------------------------------------------------
// Creation function of the goal-planner
//------------------------------------------------------------------------------
extern "C" Planner* new_planner()
{
    return new bs_cluster_planner();
}



/************************* CONSTRUCTION / DESTRUCTION *************************/

bs_cluster_planner::bs_cluster_planner()
{
    // TODO: Initialization of the attributes that doesn't depend of anything
}


bs_cluster_planner::~bs_cluster_planner()
{
    // TODO: Cleanup of the allocated memory still remaining
}


/************************* IMPLEMENTATION OF Classifier ***********************/

bool bs_cluster_planner::setup(const tExperimentParametersList& parameters)
{
    // TODO: Implement it
    return true;
}


bool bs_cluster_planner::learn(ITask* task)
{
  scalar_t mup,sigmap;
  init(task);
  unsigned int ep_counter=0;
  unsigned int number_collect_episodes=4;
  unsigned int number_episodes=100;
  unsigned int total_collect_steps=300;
  unsigned int total_steps=1000;
  unsigned int t_counter;
  unsigned int won=0;
  scalar_t reward=0;
  scalar_t bs_zeta=0.95;

  IPerception* perception;


  while( ep_counter<number_collect_episodes){

    for (unsigned int j = 0; j <total_collect_steps; j++){

      perception = task->perception();
      computeState(perception); 
      if(ep_counter==0) previous_states.push_back(current_state);
      else  previous_states[j]=current_state;
      
      scalar_t temp_reward = 0;
      task->performAction(2, &temp_reward);	
    }
    
    for (unsigned int j = 0; j < h_c*w_c; ++j){
      for (unsigned int k = 0; k < num_features; ++k){
	mup=0;
	for (unsigned int i = 0; i < previous_states.size(); ++i){
	  mup +=previous_states[i][j][k];
	}
	vec_gsp_grid[j][k].mup = mup/previous_states.size();  
	vec_gsp_grid[j][k].zeta = bs_zeta;  
	
      }
      
    }	
    
    for (unsigned int j = 0; j < h_c*w_c; ++j){
      for (unsigned int k = 0; k < num_features; ++k){
	sigmap=0;
	for (unsigned int i = 0; i < previous_states.size(); ++i){
	  sigmap +=(previous_states[i][j][k]-vec_gsp_grid[j][k].mup)*(previous_states[i][j][k]-vec_gsp_grid[j][k].mup);
	}
	vec_gsp_grid[j][k].sigma = sqrt(sigmap/previous_states.size()); 
	vec_gsp_grid[j][k].sigmap = sqrt(sigmap/previous_states.size());  
	
      }     
    }
    clusters_mat_gsp.push_back(vec_gsp_grid);
    ep_counter++;
    
  }
  return true;
}
  

unsigned int bs_cluster_planner::chooseAction(IPerception* perception)
{

  vector<scalar_t> bsurprises;  
  scalar_t max_surprise;	
  unsigned int max_indx;
  unsigned int min_max_indx=0;
  scalar_t min_max_surprise;   
   
  computeState(perception); 
  
  for(unsigned int k = 0; k<clusters_mat_gsp.size();k++){
    max_indx=0;
    max_surprise=1.5;
    for (unsigned int j=0 ; j<h_c*w_c; j++){
      if (k==0) bsurprises.push_back(0.0);
      else bsurprises[j]=0.0;
      
      for (unsigned int i=0 ; i<num_features; i++){
	bsurprises[j]+= clusters_mat_gsp[k][j][i].get_surprise(current_state[j][i]);
      }
      if (bsurprises[j]>max_surprise){
        max_surprise = bsurprises[j];
        max_indx    = j; 
      }
    }

    if (k==0){
      min_max_surprise = max_surprise;
      min_max_indx = max_indx;
    } 
    if (max_surprise<min_max_surprise){
      min_max_surprise = max_surprise;
      min_max_indx = max_indx;
    }
  }
  

  if (min_max_indx%w_c<0.35*w_c) return 2;
  if (min_max_indx%w_c>0.6*w_c) return 3;   
  return 0;
  
}

bool bs_cluster_planner::reportFeaturesUsed(tFeatureList &list)
{
  list.push_back(tFeature(0, 0));
  return true;
}

/////////////////////////////
bool bs_cluster_planner::computeState(IPerception* perception)
{
  unsigned int *indexes;
  scalar_t *values;
 
    for (unsigned int gr_h = 0 ; gr_h < h_c; ++gr_h){
    for (unsigned int gr_w = 0 ; gr_w < w_c; ++gr_w){
     
      for (unsigned int j=0; j< perception->nbHeuristics(); j++){
	indexes = new unsigned int[perception->nbFeatures(j)];
	for (unsigned int k=0; k<perception->nbFeatures(j); k++) indexes[k]=k;
	values = new scalar_t[perception->nbFeatures(j)];
        unsigned int current_index=0;
	perception->computeSomeFeatures(0,
				    grid_coordinates[gr_h*w_c + gr_w],
                                    j, 	
				    perception->nbFeatures(j),
				    indexes,
				    values);

	for (unsigned int k=0; k<perception->nbFeatures(j); k++){
	  current_state[gr_h*w_c + gr_w][current_index]= values[k];
	  current_index++;
	}
	delete[] indexes;
	delete[] values;
      }
    }
  }
    return true;
}

////////////////////////////////////
bool bs_cluster_planner::init(ITask* task)
{
  dim_t dime;
  
  IPerception* perception = task->perception();
  dime=perception->viewSize(0);
  
  h_c= dime.height/(2*perception->roiExtent());
  w_c= dime.width/(2*perception->roiExtent());

  unsigned int num_heuristics = perception->nbHeuristics();
  
  num_features = 0;
  
  for (unsigned int i = 0; i<num_heuristics; i++)
    num_features +=perception->nbFeatures(i);
  
  vector<scalar_t> temp_vec;
  temp_vec.assign(num_features,0.0);
  for (unsigned int k = 0 ; k<h_c*w_c; ++k) current_state.push_back(temp_vec);
  
  vec_gsp temp_vec_gsp; 
  for (unsigned int i = 0; i < num_features; ++i) temp_vec_gsp.push_back(gsc_priors());
  for (unsigned int i = 0; i < h_c*w_c; ++i) vec_gsp_grid.push_back(temp_vec_gsp);
  
  coordinates_t temp_coord;
  for (unsigned int gr_h = 0 ; gr_h < h_c; ++gr_h){
    for (unsigned int gr_w = 0 ; gr_w < w_c; ++gr_w){
      temp_coord.x =  2*perception->roiExtent()*gr_w + perception->roiExtent();
      temp_coord.y =  2*perception->roiExtent()*gr_h + perception->roiExtent();
       if (gr_w==w_c-1) temp_coord.x--;
       if (gr_h==h_c-1) temp_coord.y--;
       grid_coordinates.push_back(temp_coord);
    }
  }
  
  return true;
}




bool bs_cluster_planner::loadModel(PredictorModel &model)
{
  return true;
}

bool bs_cluster_planner::saveModel(PredictorModel &model)
{
  return true;
}

