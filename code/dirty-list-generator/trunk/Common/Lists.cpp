/*	Dirty Mod List Generator

    Outputs a list of dirty mods and their ITM/UDR counts and CRCs,
	using information from the Better Oblivion Sorting Software masterlist.

	Written using code adapted from Better Oblivion Sorting Software.
	All code (new and adapted), authored by WrinklyNinja.

    Copyright (C) 2011  WrinklyNinja & the BOSS development team.
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2512 $, $Date: 2011-04-01 10:38:36 +0100 (Fri, 01 Apr 2011) $
*/

#include "Lists.h"
#include "Helpers.h"
#include "Helpers.h"
#include <boost/algorithm/string.hpp>

namespace boss {
	using namespace std;
	using boost::algorithm::to_lower_copy;

	ofstream dirtylist;

	string currentList = "master";

	vector<string> errorMessageBuffer;  //Holds any error messages generated during parsing for printing later.

	//Find a mod by name. Will also find the starting position of a group.
	bool ModInList(vector<item> list, fs::path filename) {
		for (size_t i=0; i<list.size(); i++) {
			if (Tidy(list[i].name.string()) == Tidy(filename.string()))  //Look for exact match.
				return true;
		}
		return false;
	}

    //Save the formatted output list of dirty mods.
	void SaveModlist(vector<item> list) {
		char x = ' ';

		//Iterate through items, printing out all group markers, mods and messsages.
		for (size_t i=0; i<list.size(); i++) {
			if (list[i].name == list[i-1].name)
				continue;
			if (to_lower_copy(list[i].name.string())[0] != x) {
				dirtylist << endl;
				x = to_lower_copy(list[i].name.string())[0];
			}
			dirtylist << list[i].name.string();  //Print the mod name.
			//Print the messages with the appropriate syntax.
			for (size_t j=0; j<list[i].messages.size(); j++) {
				if (list[i].messages[j].data != "")
					dirtylist << "  [" << list[i].messages[j].data << "]";  
			}
			dirtylist << endl;
		}
		return;
	}

	bool SortModsByName(item mod1,item mod2) {
		string n1, n2;
		n1 = to_lower_copy(mod1.name.string());
		n2 = to_lower_copy(mod2.name.string());
		return (n1 < n2);
	}
}