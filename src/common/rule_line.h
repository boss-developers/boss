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

#ifndef COMMON_RULE_LINE_H_
#define COMMON_RULE_LINE_H_

#include <cstddef>
#include <cstdint>

#include <string>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/fusion/adapted/struct/detail/extension.hpp>

#include "common/dll_def.h"

namespace boss {

namespace fs = boost::filesystem;

class BOSS_COMMON Game;
class BOSS_COMMON Message;
class BOSS_COMMON ParsingError;

//////////////////////////////
// Userlist Classes
//////////////////////////////

class BOSS_COMMON RuleLine {
 public:
	RuleLine();
	RuleLine(const std::uint32_t inKey, const std::string inObject);

	bool IsObjectMessage() const;
	std::string KeyToString() const;  // Has HTML-safe output.
	Message ObjectAsMessage() const;

	std::uint32_t Key() const;
	std::string Object() const;

	void Key(const std::uint32_t inKey);
	void Object(const std::string inObject);
 private:
	friend struct boost::fusion::extension::access;
	std::uint32_t key;
	std::string object;
};

class BOSS_COMMON Rule : public RuleLine {
 public:
	Rule();

	bool Enabled() const;
	std::vector<RuleLine> Lines() const;

	RuleLine LineAt(const std::size_t pos) const;

	void Enabled(const bool e);
	void Lines(const std::vector<RuleLine> inLines);
 private:
	friend struct boost::fusion::extension::access;
	bool enabled;
	std::vector<RuleLine> lines;
};

class BOSS_COMMON RuleList {
 public:
	RuleList();
	void Load(const Game& parentGame, const fs::path file);  // Throws exception on fail.
	void Save(const fs::path file);                          // Throws exception on fail.
	std::size_t FindRule(const std::string ruleObject,
	                     const bool onlyEnabled) const;

	std::vector<Rule> Rules() const;
	std::vector<ParsingError> ErrorBuffer() const;

	Rule RuleAt(const std::size_t pos) const;

	void Rules(const std::vector<Rule> inRules);
	void ErrorBuffer(const std::vector<ParsingError> buffer);

	void Erase(const std::size_t pos);
	void Insert(const std::size_t pos, const Rule rule);
	void Replace(const std::size_t pos, const Rule rule);

	void Clear();

 private:
	void CheckSyntax(const Game& parentGame);  // Rule checker function, checks for syntax (not parsing) errors.

	std::vector<Rule> rules;
	std::vector<ParsingError> errorBuffer;
};

}  // namespace boss
#endif  // COMMON_RULE_LINE_H_
