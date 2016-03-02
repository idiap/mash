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


/** @file   platform.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Platform-specific declarations 
*/

#ifndef _MASH_PLATFORM_H_
#define _MASH_PLATFORM_H_

#define MASH_PLATFORM_WIN32 1
#define MASH_PLATFORM_LINUX 2
#define MASH_PLATFORM_APPLE 3

#if defined( __WIN32__ ) || defined( _WIN32 )
#   define MASH_PLATFORM MASH_PLATFORM_WIN32

#elif defined( __APPLE_CC__)
#   define MASH_PLATFORM MASH_PLATFORM_APPLE

#else
#   define MASH_PLATFORM MASH_PLATFORM_LINUX
#endif


/// Used to export symbols from library
#if MASH_PLATFORM == MASH_PLATFORM_WIN32
#	ifdef MASH_EXPORTS
#		define MASH_SYMBOL __declspec(dllexport)
#	else
#		define MASH_SYMBOL __declspec(dllimport)
#	endif
#else
#	define MASH_SYMBOL
#endif

#endif
