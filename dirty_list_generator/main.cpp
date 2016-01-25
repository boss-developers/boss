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

#define NOMINMAX  // We don't want the dummy min/max macros since they overlap with the std:: algorithms

#include <clocale>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/detail/utf8_codecvt_facet.hpp>

#include "api/boss.h"
#include "base/fstream.h"
#include "common/conditional_data.h"
#include "common/error.h"
#include "common/game.h"
#include "common/globals.h"
#include "common/item_list.h"
#include "common/keywords.h"
#include "support/logger.h"

// TODO(MCP): Split this up into its own wrapper file?
namespace boss {

// Save the formatted output list of dirty mods.
void SaveDirtyList(std::vector<Item> &list, boss::boss_fstream::ofstream &out, const uint64_t max_name_length) {
	std::string output;
	for (std::size_t i = 0; i < list.size(); i++) {
		std::string data;
		data += list[i].Name();
		if (list[i].Messages().size() == 0)
			data += '\n';
		std::vector<Message> messages = list[i].Messages();
		// TODO(MCP): Convert this to a for-each loop?
		for (std::vector<Message>::iterator messageIter = messages.begin();
		     messageIter != messages.end(); ++messageIter) {
			if (list[i].Name().size() < (max_name_length + 3))
				data.insert(data.size(), max_name_length - list[i].Name().size() + 3, ' ');
			if (messageIter != messages.begin())
				data.insert(data.size(), list[i].Name().size(), ' ');
			data += messageIter->Data();  // Print the mod name and message.
			if (messageIter != messages.end() || messageIter->Data().empty()) {
				data += '\n';
			}
		}
		output += data;
	}
	out << output;
}

int DirtyListGeneratorMain() {
	namespace fs = boost::filesystem;
	Game game;
	std::vector<Item> masterlist;
	boss::boss_fstream::ofstream dirtylist;
	const fs::path dirtylist_path = "dirtylist.txt";

	// Set the locale to get encoding conversions working correctly.
	// Not sure if this is still needed, but better safe than sorry.
	std::setlocale(LC_CTYPE, "");
	std::locale global_loc = std::locale();
	std::locale loc(global_loc, new fs::detail::utf8_codecvt_facet());
	fs::path::imbue(loc);

	// Game checks.
	LOG_DEBUG("Detecting game...");
	try {
		gl_last_game = AUTODETECT;  // Clear this setting in case the GUI was run.
		std::vector<std::uint32_t> detected, undetected;
		std::uint32_t detectedGame = DetectGame(detected, undetected);
		if (detectedGame == AUTODETECT) {
			// Now check what games were found.
			if (detected.empty()) {
				throw boss_error(BOSS_ERROR_NO_GAME_DETECTED);
			} else if (detected.size() == 1) {
				detectedGame = detected.front();
			} else {
				std::size_t ans;
				// Ask user to choose game.
				std::cout << std::endl << "Please pick which game to run BOSS for:" << std::endl;
				for (std::size_t i = 0; i < detected.size(); i++)
					std::cout << i << " : " << Game(detected[i], "", true).Name() << std::endl;

				std::cin >> ans;
				if (ans < 0 || ans >= detected.size()) {
					std::cout << "Invalid selection." << std::endl;
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
		std::exit(1);  // Fail in screaming heap.
	}

	// Open output file.
	dirtylist.open(dirtylist_path.c_str());
	if (dirtylist.fail()) {
		std::cout << "Critical Error: \"" + dirtylist_path.string() + "\" cannot be opened for writing! Exiting." << std::endl;
	}

	// Check if it actually exists, because the parser doesn't fail if there is no file...
	if (!fs::exists(game.Masterlist())) {
		// Print error message to console and exit.
		std::cout << "Critical Error: \"" + game.Masterlist().string() + "\" cannot be read! Exiting." << std::endl;
		std::exit(1);  // Fail in screaming heap.
	}

	// Parse masterlist into data structure.
	try {
		ItemList Masterlist, Cleanlist;
		Masterlist.Load(game, game.Masterlist());

		masterlist = Masterlist.Items();
	} catch (boss_error &e) {
		LOG_ERROR("Critical Error: %s", e.getString());
		std::exit(1);  // Fail in screaming heap.
	}

	// Now we need to remove all the mods that are in cleanlist from masterlist. This would be faster using a hashset for cleanlist instead of an ItemList,
	// but speed is not of the essence.
	std::vector<Item> dirty_mods;
	uint64_t max_name_length = 0;
	for (std::vector<Item>::iterator itemIter = masterlist.begin(); itemIter != masterlist.end(); ++itemIter) {
		if ((itemIter->Type() == MOD) && !itemIter->Messages().empty()) {
			bool keep = false;
			Message message;
			std::vector<Message> messages = itemIter->Messages();
			std::vector<Message> dirty_messages;
			for (std::vector<Message>::iterator messageIter = messages.begin();
			     messageIter != messages.end(); ++messageIter) {
				if ((messageIter->Key() == DIRTY) ||
				    (messageIter->Key() == SAY &&
				     messageIter->Data().find("Needs TES4Edit") != std::string::npos)) {
					if (messageIter->Data().find("Do not clean. \"Dirty\" edits are intentional and required for the mod to function.") != std::string::npos) {
						keep = false;
						break;
					}
					keep = true;
					std::string data;

					std::size_t pos1 = messageIter->Data().find(" records");  // Extract ITM/UDR counts from message.
					if (pos1 != std::string::npos) {
						data += messageIter->Data().substr(0, pos1);
					}

					if (!messageIter->Conditions().empty()) {  // Extract CRCs from conditional.
						std::size_t pos2;
						pos1 = messageIter->Conditions().find_last_of(',') + 1;
						pos2 = messageIter->Conditions().find_last_of(')');
						if (!data.empty())
							data += ", ";
						data += "CRC " + messageIter->Conditions().substr(pos1, pos2 - pos1);
					}

					if (!data.empty()) {
						message.Data("[" + data + "]");
						message.Key(SAY);
						dirty_messages.push_back(message);
					}
				}
			}
			if (keep && message.Key() == SAY) {
				std::cout << "Added: " << itemIter->Name() << std::endl;
				if (itemIter->Name().size() > max_name_length)
					max_name_length = itemIter->Name().size();
				dirty_mods.push_back(Item(itemIter->Name(), MOD, dirty_messages));
			}
		}
	}
	masterlist.clear();
	SaveDirtyList(dirty_mods, dirtylist, max_name_length);
	dirtylist.close();
	return 0;
}

}  // namespace boss

int main() {
	return boss::DirtyListGeneratorMain();
}
