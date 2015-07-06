/*	BOSS

	A "one-click" program for users that quickly optimises and avoids
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge,
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

	Copyright (C) 2009-2012    BOSS Development Team.

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

	$Revision: 3135 $, $Date: 2011-08-17 22:01:17 +0100 (Wed, 17 Aug 2011) $
*/

#include "common/item_list.h"

#include <cstddef>
#include <cstdint>
#include <ctime>

#include <algorithm>  // For sort function; not sure if right header as it wasn't included before but none of the included headers seem to define the sort function
#include <fstream>
#include <ostream>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include "common/conditional_data.h"
#include "common/error.h"
#include "common/game.h"
#include "common/globals.h"
#include "common/keywords.h"
#include "parsing/grammar.h"
#include "support/helpers.h"
#include "support/logger.h"
#include "support/platform.h"

namespace boss {

namespace fs = boost::filesystem;

//////////////////////////////
// ItemList Class Functions
//////////////////////////////

struct itemComparator {
	const Game& parentGame;
	itemComparator(const Game& game) : parentGame(game) {}

	bool operator() (const Item item1, const Item item2) {
		// Return true if item1 goes before item2, false otherwise.
		// Master files should go before other files.
		// Groups should not change position (but master files should be able to cross groups).

		bool isItem1MasterFile = item1.IsMasterFile(parentGame);
		bool isItem2MasterFile = item2.IsMasterFile(parentGame);

		if (isItem1MasterFile && !isItem2MasterFile) {
			return true;
		} else if (parentGame.GetLoadOrderMethod() == LOMETHOD_TIMESTAMP) {
			if (!isItem1MasterFile && isItem2MasterFile)
				return false;
			return (difftime(item1.GetModTime(parentGame),
			        item2.GetModTime(parentGame)) < 0);
		}
		return false;
	}
};

ItemList::ItemList() : lastRecognisedPos(0) {}

void ItemList::Load(const Game& parentGame, const fs::path path) {
	Clear();
	if (fs::exists(path) && fs::is_directory(path)) {
		LOG_DEBUG("Reading user mods...");
		std::size_t max;
		if (parentGame.GetLoadOrderMethod() == LOMETHOD_TEXTFILE) {
			/* Game uses the new load order system.
			 *
			 * Check if loadorder.txt exists, and read that if it does.
			 * If it doesn't exist, then read plugins.txt and scan the given directory for mods,
			 * adding those that weren't in the plugins.txt to the end of the load order, in the order they are read.
			 *
			 * There is no sure-fire way of managing such a situation. If no loadorder.txt, then
			 * no utilties compatible with that load order method have been installed, so it won't
			 * break anything apart from the load order not matching the load order in the Bashed
			 * Patch's Masters list if it exists. That isn't something that can be easily accounted
			 * for though.
			 */
			LOG_INFO("Using textfile-based load order mechanism.");
			if (fs::exists(parentGame.LoadOrderFile())) {  // If the loadorder.txt exists, get the load order from that.
				Load(parentGame, parentGame.LoadOrderFile());
			} else {
				if (fs::exists(parentGame.ActivePluginsFile()))  // If the plugins.txt exists, get the active load order from that.
					Load(parentGame, parentGame.ActivePluginsFile());
				if (parentGame.Id() == SKYRIM) {
					// Make sure that Skyrim.esm is first.
					Move(0, Item("Skyrim.esm"));
					// Add Update.esm if not already present.
					if (Item("Update.esm").Exists(parentGame) &&
					    FindItem("Update.esm", MOD) == items.size())
						Move(GetLastMasterPos(parentGame) + 1,
						     Item("Update.esm"));
				}
			}
			// Then scan through loadorder, removing any plugins that aren't in the data folder.
			/* MCP Note: Couldn't we change this to a for-loop instead? Maybe a for-each loop? Would probably look better than a while-loop.
			 * Requires C++11 but that's already a requirement so should be fine.
			 */
			std::vector<Item>::iterator itemIter = items.begin();
			while (itemIter != items.end()) {
				if (!itemIter->Exists(parentGame))
					itemIter = items.erase(itemIter);
				else
					++itemIter;
			}
		}
		max = items.size();
		// Now scan through Data folder. Add any plugins that aren't already in loadorder to loadorder, at the end.
		for (fs::directory_iterator itr(path); itr != fs::directory_iterator();
		     ++itr) {
			if (fs::is_regular_file(itr->status())) {
				fs::path filename = itr->path().filename();
				std::string ext = filename.extension().string();
				if (boost::iequals(ext, ".ghost")) {
					filename = filename.stem();
					ext = filename.extension().string();
				}
				if (boost::iequals(ext, ".esp") ||
				    boost::iequals(ext, ".esm")) {
					LOG_TRACE("-- Found mod: '%s'", filename.string().c_str());
					// Add file to modlist. If the filename has a '.ghost' extension, remove it.
					const Item tempItem = Item(filename.string());
					if (parentGame.GetLoadOrderMethod() == LOMETHOD_TIMESTAMP ||
					   (parentGame.GetLoadOrderMethod() == LOMETHOD_TEXTFILE &&
					    FindItem(tempItem.Name(), MOD) == max)) {  // If the plugin is not in loadorder, add it.
						items.push_back(tempItem);
						max++;
					}
				}
			}
		}
		LOG_DEBUG("Reading user mods done: %" PRIuS " total mods found.",
		          items.size());
		itemComparator ic(parentGame);
		std::sort(items.begin(), items.end(), ic);
	} else if (path == parentGame.LoadOrderFile() ||
	           path == parentGame.ActivePluginsFile()) {
		// loadorder.txt is simple enough that we can avoid needing the full modlist parser which has the crashing issue.
		// It's just a text file with a plugin filename on each line. Skip lines which are blank or start with '#'.

		// MCP Note: changed from path.c_str() to path.string(); needs testing as error was about not being able to convert wchar_t to char
		std::ifstream in(path.string());
		if (in.fail())
			throw boss_error(BOSS_ERROR_FILE_PARSE_FAIL, path.string());

		std::string line;
		while (in.good()) {
			std::getline(in, line);

			if (line.empty() || line[0] == '#')  // Character comparison is OK because it's ASCII.
				continue;

			if (path == parentGame.ActivePluginsFile())
				line = From1252ToUTF8(line);
			items.push_back(Item(line));
		}
		in.close();


		// Then scan through items, removing any plugins that aren't in the data folder.
		std::vector<Item>::iterator itemIter = items.begin();
		while (itemIter != items.end()) {
			if (!itemIter->Exists(parentGame))
				itemIter = items.erase(itemIter);
			else
				++itemIter;
		}

		itemComparator ic(parentGame);
		std::sort(items.begin(), items.end(), ic);  // Does this work?
	} else {
		// MCP Note: Can this be used with conditionalData::EvalConditions to reduce duplication?
		Skipper skipper;
		modlist_grammar grammar;
		std::string::const_iterator begin, end;
		std::string contents;

		grammar.SetErrorBuffer(&errorBuffer);
		grammar.SetGlobalMessageBuffer(&globalMessageBuffer);
		grammar.SetVarStore(&masterlistVariables);
		grammar.SetCRCStore(&fileCRCs);
		grammar.SetParentGame(&parentGame);

		if (!fs::exists(path))
			throw boss_error(BOSS_ERROR_FILE_NOT_FOUND, path.string());

		fileToBuffer(path, contents);

		begin = contents.begin();
		end = contents.end();

		//iterator_type u32b(begin);
		//iterator_type u32e(end);

		//bool r = phrase_parse(u32b, u32e, grammar, skipper, items);
		bool r = phrase_parse(begin, end, grammar, skipper, items);

		if (!r || begin != end || !errorBuffer.Empty())
			throw boss_error(BOSS_ERROR_FILE_PARSE_FAIL, path.string());
	}
}

void ItemList::Save(const fs::path file, const fs::path oldFile) {
	std::ofstream ofile;  // Not sure if it's using the one from std:: or boost::filesystem::
	// Back up file if it already exists.
	try {
		LOG_DEBUG("Saving backup of current modlist...");
		if (fs::exists(file))
			fs::rename(file, oldFile);
	} catch(fs::filesystem_error e) {
		// Couldn't rename the file, print an error message.
		LOG_ERROR("Backup of modlist failed with error: %s", e.what());
		throw boss_error(BOSS_ERROR_FS_FILE_RENAME_FAIL, file.string(),
		                 e.what());
	}
	// Open output file.

	// MCP Note: changed from file.c_str() to file.string(); needs testing as error was about not being able to convert wchar_t to char
	ofile.open(file.string());
	if (ofile.fail()) {  // Provide error message if it can't be written.
		LOG_ERROR("Backup cannot be saved.");
		throw boss_error(BOSS_ERROR_FILE_WRITE_FAIL, file.string());
	}

	// Iterate through items, printing out all group markers, mods and messsages.
	std::vector<Item>::iterator itemIter = items.begin();
	std::vector<Message>::iterator messageIter;
	for (itemIter; itemIter != items.end(); ++itemIter) {
		if (itemIter->Type() == BEGINGROUP)
			ofile << "BEGINGROUP: " << itemIter->Name() << std::endl;  // Print the group begin marker
		else if (itemIter->Type() == ENDGROUP)
			ofile << "ENDGROUP: " << itemIter->Name() << std::endl;  // Print the group end marker
		else {
			if (!itemIter->Conditions().empty()) {
				ofile << itemIter->Conditions() << ' ';
				if (itemIter->Type() == MOD)
					ofile << "MOD: ";
			}
			if (itemIter->Type() == REGEX)
				ofile << "REGEX: ";
			ofile << itemIter->Name() << std::endl;  // Print the mod name.
			// Print the messages with the appropriate syntax.
			std::vector<Message> messages = itemIter->Messages();
			for (messageIter = messages.begin(); messageIter != messages.end();
			     ++messageIter) {
				if (!messageIter->Conditions().empty())
					ofile << ' ' << messageIter->Conditions();
				ofile << ' ' << messageIter->KeyToString() << ": " << messageIter->Data() << std::endl;
			}
		}
	}

	ofile.close();
	LOG_INFO("Backup saved successfully.");
}

void ItemList::SavePluginNames(const Game& parentGame,
                               const fs::path file,
                               const bool activeOnly,
                               const bool doEncodingConversion) {
	std::string badFilename = "", contents;
	ItemList activePlugins;
	std::size_t numActivePlugins;
	if (activeOnly) {
		// To save needing a new parser, load plugins.txt into an ItemList then fill a hashset from that.
		// Also check if gl_current_game.ActivePluginsFile() then detect encoding if it is and translate outputted text from UTF-8 to the detected encoding.
		LOG_INFO("Loading plugins.txt into ItemList.");
		if (fs::exists(parentGame.ActivePluginsFile())) {
			activePlugins.Load(parentGame, parentGame.ActivePluginsFile());
			numActivePlugins = activePlugins.Items().size();
		}
	}

	LOG_INFO("Writing new \"%s\"", file.string().c_str());
	std::ofstream outfile;

	// MCP Note: changed from file.c_str() to file.string(); needs testing as error was about not being able to convert wchar_t to char
	outfile.open(file.string(), std::ios_base::trunc);
	if (outfile.fail())
		throw boss_error(BOSS_ERROR_FILE_WRITE_FAIL, file.string());

	std::size_t max = items.size();
	for (std::size_t i = 0; i < max; i++) {
		if (items[i].Type() == MOD) {
			if (activeOnly && (activePlugins.FindItem(items[i].Name(), MOD) == numActivePlugins || (parentGame.Id() == SKYRIM && items[i].Name() == "Skyrim.esm")))
				continue;
			LOG_DEBUG("Writing \"%s\" to \"%s\"", items[i].Name().c_str(),
			          file.string().c_str());
			if (doEncodingConversion) {  // Not UTF-8.
				try {
					outfile << FromUTF8To1252(items[i].Name()) << std::endl;
				} catch (boss_error /*&e*/) {
					badFilename = items[i].Name();
				}
			} else
				outfile << items[i].Name() << std::endl;
		}
	}
	outfile.close();

	if (!badFilename.empty())
		throw boss_error(BOSS_ERROR_ENCODING_CONVERSION_FAIL, badFilename,
		                 "1252");
}

void ItemList::EvalConditions(const Game& parentGame) {
	boost::unordered_set<std::string> setVars;
	boost::unordered_set<std::string> activePlugins;
	bool res;

	if (fs::exists(parentGame.ActivePluginsFile())) {
		ItemList active;
		active.Load(parentGame, parentGame.ActivePluginsFile());
		std::vector<Item> items = active.Items();
		for (std::size_t i = 0, max = items.size(); i < max; i++) {
			activePlugins.insert(to_lower_copy(items[i].Name()));
		}
	}

	// First eval variables.
	// Need to convert these from a vector to an unordered set.
	if (!masterlistVariables.empty()) {
		LOG_INFO("Starting to evaluate variable conditionals.");
		std::vector<MasterlistVar>::iterator varIter = masterlistVariables.begin();
		res = varIter->EvalConditions(setVars, fileCRCs, activePlugins, NULL,
		                              errorBuffer, parentGame);
		if (res) {
			setVars.insert(varIter->Data());
			++varIter;
		} else
			varIter = masterlistVariables.erase(varIter);
		// Eval the rest of the vars now that res has been initialised.
		while (varIter != masterlistVariables.end()) {
			res = varIter->EvalConditions(setVars, fileCRCs, activePlugins,
			                              &res, errorBuffer, parentGame);
			if (res) {
				setVars.insert(varIter->Data());
				++varIter;
			} else
				varIter = masterlistVariables.erase(varIter);
		}
	}

	// Now eval global messages.
	if (!globalMessageBuffer.empty()) {
		LOG_INFO("Starting to evaluate global message conditionals.");
		std::vector<Message>::iterator messageIter = globalMessageBuffer.begin();
		res = messageIter->EvalConditions(setVars, fileCRCs, activePlugins,
		                                  NULL, errorBuffer, parentGame);
		if (res)
			++messageIter;
		else
			messageIter = globalMessageBuffer.erase(messageIter);
		// Eval the rest of the global messages now that res has been initialised.
		while (messageIter != globalMessageBuffer.end()) {
			res = messageIter->EvalConditions(setVars, fileCRCs, activePlugins,
			                                  &res, errorBuffer, parentGame);
			if (res)
				++messageIter;
			else
				messageIter = globalMessageBuffer.erase(messageIter);
		}
	}

	// Now eval items. Need to keep track of the previous item.
	LOG_INFO("Starting to evaluate item conditionals.");
	bool wasPlugin = false;
	std::vector<Item>::iterator itemIter = items.begin();
	while (itemIter != items.end()) {
		if (itemIter->Type() == MOD || itemIter->Type() == REGEX) {
			if (!wasPlugin)
				res = itemIter->EvalConditions(setVars, fileCRCs,
				                               activePlugins, NULL,
				                               errorBuffer, parentGame);
			else
				res = itemIter->EvalConditions(setVars, fileCRCs,
				                               activePlugins, &res,
				                               errorBuffer, parentGame);  // Look at previous plugin's conditional eval result.
			if (res)
				++itemIter;
			else
				itemIter = items.erase(itemIter);
			wasPlugin = true;
		} else if (itemIter->Type() == BEGINGROUP) {
			if (itemIter->EvalConditions(setVars, fileCRCs, activePlugins,
			                             NULL, errorBuffer, parentGame)) {  // Don't need to record result as nothing will look at a previous group's conditional.
				++itemIter;
			} else {
				// Need to remove all the plugins in the group.
				std::size_t endPos = FindLastItem(itemIter->Name(), ENDGROUP);
				itemIter = items.erase(itemIter, items.begin() + endPos + 1);
			}
			wasPlugin = false;
		} else {
			++itemIter;  // ENDGROUP items should not be conditional, so treat them like they're not.
			wasPlugin = false;
		}
	}
}

void ItemList::EvalRegex(const Game& parentGame) {
	// Store installed mods in a hashset. Case insensitivity not required as regex itself is case-insensitive.
	boost::unordered_set<std::string> hashset;
	boost::unordered_set<std::string>::iterator setPos;
	for (fs::directory_iterator itr(parentGame.DataFolder());
	     itr != fs::directory_iterator(); ++itr) {
		if (fs::is_regular_file(itr->status())) {
			fs::path filename = itr->path().filename();
			std::string ext = filename.extension().string();
			if (boost::iequals(ext, ".ghost")) {
				filename = filename.stem();
				ext = filename.extension().string();
			}
			if (boost::iequals(ext, ".esp") || boost::iequals(ext, ".esm")) {
				hashset.insert(filename.string());
			}
		}
	}

	/* Now iterate through items vector, working on regex entries.
	 * First remove a regex entry, then look for matches in the hashset.
	 * Add all matches with the messages attached to the regex entry to the items vector in the position the regex entry occupied.
	 */
	std::vector<Item>::iterator itemIter = items.begin();
	while (itemIter != items.end()) {
		if (itemIter->Type() == REGEX) {
			// TODO(MCP): Replace this with the standard library version
			boost::regex reg;  // Form a regex.
			try {
				reg = boost::regex(itemIter->Name()+"(\\.ghost)?",
				                   boost::regex::extended|boost::regex::icase);  // Ghost extension is added so ghosted mods will also be found.
			} catch (boost::regex_error /*&e*/) {
				boss_error be = boss_error(BOSS_ERROR_REGEX_EVAL_FAIL,
				                           itemIter->Name());
				LOG_ERROR("\"%s\" is not a valid regular expression. Item skipped.",
				          be.getString().c_str());
				errorBuffer = ParsingError(be.getString());
				++itemIter;
				continue;
			}
			std::vector<Message> messages = itemIter->Messages();
			itemIter = items.erase(itemIter);
			// Now start looking.
			setPos = FindRegexMatch(hashset, reg, hashset.begin());
			while (setPos != hashset.end()) {  // Now insert the current found mod in the position of the regex mod.
				itemIter = items.insert(itemIter, Item(*setPos, MOD, messages));
				setPos = FindRegexMatch(hashset, reg, ++setPos);
				++itemIter;
			}
		} else
			++itemIter;
	}
}

void ItemList::ApplyMasterPartition(const Game& parentGame) {
	// Need to iterate through items vector, sorting it according to the rule that master items come before other items.
	std::size_t lastMasterPos = GetLastMasterPos(parentGame);
	std::size_t pos = GetNextMasterPos(parentGame, lastMasterPos + 1);
	while (pos < items.size()) {
		Item master = items[pos];
		items.erase(items.begin() + pos);
		items.insert(items.begin() + lastMasterPos + 1, master);
		++lastMasterPos;
		LOG_INFO("Master file \"%s\" moved before non-master plugins.",
		         master.Name().c_str());
		pos = GetNextMasterPos(parentGame, pos + 1);
	}
}

std::size_t ItemList::FindItem(const std::string name, const std::uint32_t type) const {
	std::size_t max = items.size();
	for (std::size_t i = 0; i < max; i++) {
		if (items[i].Type() == type && boost::iequals(items[i].Name(), name))
			return i;
	}
	return max;
}

std::size_t ItemList::FindLastItem(const std::string name, const std::uint32_t type) const {
	std::size_t max = items.size();
	for (std::vector<Item>::const_iterator it = items.end(), begin = items.begin();
	     it != begin; --it) {
		if (it->Type() == type && boost::iequals(it->Name(), name))
			return size_t(it - begin);
	}
	return max;
}

std::size_t ItemList::GetLastMasterPos(const Game& parentGame) const {
	std::size_t i = 0;
	while (i < items.size() &&
	      (items[i].IsGroup() || items[i].IsMasterFile(parentGame))) {  // SLLOOOOOWWWWW probably.
		i++;
	}
	if (i > 0)
		return i - 1;  // i is position of first plugin.
	return 0;
}

std::size_t ItemList::GetNextMasterPos(const Game& parentGame,
                                       std::size_t currPos) const {
	if (currPos >= items.size())
		return items.size();
	while (currPos < items.size() &&
	      (items[currPos].IsGroup() || !items[currPos].IsMasterFile(parentGame))) {  // SLLOOOOOWWWWW probably.
		currPos++;
	}
	return currPos;  // Position of first master after currPos.
}

std::vector<Item> ItemList::Items() const {
	return items;
}

ParsingError ItemList::ErrorBuffer() const {
	return errorBuffer;
}

std::vector<Message> ItemList::GlobalMessageBuffer() const {
	return globalMessageBuffer;
}

std::size_t ItemList::LastRecognisedPos() const {
	return lastRecognisedPos;
}

std::vector<MasterlistVar> ItemList::Variables() const {
	return masterlistVariables;
}

boost::unordered_map<std::string, std::uint32_t> ItemList::FileCRCs() const {
	return fileCRCs;
}

Item ItemList::ItemAt(const std::size_t pos) const {
	if (pos < items.size())
		return items[pos];
	return Item();
}

void ItemList::Items(const std::vector<Item> inItems) {
	items = inItems;
}

void ItemList::ErrorBuffer(const ParsingError buffer) {
	errorBuffer = buffer;
}

void ItemList::GlobalMessageBuffer(const std::vector<Message> buffer) {
	globalMessageBuffer = buffer;
}

void ItemList::LastRecognisedPos(const std::size_t pos) {
	lastRecognisedPos = pos;
}

void ItemList::Variables(const std::vector<MasterlistVar> variables) {
	masterlistVariables = variables;
}

void ItemList::FileCRCs(const boost::unordered_map<std::string, std::uint32_t> crcs) {
	fileCRCs = crcs;
}

void ItemList::Clear() {
	items.clear();
	errorBuffer.Clear();
	globalMessageBuffer.clear();
	lastRecognisedPos = 0;
	masterlistVariables.clear();
	fileCRCs.clear();
}

void ItemList::Erase(const std::size_t pos) {
	items.erase(items.begin() + pos);
}

void ItemList::Erase(const std::size_t startPos, const std::size_t endPos) {
	items.erase(items.begin() + startPos, items.begin() + endPos);
}

void ItemList::Insert(const std::size_t pos, const std::vector<Item> source,
                      const std::size_t sourceStart, const std::size_t sourceEnd) {
	items.insert(items.begin() + pos, source.begin() + sourceStart,
	             source.begin() + sourceEnd);
}

void ItemList::Insert(const std::size_t pos, const Item item) {
	items.insert(items.begin() + pos, item);
}

void ItemList::Move(std::size_t newPos, const Item item) {
	std::size_t itemPos = FindItem(item.Name(), item.Type());
	if (itemPos == items.size()) {
		Insert(newPos, item);
	} else {
		if (itemPos < newPos)
			newPos--;
		Erase(itemPos);
		Insert(newPos, item);
	}
}

// Searches a hashset for the first matching string of a regex and returns its iterator position. Usage internal to BOSS-Common.
boost::unordered_set<std::string>::iterator ItemList::FindRegexMatch(
    const boost::unordered_set<std::string> set,
    const boost::regex reg,
    boost::unordered_set<std::string>::iterator startPos) {
	while(startPos != set.end()) {
		if (boost::regex_match(*startPos, reg))
			break;
		++startPos;
	}
	return startPos;
}

}  // namespace boss
