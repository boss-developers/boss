/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision$, $Date$
*/

//This is the start of implementing userlist and modlist classes into trunk. 
//Starting with only the modlist stuff.

#ifndef __BOSS_USERLIST_H__
#define __BOSS_USERLIST_H__


#include "Globals.h"

#include <string>
#include <fstream>
#include <vector>

namespace boss {
	using namespace std;

	//Class to store userlist rules.
	class Rules {
	public:
		vector<string> keys,objects;			//Holds keys and objects for each rule line.
		vector<int> rules;						//Tells BOSS where each rule starts.
		string messages;						//Stores output messages.
		void AddRules();						//Populates object vectors with rules from userlist.txt.
		void PrintMessages(ofstream& output);	//Prints the output messages.
	};
}
#endif
