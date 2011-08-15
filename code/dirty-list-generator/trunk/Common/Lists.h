/*	Dirty Mod List Generator

    Outputs a list of dirty mods and their ITM/UDR counts and CRCs,
	using information from the Better Oblivion Sorting Software masterlist.

	Written using code adapted from Better Oblivion Sorting Software.
	All code (new and adapted), authored by WrinklyNinja.

    Copyright (C) 2011  WrinklyNinja & the BOSS development team.
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2512 $, $Date: 2011-04-01 10:38:36 +0100 (Fri, 01 Apr 2011) $
*/

#ifndef __DLG_LISTS_H__
#define __DLG_LISTS_H__

#include <string>
#include <vector>
#include "boost/filesystem.hpp"

namespace boss {
	using namespace std;
	namespace fs = boost::filesystem;

	extern ofstream dirtylist;

	extern string currentList;

	////////////////////////////////////////
	// General data structures
	////////////////////////////////////////

	enum keyType {
		NONE,
		//Masterlist keywords.
		SAY,
		TAG,
		REQ,
		INC,
		DIRTY,
		WARN,
		ERR,
		//Special masterlist keywords.
		OOOSAY,
		BCSAY,
		
	};

	extern vector<string> errorMessageBuffer;  //Holds any error messages generated during parsing for printing later.

	////////////////////////////////////////
	// Modlist/Masterlist data structures
	////////////////////////////////////////

	enum metaType {
		IF,
		IFNOT
	};

	enum itemType {
		MOD,
		BEGINGROUP,
		ENDGROUP
	};

	struct message {
		keyType key;
		string data;
	};

	struct item {
		itemType type;
		fs::path name;
		vector<message> messages;
	};

	//Save the formatted output list of dirty mods.
	void SaveModlist(vector<item> list);

	bool SortModsByName(item mod1,item mod2);

	bool ModInList(vector<item> list, fs::path filename);
}

#endif