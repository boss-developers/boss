/*	Better Oblivion Sorting Software
	
	A "one-click" program for users that quickly optimises and avoids 
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge, 
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

    Copyright (C) 2011 BOSS Development Team. Copyright license:
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 3163 $, $Date: 2011-08-21 22:03:18 +0100 (Sun, 21 Aug 2011) $
*/

#ifndef __SUPPORT_HELPERS__HPP__
#define __SUPPORT_HELPERS__HPP__

#include "Types.h"
#include "Common/DllDef.h"
#include "Common/Classes.h"

#include <cstring>
#include <iostream>
#include <boost/filesystem.hpp>

namespace boss {
	using namespace std;
	namespace fs = boost::filesystem;

	//////////////////////////////////////////////////////////////////////////
	// Helper functions
	//////////////////////////////////////////////////////////////////////////

	//Changes uppercase to lowercase.	
	BOSS_COMMON_EXP string Tidy(string text);

	//Calculate the CRC of the given file for comparison purposes.
	unsigned int GetCrc32(const fs::path& filename);

	//Gets the given OBSE dll or OBSE plugin dll's version number.
	//Also works for FOSE and NVSE.
	string GetExeDllVersion(const fs::path& filename);

	//Reads an entire file into a string buffer.
	void fileToBuffer(const fs::path file, string& buffer);

	//UTF-8 file validator.
	bool ValidateUTF8File(const fs::path file);

	//Converts an integer to a string using BOOST's Spirit.Karma. Faster than a stringstream conversion.
	BOSS_COMMON_EXP string IntToString(const unsigned int n);

	//Converts an integer to a hex string using BOOST's Spirit.Karma. Faster than a stringstream conversion.
	string IntToHexString(const unsigned int n);

	//Converts a boolean to a string representation (true/false)
	string BoolToString(bool b);
}

#endif
