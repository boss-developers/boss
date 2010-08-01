/*	Better Oblivion Sorting Software
	2.0 Beta
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    	Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    	http://creativecommons.org/licenses/by-nc-nd/3.0/

	 $Id: Types.h 1200 2010-07-29 22:51:09Z leandor@gmail.com $
	$URL: https://better-oblivion-sorting-software.googlecode.com/svn/BOSS%20source%20code/Support/Types.h $
*/

#ifndef __SUPPORT_TYPES__HPP__
#define __SUPPORT_TYPES__HPP__

namespace boss {

	//////////////////////////////////////////////////////////////////////////
	// Common type definitions
	//////////////////////////////////////////////////////////////////////////

	typedef unsigned long	ulong;
	typedef unsigned short	ushort;

	
	//////////////////////////////////////////////////////////////////////////
	// Constants
	//////////////////////////////////////////////////////////////////////////

	// An arbitraty large number. Controls the size of some buffers used to read data from files.
	const ulong MAXLENGTH = 4096UL;

};

#endif __SUPPORT_TYPES__HPP__