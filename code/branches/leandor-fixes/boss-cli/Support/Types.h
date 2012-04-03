/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 1767 $, $Date: 2010-10-30 18:46:25 +0100 (Sat, 30 Oct 2010) $
*/

#ifndef __SUPPORT_TYPES__HPP__
#define __SUPPORT_TYPES__HPP__


namespace boss {

	//////////////////////////////////////////////////////////////////////////
	// Common type definitions
	//////////////////////////////////////////////////////////////////////////

	typedef unsigned long	ulong;
	typedef unsigned int	uint;
	typedef unsigned short	ushort;

	
	//////////////////////////////////////////////////////////////////////////
	// Constants
	//////////////////////////////////////////////////////////////////////////

	// An arbitrary large number. Controls the size of some buffers used to read data from files.
	const ulong MAXLENGTH = 4096UL;
}

#endif
