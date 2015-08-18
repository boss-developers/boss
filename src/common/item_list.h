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

#ifndef COMMON_ITEM_LIST_H_
#define COMMON_ITEM_LIST_H_

#include <cstddef>
#include <cstdint>

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
/*
 * #include <boost/unordered_map.hpp>
 * #include <boost/unordered_set.hpp>
 */

#include "common/dll_def.h"
#include "common/error.h"

namespace boss {

namespace fs = boost::filesystem;

// Forward Declarations

class BOSS_COMMON Game;
class BOSS_COMMON Item;
class BOSS_COMMON MasterlistVar;
class BOSS_COMMON Message;
//class BOSS_COMMON ParsingError;

class BOSS_COMMON ItemList {
 public:
	ItemList();
	void Load(const Game &parentGame, const fs::path path);        // Load by scanning path. If path is a directory, it scans it for plugins.
	                                                               // If path is a file, it parses it using the modlist grammar.
	                                                               // May throw exception on fail.
	void Save(const fs::path file, const fs::path oldFile);        // Output to file in MF2. Backs up any existing file to oldFile.
	                                                               // Throws exception on fail.
	void SavePluginNames(const Game &parentGame,
	                     const fs::path file,
	                     const bool activeOnly,
	                     const bool doEncodingConversion);         // Save only a list of plugin filenames to the given file. For use with Skyrim. Throws exception on fail.
	void EvalConditions(const Game &parentGame);                   // Evaluates the conditionals for each item, discarding those items whose conditionals evaluate to false. Also evaluates global message conditionals.
	void EvalRegex(const Game &parentGame);
	void ApplyMasterPartition(const Game &parentGame);             // Puts all master files before other plugins. Can throw exception.

	std::size_t FindItem(const std::string name,
	                     const std::uint32_t type) const;          // Find the position of the item with name 'name'. Case-insensitive.
	std::size_t FindLastItem(const std::string name,
	                         const std::uint32_t type) const;      // Find the last item with the name 'name'. Case-insensitive.
	std::size_t GetLastMasterPos(const Game &parentGame) const;    // Can throw exception.
	std::size_t GetNextMasterPos(const Game &parentGame,
	                             std::size_t currPos) const;       // Can throw exception.

	Item ItemAt(std::size_t pos) const;

	std::vector<Item> Items() const;
	ParsingError ErrorBuffer() const;
	std::vector<Message> GlobalMessageBuffer() const;
	std::size_t LastRecognisedPos() const;
	std::vector<MasterlistVar> Variables() const;
	std::unordered_map<std::string, std::uint32_t> FileCRCs() const;

	void Items(const std::vector<Item> items);
	void ErrorBuffer(const ParsingError buffer);
	void GlobalMessageBuffer(const std::vector<Message> buffer);
	void LastRecognisedPos(const std::size_t pos);
	void Variables(const std::vector<MasterlistVar> variables);
	void FileCRCs(const std::unordered_map<std::string, std::uint32_t> crcs);

	void Clear();
	void Erase(const std::size_t pos);
	void Erase(const std::size_t startPos, const std::size_t endPos);
	void Insert(const std::size_t pos, const std::vector<Item> source,
	            std::size_t sourceStart, std::size_t sourceEnd);
	void Insert(const std::size_t pos, const Item item);
	void Move(std::size_t newPos, const Item item);  // Adds the item if it isn't already present.

 private:
	// Searches a hashset for the first matching string of a
	// regex and returns its iterator position.
	std::unordered_set<std::string>::iterator FindRegexMatch(
	    const std::unordered_set<std::string> set,
	    const boost::regex reg,
	    std::unordered_set<std::string>::iterator startPos);

	std::vector<Item> items;
	ParsingError errorBuffer;
	std::vector<Message> globalMessageBuffer;
	std::size_t lastRecognisedPos;
	std::vector<MasterlistVar> masterlistVariables;
	std::unordered_map<std::string, std::uint32_t> fileCRCs;
};

}  // namespace boss
#endif  // COMMON_ITEM_LIST_H_
