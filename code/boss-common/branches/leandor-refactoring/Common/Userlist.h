/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 1284 $, $Date: 2010-08-05 03:43:59 +0100 (Thu, 05 Aug 2010) $
*/

//This is the start of implementing userlist and modlist classes into trunk. 
//Starting with only the modlist stuff.

#ifndef __BOSS_USERLIST_H__
#define __BOSS_USERLIST_H__

#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <boost/filesystem.hpp>
#include <Support/Helpers.h>

namespace boss {
	using namespace std;
		
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
		int GetRuleIndex(string object, string key);		//Finds the rule line which references the given object.
	};
}

#endif __BOSS_USERLIST_H__