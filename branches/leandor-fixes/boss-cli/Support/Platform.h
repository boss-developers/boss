/* 
 * Platform.h
 *
 * Better Oblivion Sorting Software
 *
 * Quick and Dirty Load Order Utility
 * (Making C++ look like the scripting language it isn't.)
 *
 * Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
 * http://creativecommons.org/licenses/by-nc-nd/3.0/
 *
 * $Revision: 1782 $, $Date: 2010-10-31 21:45:54 +0000 (Sun, 31 Oct 2010) $
 */

#ifndef __SUPPORT_PLATFORM__HPP__
#define __SUPPORT_PLATFORM__HPP__

#if (_WIN32 || _WIN64)

// windows-specific stuff

// for specifying size_t parameters to printf-like functions
# define PRIuS "Iu"

// ignore gnu __attribute__ specifiers
# define __attribute__(x)

#else
// everything other than windows
# define PRIuS "zu"
#endif

#endif // __SUPPORT_PLATFORM__HPP__
