/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 3163 $, $Date: 2011-08-21 22:03:18 +0100 (Sun, 21 Aug 2011) $
*/

#ifndef __SUPPORT_HELPERS__HPP__
#define __SUPPORT_HELPERS__HPP__

#include "Types.h"
#include "Common/DllDef.h"

#include <cstring>
#include <iostream>
#include <boost/filesystem.hpp>

namespace boss {
	using namespace std;
	namespace fs = boost::filesystem;

	//////////////////////////////////////////////////////////////////////////
	// Helper functions
	//////////////////////////////////////////////////////////////////////////

	//Changes uppercase to lowercase and removes preceding and trailing spaces.	
	BOSS_COMMON string Tidy(string text);

	//Checks if a given object is an esp or an esm.
	bool IsPlugin(string object);
	
	//Checks if the plugin exists at the given location, even if ghosted.
	bool Exists(const fs::path plugin);

	//Determines if a given mod is a game's main master file or not.
	bool IsMasterFile(const string plugin);

	//Reads the header from mod file and prints a string representation which includes the version text, if found.
	string GetModHeader(const fs::path& filename);

	//Calculate the CRC of the given file for comparison purposes.
	unsigned int GetCrc32(const fs::path& filename);

	//Checks if the given plugin is ghosted in the user's install.
	//NOT if the plugin given has a '.ghost' extension.
	bool IsGhosted(fs::path plugin);

	//Gets the given OBSE dll or OBSE plugin dll's version number.
	//Also works for FOSE and NVSE.
	string GetExeDllVersion(const fs::path& filename);
}

#endif
