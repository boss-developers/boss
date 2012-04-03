/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 1767 $, $Date: 2010-10-30 18:46:25 +0100 (Sat, 30 Oct 2010) $
*/

//This is the start of implementing userlist and modlist classes into trunk. 
//Starting with only the modlist stuff.

#ifndef __BOSS_USERLIST_H__
#define __BOSS_USERLIST_H__


#include "Globals.h"
#include "Support/Helpers.h"

#include <string>
#include <fstream>
#include <vector>
#include "boost/filesystem.hpp"


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
		void PrintMessages(ofstream& output);	//Prints the output messages.
	};

	//Class to store and use modlist.
	class Mods {
	public:
		vector<string> mods;					//Stores the mods in your data folder.
		vector< vector<string> > modmessages;	//Stores the messages attached to each mod. First dimension matches up with the mods vector, then second lists the messages attached to that mod.
		void AddMods();							//Adds mods in directory to modlist in date order (AKA load order).
		int SaveModList();						//Save mod list to modlist.txt. Backs up old modlist.txt as modlist.old first.
		int GetModIndex(string mod);			//Look for a mod in the modlist, even if the mod in question is ghosted.
	};
}

#endif
