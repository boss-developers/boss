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

#ifndef COMMON_CONDITIONAL_DATA_H_
#define COMMON_CONDITIONAL_DATA_H_

#include <cstddef>
#include <cstdint>
#include <ctime>

#include <string>
#include <vector>

#include <boost/fusion/adapted/struct/detail/extension.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include "common/dll_def.h"
#include "support/helpers.h"

//#include "common/error.h"

namespace boss {

// Forward Declarations

class BOSS_COMMON Game;
class BOSS_COMMON ParsingError;
//class Version;

//////////////////////////////
// Masterlist Classes
//////////////////////////////

// Base class for all conditional data types.
class BOSS_COMMON conditionalData {
 public:
	conditionalData();
	conditionalData(const std::string inData, const std::string inConditions);

	std::string Data() const;
	std::string Conditions() const;

	void Data(const std::string inData);
	void Conditions(const std::string inConditions);

	bool EvalConditions(boost::unordered_set<std::string>& setVars,
	                    boost::unordered_map<std::string, std::uint32_t>& fileCRCs,
	                    boost::unordered_set<std::string>& activePlugins,
	                    bool *condResult,
	                    ParsingError& errorBuffer,
	                    const Game& parentGame);
 private:
	friend struct boost::fusion::extension::access;
	std::string data;
	std::string conditions;
};

class BOSS_COMMON MasterlistVar : public conditionalData {
 public:
	MasterlistVar();
	MasterlistVar(std::string inData, std::string inConditions);
 private:
	friend struct boost::fusion::extension::access;
};

class BOSS_COMMON Message : public conditionalData {
 public:
	Message();
	Message(const std::uint32_t inKey, const std::string inData);

	std::uint32_t Key() const;
	void Key(const std::uint32_t inKey);

	std::string KeyToString() const;  // Has HTML-safe output.
 private:
	friend struct boost::fusion::extension::access;
	std::uint32_t key;
};

class BOSS_COMMON Item : public conditionalData {
 public:
	Item();
	Item(const std::string inName);
	Item(const std::string inName, const std::uint32_t inType);
	Item(const std::string inName, const std::uint32_t inType,
	     const std::vector<Message> inMessages);

	std::vector<Message> Messages() const;
	std::uint32_t Type() const;
	std::string Name() const;
	void Messages(const std::vector<Message> inMessages);
	void Type(const std::uint32_t inType);
	void Name(const std::string inName);

	bool IsPlugin() const;
	bool IsGroup() const;
	bool IsGameMasterFile(const Game& parentGame) const;
	bool IsMasterFile(const Game& parentGame) const;
	bool IsFalseFlagged(const Game& parentGame) const;          // True if IsMasterFile does not match file extension.
	bool IsGhosted(const Game& parentGame) const;               // Checks if the file exists in ghosted form.
	bool Exists(const Game& parentGame) const;                  // Checks if the file exists in the data folder, ghosted or not.
	Version GetVersion(const Game& parentGame) const;           // Outputs the file's header.
	std::time_t GetModTime(const Game& parentGame) const;       // Can throw exception.

	void UnGhost(const Game& parentGame) const;                 // Can throw exception.
	void SetModTime(const Game& parentGame,
	                const std::time_t modificationTime) const;  // Can throw exception.

	void InsertMessage(std::size_t pos, Message item);
	void ClearMessages();

	bool EvalConditions(boost::unordered_set<std::string>& setVars,
	                    boost::unordered_map<std::string, std::uint32_t>& fileCRCs,
	                    boost::unordered_set<std::string>& activePlugins,
	                    bool * condResult,
	                    ParsingError& errorBuffer,
	                    const Game& parentGame);

 private:
	friend struct boost::fusion::extension::access;
	std::vector<Message> messages;
	// string data is now filename (or group name).
	// Trimmed and case-preserved. ".ghost" extensions are removed.
	std::uint32_t type;
};

}  // namespace boss
#endif  // COMMON_CONDITIONAL_DATA_H_
