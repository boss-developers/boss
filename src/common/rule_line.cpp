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

#include "common/rule_line.h"

#include <cstddef>
#include <cstdint>

#include <fstream>
#include <ostream>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include "common/conditional_data.h"
#include "common/error.h"
#include "common/game.h"
#include "common/globals.h"
#include "common/keywords.h"
#include "output/output.h"
#include "parsing/grammar.h"
#include "support/helpers.h"
#include "support/logger.h"

namespace boss {

namespace fs = boost::filesystem;

//////////////////////////////
// RuleLine Class Functions
//////////////////////////////

// TODO(MCP): Maybe condense these two constructors into one using default values for the paramaters?
RuleLine::RuleLine() {
	key = NONE;
	object = "";
}

RuleLine::RuleLine(const std::uint32_t inKey, const std::string inObject) {
	key = inKey;
	object = inObject;
}

bool RuleLine::IsObjectMessage() const {
	// MCP Note: Possibly change these if-statments to a switch-statement
	// First character of message, must be a message symbol, or part of an MF2 keyword.
	if (object[0] == '?' || object[0] == '$' || object[0] == '^' ||
	    object[0] == '%' || object[0] == ':' || object[0] == '"' ||
	    object[0] == '*') {
		return true;
	}
	std::size_t pos = object.find(':');
	if (pos == std::string::npos)
		return false;
	std::string keyString = object.substr(0, pos);
	if (keyString == "SAY" || keyString == "TAG" || keyString == "REQ" ||
	    keyString == "INC" || keyString == "DIRTY" || keyString == "WARN" ||
	    keyString == "ERROR")
		return true;
	return false;
}

std::string RuleLine::KeyToString() const {
	/*switch (key) {
		case ADD:
			return "ADD";
		case OVERRIDE:
			return "OVERRIDE";
		case FOR:
			return "FOR";
		case BEFORE:
			return "BEFORE";
		case AFTER:
			return "AFTER";
		case TOP:
			return "TOP";
		case BOTTOM:
			return "BOTTOM";
		case APPEND:
			return "APPEND";
		case REPLACE:
			return "REPLACE";
		default:
			return "NONE";
	}*/
	if (key == ADD)
		return "ADD";
	else if (key == OVERRIDE)
		return "OVERRIDE";
	else if (key == FOR)
		return "FOR";
	else if (key == BEFORE)
		return "BEFORE";
	else if (key == AFTER)
		return "AFTER";
	else if (key == TOP)
		return "TOP";
	else if (key == BOTTOM)
		return "BOTTOM";
	else if (key == APPEND)
		return "APPEND";
	else if (key == REPLACE)
		return "REPLACE";
	return "NONE";
}

Message RuleLine::ObjectAsMessage() const {
	switch (object[0]) {
		case '?':
			return Message(SAY, object.substr(1));
		case '$':
			return Message(SAY, object.substr(1));
		case '^':
			return Message(SAY, object.substr(1));
		case '%':
			return Message(TAG, object.substr(1));
		case ':':
			return Message(REQ, object.substr(1));
		case '"':
			return Message(INC, object.substr(1));
		case '*':
			return Message(ERR, object.substr(1));
		default:
			std::size_t pos = object.find(':');
			if (pos == std::string::npos)
				return Message(NONE, "");

			std::string keyString = object.substr(0, pos);
			if (keyString == "SAY")
				return Message(SAY, object.substr(pos + 1));
			else if (keyString == "TAG")
				return Message(TAG, object.substr(pos + 1));
			else if (keyString == "REQ")
				return Message(REQ, object.substr(pos + 1));
			else if (keyString == "INC")
				return Message(INC, object.substr(pos + 1));
			else if (keyString == "DIRTY")
				return Message(DIRTY, object.substr(pos + 1));
			else if (keyString == "WARN")
				return Message(WARN, object.substr(pos + 1));
			else if (keyString == "ERROR")
				return Message(ERR, object.substr(pos + 1));
			return Message(NONE, object.substr(pos + 1));
	}
}

std::uint32_t RuleLine::Key() const {
	return key;
}

std::string RuleLine::Object() const {
	return object;
}

void RuleLine::Key(const std::uint32_t inKey) {
	key = inKey;
}

void RuleLine::Object(const std::string inObject) {
	object = inObject;
}

//////////////////////////////
// Rule Class Functions
//////////////////////////////

Rule::Rule() : RuleLine() {
	enabled = true;
	lines.clear();
}

bool Rule::Enabled() const {
	return enabled;
}

std::vector<RuleLine> Rule::Lines() const {
	return lines;
}

RuleLine Rule::LineAt(const std::size_t pos) const {
	if (pos == 0)
		return RuleLine(Key(), Object());  // Return sort line.
	else if (pos - 1 < lines.size())
		return lines[pos - 1];
	return RuleLine();
}

void Rule::Enabled(const bool e) {
	enabled = e;
}

void Rule::Lines(const std::vector<RuleLine> inLines) {
	lines = inLines;
}


//////////////////////////////
// RuleList Class Functions
//////////////////////////////

RuleList::RuleList() {
	rules.clear();
	errorBuffer.clear();
}

void RuleList::Load(const Game &parentGame, const fs::path file) {
	Skipper skipper;
	userlist_grammar grammar;
	std::string::const_iterator begin, end;
	std::string contents;

	Clear();

	skipper.SkipIniComments(false);
	grammar.SetErrorBuffer(&errorBuffer);

	if (!fs::exists(file)) {
		// MCP Note: changed from file.c_str() to file.string(); needs testing as error was about not being able to convert wchar_t to char
		//ofstream userlist_file(file.c_str(), ios_base::binary);
		std::ofstream userlist_file(file.string(), std::ios_base::binary);  // Is this one std:: or boost::filesystem::? Not sure
		if (!userlist_file.fail())
			userlist_file << '\xEF' << '\xBB' << '\xBF';  // Write UTF-8 BOM to ensure the file is recognised as having the UTF-8 encoding.
		else
			throw boss_error(BOSS_ERROR_FILE_WRITE_FAIL, file.string());
		userlist_file.close();
		return;
	}

	fileToBuffer(file, contents);

	begin = contents.begin();
	end = contents.end();

	bool r = phrase_parse(begin, end, grammar, skipper, rules);

	if (!r || begin != end)  // This might not work correctly.
		throw boss_error(BOSS_ERROR_FILE_PARSE_FAIL, file.string());

	CheckSyntax(parentGame);
}

void RuleList::Save(const fs::path file) {
	// MCP Note: changed from file.c_str() to file.string(); needs testing as error was about not being able to convert wchar_t to char
	//ofstream outFile(file.c_str(),ios_base::trunc);
	std::ofstream outFile(file.string(), std::ios_base::trunc);

	if (outFile.fail()) {  // Provide error message if it can't be written.
		LOG_ERROR("Backup cannot be saved.");
		throw boss_error(BOSS_ERROR_FILE_WRITE_FAIL, file.string());
	}

	for (std::vector<Rule>::iterator ruleIter = rules.begin();
	     ruleIter != rules.end(); ++ruleIter) {
		if (!ruleIter->Enabled())
			outFile << "DISABLE ";
		outFile << boost::to_upper_copy(ruleIter->KeyToString()) << ": " << ruleIter->Object() << std::endl;

		std::vector<RuleLine> lines = ruleIter->Lines();
		for (std::vector<RuleLine>::iterator lineIter = lines.begin();
		     lineIter != lines.end(); ++lineIter)
			outFile << boost::to_upper_copy(lineIter->KeyToString()) << ": " << lineIter->Object() << std::endl;
		outFile << std::endl;
	}
	outFile.close();
}

std::size_t RuleList::FindRule(const std::string ruleObject,
                               const bool onlyEnabled) const {
	std::size_t max = rules.size();
	for (std::size_t i = 0; i < max; i++) {
		// TODO(MCP): Move !onlyEnabled to front of if-statement to take advantage of short-cirtuit evaluation.
		if ((onlyEnabled && rules[i].Enabled()) || !onlyEnabled) {
			if (boost::iequals(rules[i].Object(), ruleObject))
				return i;
		}
	}
	return max;
}

std::vector<Rule> RuleList::Rules() const {
	return rules;
}

std::vector<ParsingError> RuleList::ErrorBuffer() const {
	return errorBuffer;
}

Rule RuleList::RuleAt(const std::size_t pos) const {
	if (pos < rules.size())
		return rules[pos];
	return Rule();
}

void RuleList::Rules(const std::vector<Rule> inRules) {
	rules = inRules;
}

void RuleList::ErrorBuffer(const std::vector<ParsingError> buffer) {
	errorBuffer = buffer;
}

void RuleList::Erase(const std::size_t pos) {
	rules.erase(rules.begin() + pos);
}

void RuleList::Insert(const std::size_t pos, const Rule rule) {
	rules.insert(rules.begin() + pos, rule);
}

void RuleList::Replace(const std::size_t pos, const Rule rule) {
	if (pos < rules.size())
		rules[pos] = rule;
}

void RuleList::Clear() {
	rules.clear();
	errorBuffer.clear();
}

void RuleList::CheckSyntax(const Game &parentGame) {
	// Loop through rules, check syntax of each. If a rule has invalid syntax, remove it.
	std::vector<Rule>::iterator it = rules.begin();
	while(it != rules.end()) {
		std::string ruleKeyString = it->KeyToString();
		Item ruleObject = Item(it->Object());
		try {
			if (ruleObject.IsPlugin()) {
				if (ruleKeyString != "FOR" &&
				    ruleObject.IsGameMasterFile(parentGame))
					throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % ESortingMasterEsm).str());
			} else {
				if (boost::iequals(ruleObject.Name(), "esms"))
					throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % ESortingGroupEsms).str());
				if (ruleKeyString == "ADD")
					throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % EAddingModGroup).str());
				else if (ruleKeyString == "FOR")
					throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % EAttachingMessageToGroup).str());
			}
			std::vector<RuleLine> lines = it->Lines();
			bool hasSortLine = false, hasReplaceLine = false;
			for (std::size_t i = 0, max = lines.size(); i < max; i++) {
				Item subject = Item(lines[i].Object());
				if (lines[i].Key() == BEFORE || lines[i].Key() == AFTER) {
					if (hasSortLine)
						throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % EMultipleSortLines).str());
					if (i != 0)
						throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % ESortNotSecond).str());
					if (ruleKeyString == "FOR")
						throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % ESortLineInForRule).str());
					if (boost::iequals(ruleObject.Name(), subject.Name()))
						throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % ESortingToItself).str());
					if ((ruleObject.IsPlugin() && !subject.IsPlugin()) ||
					    (!ruleObject.IsPlugin() && subject.IsPlugin()))
						throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % EReferencingModAndGroup).str());
					if (lines[i].Key() == BEFORE) {
						if (boost::iequals(subject.Name(), "esms"))
							throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % ESortingGroupBeforeEsms).str());
						else if (subject.IsGameMasterFile(parentGame))
							throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % ESortingModBeforeGameMaster).str());
						else if (!ruleObject.IsMasterFile(parentGame) &&
						         subject.IsMasterFile(parentGame))
							throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % ESortingPluginBeforeMaster).str());
					} else if (ruleObject.IsMasterFile(parentGame) &&
					           !subject.IsMasterFile(parentGame)) {
						throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % ESortingMasterAfterPlugin).str());
					}
					hasSortLine = true;
				} else if (lines[i].Key() == TOP || lines[i].Key() == BOTTOM) {
					if (hasSortLine)
						throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % EMultipleSortLines).str());
					if (i != 0)
						throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % ESortNotSecond).str());
					if (ruleKeyString == "FOR")
						throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % ESortLineInForRule).str());
					if (lines[i].Key() == TOP &&
					    boost::iequals(subject.Name(), "esms"))
						throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % EInsertingToTopOfEsms).str());
					if (!ruleObject.IsPlugin() || subject.IsPlugin())
						throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % EInsertingGroupOrIntoMod).str());
					hasSortLine = true;
				} else if (lines[i].Key() == APPEND ||
				           lines[i].Key() == REPLACE) {
					if (!ruleObject.IsPlugin())
						throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % EAttachingMessageToGroup).str());
					if (lines[i].Key() == REPLACE) {
						if (hasReplaceLine)
							throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % EMultipleReplaceLines).str());
						if ((ruleKeyString == "FOR" && i != 0) ||
						    (ruleKeyString != "FOR" && i != 1))
							throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % EReplaceNotFirst).str());
						hasReplaceLine = true;
					}
					if (!lines[i].IsObjectMessage())
						throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % EAttachingNonMessage).str());
				}
			}
			++it;
		} catch (ParsingError &e) {
			it = rules.erase(it);
			errorBuffer.push_back(e);
			LOG_ERROR(Outputter(PLAINTEXT, e).AsString().c_str());
		}
	}
}

}  // namespace boss
