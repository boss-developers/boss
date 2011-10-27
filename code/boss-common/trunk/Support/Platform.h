/*	Better Oblivion Sorting Software
	
	A "one-click" program for users that quickly optimises and avoids 
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge, 
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

    Copyright (C) 2011  Random/Random007/jpearce, WrinklyNinja & the BOSS 
	development team. Copyright license:
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2488 $, $Date: 2011-03-27 14:31:33 +0100 (Sun, 27 Mar 2011) $
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
