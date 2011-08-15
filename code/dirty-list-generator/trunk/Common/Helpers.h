/*	Dirty Mod List Generator

    Outputs a list of dirty mods and their ITM/UDR counts and CRCs,
	using information from the Better Oblivion Sorting Software masterlist.

	Written using code adapted from Better Oblivion Sorting Software.
	All code (new and adapted), authored by WrinklyNinja.

    Copyright (C) 2011  WrinklyNinja & the BOSS development team.
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2512 $, $Date: 2011-04-01 10:38:36 +0100 (Fri, 01 Apr 2011) $
*/

#ifndef __DLG_HELPERS__HPP__
#define __DLG_HELPERS__HPP__

#include <string>
#include "boost/filesystem.hpp"

namespace boss {
	using namespace std;
	namespace fs = boost::filesystem;

	//////////////////////////////////////////////////////////////////////////
	// Helper functions
	//////////////////////////////////////////////////////////////////////////

	//Changes uppercase to lowercase and removes preceding and trailing spaces.	
	string Tidy(string filename);

	//Converts an integer to a string using BOOST's Spirit.Karma. Faster than a stringstream conversion.
	string IntToString(unsigned int n);

	//Converts an integer to a hex string using BOOST's Spirit.Karma. Faster than a stringstream conversion.
	string IntToHexString(unsigned int n);

	//Reads an entire file into a string buffer.
	void fileToBuffer(const fs::path file, string& buffer);
}

#endif
