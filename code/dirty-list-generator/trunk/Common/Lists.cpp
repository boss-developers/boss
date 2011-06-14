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
#include <boost/algorithm/string.hpp>

namespace boss {
	using namespace std;
	using boost::algorithm::to_lower_copy;

	vector<string> errorMessageBuffer;  //Holds any error messages generated during parsing for printing later.

    //Save the formatted output list of dirty mods.
	void SaveModlist(vector<item> list, fs::path file) {
		ofstream ofile;
		char x;

		//Open output file.
		ofile.open(file.c_str());
		if (ofile.fail()) //Provide error message if it can't be written.
			cout << "File couldn't be saved." << endl;

		//Iterate through items, printing out all group markers, mods and messsages.
		for (size_t i=0; i<list.size(); i++) {
			if (list[i].name == list[i-1].name)
				continue;
			if (to_lower_copy(list[i].name.string())[0] != x) {
				ofile << endl;
				x = to_lower_copy(list[i].name.string())[0];
			}
			ofile << list[i].name.string();  //Print the mod name.
			//Print the messages with the appropriate syntax.
			for (size_t j=0; j<list[i].messages.size(); j++) {
				if (list[i].messages[j].data != "")
					ofile << "  [" << list[i].messages[j].data << "]";  
			}
			ofile << endl;
		}

		ofile.close();
		return;
	}

	bool SortModsByName(item mod1,item mod2) {
		string n1, n2;
		n1 = to_lower_copy(mod1.name.string());
		n2 = to_lower_copy(mod2.name.string());
		return (n1 < n2);
	}
}