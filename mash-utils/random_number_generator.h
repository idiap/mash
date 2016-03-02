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


/** @file   random_number_generator.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'RandomNumberGenerator' class
*/

#ifndef _MASH_RANDOMNUMBERGENERATOR_H_
#define _MASH_RANDOMNUMBERGENERATOR_H_

#include "platform.h"

namespace Mash {

    //------------------------------------------------------------------------------------
    /// @brief  Random number generator
    ///
    /// See http://en.wikipedia.org/wiki/Linear_congruential_generator
    //------------------------------------------------------------------------------------
    class MASH_SYMBOL RandomNumberGenerator
    {
        //_____ Construction / Destruction __________
    public:
        //--------------------------------------------------------------------------------
        /// @brief  Constructor, the seed is derived from the current time
        //--------------------------------------------------------------------------------
        RandomNumberGenerator();

        //--------------------------------------------------------------------------------
        /// @brief  Destructor
        //--------------------------------------------------------------------------------
        ~RandomNumberGenerator();


        //_____ Methods __________
    public:
        //--------------------------------------------------------------------------------
        /// @brief  Sets the seed of the random number generation
        //--------------------------------------------------------------------------------
        void setSeed(unsigned int uiSeed);

        //--------------------------------------------------------------------------------
        /// @brief  Returns the seed of the random number generation
        //--------------------------------------------------------------------------------
        inline unsigned int seed() const
        {
            return m_uiSeed;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Resets the generator to its initial seed
        //--------------------------------------------------------------------------------
        void reset();
        
        //--------------------------------------------------------------------------------
        /// @brief  Returns a number in the interval [0, max]
        //--------------------------------------------------------------------------------
        unsigned int randomize(unsigned int max);

        //--------------------------------------------------------------------------------
        /// @brief  Returns a number in the interval [min, max]
        //--------------------------------------------------------------------------------
        unsigned int randomize(unsigned int min, unsigned int max);

        //--------------------------------------------------------------------------------
        /// @brief  Returns a number in the interval [0, max]
        //--------------------------------------------------------------------------------
        int randomize(int max);

        //--------------------------------------------------------------------------------
        /// @brief  Returns a number in the interval [min, max]
        //--------------------------------------------------------------------------------
        int randomize(int min, int max);

        //--------------------------------------------------------------------------------
        /// @brief  Returns a number in the interval [0, max[
        //--------------------------------------------------------------------------------
        float randomize(float max);

        //--------------------------------------------------------------------------------
        /// @brief  Returns a number in the interval [min, max[
        //--------------------------------------------------------------------------------
        float randomize(float min, float max);
        
        //--------------------------------------------------------------------------------
        /// @brief  Returns a number in the interval [0, (2^32)-1]
        //--------------------------------------------------------------------------------
        unsigned int randomize();


        //_____ Attributes __________
    private:
        unsigned int m_uiSeed;
        unsigned int m_uiCurrent;
    };
}

#endif
