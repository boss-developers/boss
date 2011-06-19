/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

//Contains the modlist (inc. masterlist) and userlist data structures, and some helpful functions for dealing with them.

#ifndef __BOSS_LISTS_H__
#define __BOSS_LISTS_H__

#include <string>
#include <vector>
#include "boost/filesystem.hpp"

namespace boss {
	using namespace std;
	namespace fs = boost::filesystem;

	////////////////////////////////////////
	// General data structures
	////////////////////////////////////////

	enum keyType {
		NONE,
		//Userlist keywords.
		ADD,
		OVERRIDE,
		FOR,
		BEFORE,
		AFTER,
		TOP,
		BOTTOM,
		APPEND,
		REPLACE,
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

	extern vector<string> userlistErrorBuffer;  //Holds any error messages generated during parsing for printing later.
	extern vector<string> masterlistErrorBuffer;  //Holds any error messages generated during parsing for printing later.
	extern vector<string> iniErrorBuffer;  //Holds any error messages generated during parsing for printing later.

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

	extern vector<message> globalMessageBuffer;  //Holds any global messages from the masterlist to be printed in BOSS.

	////////////////////////////////////////
	// Userlist data structures
	////////////////////////////////////////

	//Userlist data structure.
	struct line {
		keyType key;
		string object;
	};

	struct rule {
		keyType ruleKey;
		string ruleObject;
		vector<line> lines;
	};

	////////////////////////////////////////
	// Helpful functions
	////////////////////////////////////////

	//Find a mod by name. Will also find the starting position of a group.
	size_t GetModPos(vector<item> modList, string filename);

	//Find the end of a group by name.
	size_t GetGroupEndPos(vector<item> modList, string groupname);

	//Date comparison, used for sorting mods in modlist.
	bool SortModsByDate(item mod1, item mod2);

	//Adds mods in directory to modlist in date order (AKA load order).
	void BuildModlist(vector<item> &modList);

	//Save the modlist (or masterlist) to a file, printing out all the information in the data structure.
	void SaveModlist(vector<item> modList, fs::path file);

	//Returns a string representation of the given keyType.
	//Possibly a better way to do this.
	string KeyToString(keyType key);

	//Returns a keyType representation of the given key string.
	//Possibly a better way to do this.
	keyType StringToKey(string key);
}

#endif