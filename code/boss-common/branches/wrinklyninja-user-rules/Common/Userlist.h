/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 1284 $, $Date: 2010-08-05 03:43:59 +0100 (Thu, 05 Aug 2010) $
*/

/*	This file is my playground for working on userlist implementation, though it's also got a WIP modlist class too
	that will hopefully take over from the current implementation soon too (it just needs to list files in date order now)
*/

#ifndef __BOSS_USERLIST_H__
#define __BOSS_USERLIST_H__

#include <string>
#include <fstream>
#include <vector>
#include "boost/filesystem.hpp"
#include <Support/Helpers.h>

namespace boss {
	using namespace std;

	//Date comparison, used for sorting mods in modlist class.
	bool SortByDate(string mod1,string mod2);

	//Checks if a given object is an esp, an esm or a ghosted mod.
	bool IsPlugin(string object);
	
	//Class to store userlist rules.
	class Rules {
	public:
		vector<string> keys,objects;			//Holds keys and objects for each rule line.
		vector<int> rules;						//Tells BOSS where each rule starts.
		string messages;						//Stores output messages.
		void AddRules();						//Populates object vectors with rules from userlist.txt.
		void PrintRules(ofstream& output);		//Debug function, prints rules to output file stream.
		void PrintMessages(ofstream& output);	//Prints the output messages.
		int GetRuleIndex(string object);		//Finds the rule line which references the given object.
	};

	//Class to store and use modlist.
	class Mods {
	public:
		vector<string> mods;					//Stores the mods in your data folder.
		vector<vector<string>> modmessages;		//Stores the messages attached to each mod. First dimension matches up with the mods vector, then second lists the messages attached to that mod.
		void AddMods();							//Adds mods in directory to modlist in date order (AKA load order).
		void PrintModList(ofstream& out);		//Debug output function.
		int SaveModList();						//Save mod list to modlist.txt. Backs up old modlist.txt as modlist.old first.
		int GetModIndex(string mod);			//Look for a mod in the modlist, even if the mod in question is ghosted.
	};
}

#endif __BOSS_USERLIST_H__