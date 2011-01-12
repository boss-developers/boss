/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision$, $Date$
*/

#ifndef __SUPPORT_MODFORMAT__HPP__
#define __SUPPORT_MODFORMAT__HPP__

#define BOOST_FILESYSTEM_VERSION 3
#define BOOST_FILESYSTEM_NO_DEPRECATED

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
