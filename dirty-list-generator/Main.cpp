/*	Dirty Mod List Generator

    Outputs a list of dirty mods and their ITM/UDR counts and CRCs,
	using information from the BOSS masterlist.

	Copyright (C) 2009-2011    BOSS Development Team.

	This file is part of BOSS.

    BOSS is free software: you can redistribute 
	it and/or modify it under the terms of the GNU General Public License 
	as published by the Free Software Foundation, either version 3 of 
	the License, or (at your option) any later version.

    BOSS is distributed in the hope that it will 
	be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with BOSS.  If not, see 
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
		if (list[i].Name() == list[i-1].Name())
			continue;
		if (to_lower_copy(list[i].Name())[0] != x) {
			out << endl;
			x = to_lower_copy(list[i].Name())[0];
		}
		out << list[i].Name() << "   ";
		vector<Message> messages = list[i].Messages();
		for (vector<Message>::iterator messageIter = messages.begin(); messageIter != messages.end(); ++messageIter)
			out << messageIter->Data();  //Print the mod name and message.

		out << endl;
	}
	return;
}

bool SortModsByName(Item mod1,Item mod2) {
        string n1, n2;
        n1 = to_lower_copy(mod1.Name());
        n2 = to_lower_copy(mod2.Name());
        return (n1 < n2);
}


int main() {
	Game game;
	vector<Item> masterlist, cleanlist;
	ofstream dirtylist;
	const fs::path dirtylist_path		= "dirtylist.txt";
	const fs::path cleanlist_path		= "cleanlist.txt";

	//Set the locale to get encoding conversions working correctly.
	//Not sure if this is still needed, but better safe than sorry.
	setlocale(LC_CTYPE, "");
	locale global_loc = locale();
	locale loc(global_loc, new boost::filesystem::detail::utf8_codecvt_facet());
	boost::filesystem::path::imbue(loc);

	//Game checks.
	LOG_DEBUG("Detecting game...");
	try {
		gl_last_game = AUTODETECT;  //Clear this setting in case the GUI was run.
		vector<uint32_t> detected, undetected;
		uint32_t detectedGame = DetectGame(detected, undetected);
		if (detectedGame == AUTODETECT) {
			//Now check what games were found.
			if (detected.empty())
				throw boss_error(BOSS_ERROR_NO_GAME_DETECTED);
			else if (detected.size() == 1)
				detectedGame = detected.front();
			else {
				size_t ans;
				//Ask user to choose game.
				cout << endl << "Please pick which game to run BOSS for:" << endl;
				for (size_t i=0; i < detected.size(); i++)
					cout << i << " : " << Game(detected[i], "", true).Name() << endl;

				cin >> ans;
				if (ans < 0 || ans >= detected.size()) {
					cout << "Invalid selection." << endl;
					throw boss_error(BOSS_ERROR_NO_GAME_DETECTED);
				}
				detectedGame = detected[ans];
			}
		}
		game = Game(detectedGame);
		game.CreateBOSSGameFolder();
		LOG_INFO("Game detected: %s", game.Name().c_str());
	} catch (boss_error &e) {
		LOG_ERROR("Critical Error: %s", e.getString().c_str());
		exit (1); //fail in screaming heap.
	}

	//Open output file.
	dirtylist.open(dirtylist_path.c_str());
	if (dirtylist.fail()) {
		cout << "Critical Error: \"" +dirtylist_path.string() +"\" cannot be opened for writing! Exiting." << endl;
	}

	//Check if it actually exists, because the parser doesn't fail if there is no file...
	if (!fs::exists(game.Masterlist())) {
		//Print error message to console and exit.
		dirtylist << "Critical Error: \"" +game.Masterlist().string() +"\" cannot be read! Exiting." << endl;
        exit (1); //fail in screaming heap.
    } else if (!fs::exists(cleanlist_path)) {
		//Print error message to console and exit.
		dirtylist << "Critical Error: \"" +cleanlist_path.string() +"\" cannot be read! Exiting." << endl;
        exit (1); //fail in screaming heap.
	}

	//Parse masterlist into data structure.
	try {
		ItemList Masterlist, Cleanlist;
		Masterlist.Load(game, game.Masterlist());
		Cleanlist.Load(game, cleanlist_path);

		masterlist = Masterlist.Items();
		cleanlist = Cleanlist.Items();
	} catch (boss_error &e) {
		LOG_ERROR("Critical Error: %s", e.getString());
		 exit (1); //fail in screaming heap.
	}

	//Now we need to remove all the mods that are in cleanlist from masterlist. This would be faster using a hashset for cleanlist instead of an ItemList,
	//but speed is not of the essence.
	size_t pos;
	ItemList tempItemList;
	tempItemList.Items(masterlist);
	for (vector<Item>::iterator itemIter = cleanlist.begin(); itemIter != cleanlist.end(); ++itemIter) {
		pos = tempItemList.FindItem(itemIter->Name(), MOD);
		if (pos != tempItemList.Items().size())
			tempItemList.Erase(pos);
	}
	masterlist = tempItemList.Items();

	//Now we need to iterate through the masterlist and retain only those mods with dirty mod messages that aren't "Do not clean".
	//This should include SAY dirty mod messages too. If a message is conditional on certain CRCs, these should be listed in a
	//message that gets attached to the mod, with each CRC listed as "CRC hex". Otherwise, all messages should be removed.
	vector<Item> holdingVec;
	for (vector<Item>::iterator itemIter = masterlist.begin(); itemIter != masterlist.end(); ++itemIter) {
		if ((itemIter->Type() == MOD) && !itemIter->Messages().empty()) {
			bool keep = false;
			Message message;
			
			vector<Message> messages = itemIter->Messages();
			for (vector<Message>::iterator messageIter = messages.begin(); messageIter != messages.end(); ++messageIter) {
				if ((messageIter->Key() == DIRTY) || (messageIter->Key() == SAY && messageIter->Data().find("Needs TES4Edit") != string::npos)) {
					keep = true;
					string data;

					size_t pos1 = messageIter->Data().find(" records");  //Extract ITM/UDR counts from message.
					if (pos1 != string::npos)
						data += messageIter->Data().substr(0,pos1);

					if (!messageIter->Conditions().empty()) {  //Extract CRCs from conditional.
						size_t pos2;
						pos1 = messageIter->Data().find('(');
						while (pos1 != string::npos) {
							pos2 = messageIter->Data().find('|');
							if (!data.empty())
								data += ", ";
							data += "CRC " + messageIter->Data().substr(pos1+1, pos2-pos1-1);
							pos1 = messageIter->Data().find('(', pos2);
						}
					}

					if (!data.empty()) {
						message.Data(message.Data() + "[" + data + "] ");
						message.Key(SAY);
					}
				}
			}
			if (keep && message.Key() == SAY)
				holdingVec.push_back(Item(itemIter->Name(), MOD, messages));
		}
	}
	
	//Now the masterlist contents should be sorted alphabetically.
	sort(holdingVec.begin(), holdingVec.end(), SortModsByName);

	//Finally, we output it as our dirtylist.
	SaveDirtyList(holdingVec, dirtylist);
	dirtylist.close();
	return (0);
}