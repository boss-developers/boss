/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

//Contains the modlist (inc. masterlist) and userlist data structures, and some helpful functions for dealing with them.
//DOES NOT CONTAIN ANYTHING TO DO WITH THE FILE PARSERS.

#ifndef __BOSS_LISTS_H__
#define __BOSS_LISTS_H__

#ifndef BOOST_FILESYSTEM_VERSION
#define BOOST_FILESYSTEM_VERSION 3
#endif

#ifndef BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_NO_DEPRECATED
#endif

#include <string>
#include <vector>
#include "boost/filesystem.hpp"

namespace boss {
	using namespace std;
	namespace fs = boost::filesystem;


	//modlist and masterlist data structure
	enum itemType {
		MOD,
		GROUPBEGIN,
		GROUPEND
	};

	struct message {
		string key;
		string data;
	};

	//items can be mods, group beginning markers and group end markers. All three have a type and a name, but the messages vector is only 
	//non-zero size if the item is a mod with messages attached.
	struct item {
		itemType type;
		fs::path name;
		vector<message> messages;
	};

	struct modlist {
		vector<item> items;
	};

	enum keyType {
		//Userlist keywords.
		ADD = 1,
		OVERRIDE,
		FOR,
		BEFORE,
		AFTER,
		TOP,
		BOTTOM,
		APPEND,
		REPLACE,
	};

	//Userlist data structure.
	struct line {
		keyType key;
		std::string object;
	};

	struct rule {
		keyType ruleKey;
		std::string ruleObject;
		std::vector<line> lines;
	};


	//Change back to being a struct with items later
	struct userlist {
		std::vector<rule> rules;
	};

	//Helpful functions.

	//Find a mod by name. Will also find the starting position of a group.
	int GetModPos(modlist list, string filename);

	//Find the end of a group by name.
	int GetGroupEndPos(modlist list, string groupname);

	//Date comparison, used for sorting mods in modlist.
	bool SortModsByDate(item mod1, item mod2);

	//Build modlist (the one that gets saved to file, not the masterlist).
	//Adds mods in directory to modlist in date order (AKA load order).
	void BuildModlist(modlist &list);

	//Save the modlist (not masterlist) to a file, printing out all the information in the data structure.
	//This will likely just be one group and the list of filenames in that group, if it's the modlist.
	//However, if used on the masterlist, could prove useful for debugging the parser.
	void SaveModlist(modlist list, fs::path file);
}

#endif