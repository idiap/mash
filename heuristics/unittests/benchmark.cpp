/*******************************************************************************
* The MASH Framework contains the source code of all the servers in the
* "computation farm" of the MASH project (http://www.mash-project.eu),
* developed at the Idiap Research Institute (http://www.idiap.ch).
*
* Copyright (c) 2016 Idiap Research Institute, http://www.idiap.ch/
* Written by Philip Abbet (philip.abbet@idiap.ch)
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


/** Author: Philip Abbet (philip.abbet@idiap.ch)

    This heuristic is used to benchmark the performance of the system
*/

#include <mash/heuristic.h>

using namespace Mash;


//------------------------------------------------------------------------------
/// The 'Identity' heuristic class
//------------------------------------------------------------------------------
class BenchmarkHeuristic: public Heuristic
{
    //_____ Construction / Destruction __________
public:
    BenchmarkHeuristic();
    virtual ~BenchmarkHeuristic();


    //_____ Implementation of Heuristic __________
public:
    virtual unsigned int dim();

    virtual scalar_t computeFeature(unsigned int feature_index);
};


//------------------------------------------------------------------------------
/// Creation function of the heuristic
//------------------------------------------------------------------------------
extern "C" Heuristic* new_heuristic()
{
    return new BenchmarkHeuristic();
}



/************************* CONSTRUCTION / DESTRUCTION *************************/

BenchmarkHeuristic::BenchmarkHeuristic()
{
}


BenchmarkHeuristic::~BenchmarkHeuristic()
{
}


/************************* IMPLEMENTATION OF Heuristic ************************/

unsigned int BenchmarkHeuristic::dim()
{
    return 10000;
}


scalar_t BenchmarkHeuristic::computeFeature(unsigned int feature_index)
{
    return 0.0f;
}
