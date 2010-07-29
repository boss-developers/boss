/*	Better Oblivion Sorting Software
	2.0 Beta
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    	Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    	http://creativecommons.org/licenses/by-nc-nd/3.0/

	 $Id$
	$URL$
*/

#ifndef __SUPPORT_MODFORMAT__HPP__
#define __SUPPORT_MODFORMAT__HPP__

#include <cstring>

#include <Support/Types.h>

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

	ModHeader ReadHeader(string filename);
};

#endif __SUPPORT_MODFORMAT__HPP__