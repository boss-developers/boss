/*	Dirty Mod List Generator

    Outputs a list of dirty mods and their ITM/UDR counts and CRCs,
	using information from the Better Oblivion Sorting Software masterlist.

	Written using code adapted from Better Oblivion Sorting Software.
	All code (new and adapted), authored by WrinklyNinja.

    Copyright (C) 2011  WrinklyNinja & the BOSS development team.
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2512 $, $Date: 2011-04-01 10:38:36 +0100 (Fri, 01 Apr 2011) $
*/

//Contains functions for modlist/masterlist parsing.

#ifndef __DLG_PARSER_H__
#define __DLG_PARSER_H__

#include <vector>
#include "boost/filesystem.hpp"
#include "Common/Lists.h"

namespace boss {
	using namespace std;
	namespace fs = boost::filesystem;

	//Parses the given masterlist into the given data structure. Also works for the modlist.
	bool parseMasterlist(fs::path file, vector<item>& modList);
}
#endif