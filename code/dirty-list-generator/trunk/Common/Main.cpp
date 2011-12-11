/*	Dirty Mod List Generator

    Outputs a list of dirty mods and their ITM/UDR counts and CRCs,
	using information from the Better Oblivion Sorting Software masterlist.

	Copyright (C) 2009-2011    BOSS Development Team.

	This file is part of Better Oblivion Sorting Software.

    Better Oblivion Sorting Software is free software: you can redistribute 
	it and/or modify it under the terms of the GNU General Public License 
	as published by the Free Software Foundation, either version 3 of 
	the License, or (at your option) any later version.

    Better Oblivion Sorting Software is distributed in the hope that it will 
	be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Better Oblivion Sorting Software.  If not, see 
	<http://www.gnu.org/licenses/>.

	$Revision: 2512 $, $Date: 2011-04-01 10:38:36 +0100 (Fri, 01 Apr 2011) $
*/

#define NOMINMAX // we don't want the dummy min/max macros since they overlap with the std:: algorithms

#include "BOSS-Common.h"
#include <boost/filesystem/detail/utf8_codecvt_facet.hpp>
#include <boost/algorithm/string.hpp>
#include <clocale>
#include <string>
#include <iostream>
#include <algorithm>

using namespace boss;
using namespace std;

using boost::algorithm::to_lower_copy;


//Save the formatted output list of dirty mods.
void SaveDirtyList(vector<Item> list, ofstream& out) {
	char x = ' ';

	for (size_t i=0; i<list.size(); i++) {
		if (list[i].name == list[i-1].name)
			continue;
		if (to_lower_copy(list[i].name.string())[0] != x) {
			out << endl;
			x = to_lower_copy(list[i].name.string())[0];
		}
		out << list[i].name.string() << "   ";
		for (vector<Message>::iterator messageIter = list[i].messages.begin(); messageIter != list[i].messages.end(); ++messageIter)
			out << messageIter->data;  //Print the mod name and message.

		out << endl;
	}
	return;
}

bool SortModsByName(Item mod1,Item mod2) {
        string n1, n2;
        n1 = to_lower_copy(mod1.name.string());
        n2 = to_lower_copy(mod2.name.string());
        return (n1 < n2);
}


int main() {
	ItemList masterlist, cleanlist, Dirtylist;
	ofstream dirtylist;
	const fs::path dirtylist_path		= "dirtylist.txt";
	const fs::path cleanlist_path		= "cleanlist.txt";

	//Set the locale to get encoding conversions working correctly.
	//Not sure if this is still needed, but better safe than sorry.
	setlocale(LC_CTYPE, "");
	locale global_loc = locale();
	locale loc(global_loc, new boost::filesystem::detail::utf8_codecvt_facet());
	boost::filesystem::path::imbue(loc);

	//Open output file.
	dirtylist.open(dirtylist_path.c_str());
	if (dirtylist.fail()) {
		cout << "Critical Error: \"" +dirtylist_path.string() +"\" cannot be opened for writing! Exiting." << endl;
	}

	//Check if it actually exists, because the parser doesn't fail if there is no file...
	if (!fs::exists(masterlist_path)) {
		//Print error message to console and exit.
		dirtylist << "Critical Error: \"" +masterlist_path.string() +"\" cannot be read! Exiting." << endl;
        exit (1); //fail in screaming heap.
    } else if (!fs::exists(cleanlist_path)) {
		//Print error message to console and exit.
		dirtylist << "Critical Error: \"" +cleanlist_path.string() +"\" cannot be read! Exiting." << endl;
        exit (1); //fail in screaming heap.
	}

	//Parse masterlist into data structure.
	try {
		masterlist.Load(masterlist_path);
		cleanlist.Load(cleanlist_path);
	} catch (boss_error e) {
		LOG_ERROR("Critical Error: %s", e.getString());
		 exit (1); //fail in screaming heap.
	}

	//Now we need to remove all the mods that are in cleanlist from masterlist. This would be faster using a hashset for cleanlist instead of an ItemList,
	//but speed is not of the essence.
	vector<Item>::iterator foundItem;
	for (vector<Item>::iterator itemIter = cleanlist.items.begin(); itemIter != cleanlist.items.end(); ++itemIter) {
		foundItem = masterlist.FindItem(itemIter->name);
		if (foundItem != masterlist.items.end())
			masterlist.items.erase(foundItem);
	}

	//Now we need to iterate through the masterlist and retain only those mods with dirty mod messages that aren't "Do not clean".
	//This should include SAY dirty mod messages too. If a message is conditional on certain CRCs, these should be listed in a
	//message that gets attached to the mod, with each CRC listed as "CRC hex". Otherwise, all messages should be removed.
	for (vector<Item>::iterator itemIter = masterlist.items.begin(); itemIter != masterlist.items.end(); ++itemIter) {
		if ((itemIter->type != MOD) || itemIter->messages.empty())
			itemIter = masterlist.items.erase(itemIter);
		else {
			bool keep = false;
			Message message;
			message.key = NONE;
			
			for (vector<Message>::iterator messageIter = itemIter->messages.begin(); messageIter != itemIter->messages.end(); ++messageIter) {
				if ((messageIter->key == DIRTY) || (messageIter->key == SAY && messageIter->data.find("Needs TES4Edit") != string::npos)) {
					keep = true;
					string data;

					size_t pos1 = messageIter->data.find(" records");  //Extract ITM/UDR counts from message.
					if (pos1 != string::npos)
						data += messageIter->data.substr(0,pos1);

					if (!messageIter->conditionals.empty()) {  //Extract CRCs from conditional.
						size_t pos2;
						pos1 = messageIter->data.find('(');
						while (pos1 != string::npos) {
							pos2 = messageIter->data.find('|');
							if (!data.empty())
								data += ", ";
							data += "CRC " + messageIter->data.substr(pos1+1, pos2-pos1-1);
							pos1 = messageIter->data.find('(', pos2);
						}
					}

					if (!data.empty()) {
						message.data += "[" + data + "] ";
						message.key = SAY;
					}
				}
			}
			if (!keep)
				itemIter = masterlist.items.erase(itemIter);
			else {
				itemIter->messages.clear();
				if (message.key == SAY)
					itemIter->messages.push_back(message);
			}
		}
	}
	
	//Now the masterlist contents should be sorted alphabetically.
	sort(masterlist.items.begin(),masterlist.items.end(), SortModsByName);

	//Finally, we output it as our dirtylist.
	SaveDirtyList(masterlist.items, dirtylist);
	dirtylist.close();
	return (0);
}