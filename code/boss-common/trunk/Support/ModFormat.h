/*	Better Oblivion Sorting Software
	
	A "one-click" program for users that quickly optimises and avoids 
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge, 
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

    Copyright (C) 2011 BOSS Development Team. Copyright license:
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2488 $, $Date: 2011-03-27 14:31:33 +0100 (Sun, 27 Mar 2011) $
*/

#ifndef __SUPPORT_MODFORMAT__HPP__
#define __SUPPORT_MODFORMAT__HPP__

#include "Types.h"
#include <cstring>
#include <boost/filesystem.hpp>


namespace boss {

	using namespace std;

	// Structure for grouping the information gathered from each mod's header.
	struct ModHeader {
		string		Name;
		string		Description;
		string		Author;
		string		Version;
		bool		IsMaster;
	};


	struct Record {
		static const ulong TES4	=	'4SET';
		static const ulong HEDR	=	'RDEH';
		static const ulong OFST	=	'TSFO';
		static const ulong DELE	=	'ELED';
		static const ulong CNAM	=	'MANC';
		static const ulong SNAM	=	'MANS';
	};

	ModHeader ReadHeader(boost::filesystem::path filename);
}

#endif
