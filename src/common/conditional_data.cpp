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

#include "common/conditional_data.h"

#include <cstddef>
#include <cstdint>
#include <ctime>

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include "common/error.h"
#include "common/game.h"
#include "common/keywords.h"
#include "parsing/grammar.h"
#include "support/helpers.h"
#include "support/logger.h"
#include "support/mod_format.h"

namespace boss {

namespace fs = boost::filesystem;

/////////////////////////////////////
// ConditionalData Class Methods
/////////////////////////////////////

// TODO(MCP): Maybe change this constructor to use default values for the paramaters?
conditionalData::conditionalData() {
	data = "";
	conditions = "";
}

conditionalData::conditionalData(const std::string inData,
                                 const std::string inConditions) {
	data = inData;
	conditions = inConditions;
}

std::string conditionalData::Data() const {
	return data;
}

std::string conditionalData::Conditions() const {
	return conditions;
}

void conditionalData::Data(const std::string inData) {
	data = inData;
}

void conditionalData::Conditions(const std::string inConditions) {
	conditions = inConditions;
}

bool conditionalData::EvalConditions(std::unordered_set<std::string> &setVars,
                                     std::unordered_map<std::string, std::uint32_t> &fileCRCs,
                                     std::unordered_set<std::string> &activePlugins,
                                     bool *condResult,
                                     ParsingError &errorBuffer,
                                     const Game &parentGame) {
	if (!conditions.empty()) {
		Skipper skipper;
		conditional_grammar grammar;
		std::string::const_iterator begin, end;
		bool eval;

		skipper.SkipIniComments(false);
		grammar.SetVarStore(&setVars);
		grammar.SetCRCStore(&fileCRCs);
		grammar.SetErrorBuffer(&errorBuffer);
		grammar.SetParentGame(&parentGame);
		grammar.SetActivePlugins(&activePlugins);
		grammar.SetLastConditionalResult(condResult);

		begin = conditions.begin();
		end = conditions.end();

		//iterator_type u32b(begin);
		//iterator_type u32e(end);

		//bool r = phrase_parse(u32b, u32e, grammar, skipper, eval);
		// MCP Note: Where does phrase_parse come from? boost::spirit::qi?
		bool r = phrase_parse(begin, end, grammar, skipper, eval);

		if (!r || begin != end)
			throw boss_error(BOSS_ERROR_CONDITION_EVAL_FAIL, conditions, data);

		return eval;
	}
	return true;
}

/////////////////////////////////////
// MasterlistVar Class Methods
/////////////////////////////////////

MasterlistVar::MasterlistVar() : conditionalData() {}

MasterlistVar::MasterlistVar(std::string inData, std::string inConditions)
    : conditionalData(inData, inConditions) {}

//////////////////////////////
// Message Class Functions
//////////////////////////////

Message::Message() : conditionalData(), key(SAY) {}

Message::Message(const std::uint32_t inKey, const std::string inData)
    : conditionalData(inData, ""), key(inKey) {}

std::uint32_t Message::Key() const {
	return key;
}

void Message::Key(const std::uint32_t inKey) {
	key = inKey;
}

std::string Message::KeyToString() const {
	/*switch (key) {
		case SAY:
			return "SAY";
		case TAG:
			return "TAG";
		case REQ:
			return "REQ";
		case WARN:
			return "WARN";
		case ERR:
			return "ERROR";
		case INC:
			return "INC";
		case DIRTY:
			return "DIRTY";
		default:
			return "NONE";
	}*/
	if (key == SAY)
		return "SAY";
	else if (key == TAG)
		return "TAG";
	else if (key == REQ)
		return "REQ";
	else if (key == WARN)
		return "WARN";
	else if (key == ERR)
		return "ERROR";
	else if (key == INC)
		return "INC";
	else if (key == DIRTY)
		return "DIRTY";
	return "NONE";
}

//////////////////////////////
// Item Class Functions
//////////////////////////////

Item::Item() : conditionalData(), type(MOD) {}

Item::Item(const std::string inName)
    : conditionalData(inName, ""), type(MOD) {}

Item::Item(const std::string inName, const std::uint32_t inType)
    : conditionalData(inName, ""), type(inType) {}

Item::Item(const std::string inName, const std::uint32_t inType,
           const std::vector<Message> inMessages)
    : conditionalData(inName, ""), type(inType), messages(inMessages) {}

std::vector<Message> Item::Messages() const {
	return messages;
}

std::uint32_t Item::Type() const {
	return type;
}

std::string Item::Name() const {
	return Data();
}

void Item::Messages(const std::vector<Message> inMessages) {
	messages = inMessages;
}

void Item::Type(const std::uint32_t inType) {
	type = inType;
}

void Item::Name(const std::string inName) {
	Data(inName);
}

bool Item::IsPlugin() const {
	const std::string ext = boost::to_lower_copy(fs::path(Data()).extension().string());
	return (ext == ".esp" || ext == ".esm");
}

bool Item::IsGroup() const {
	return (!fs::path(Data()).has_extension() && !Data().empty());
}

bool Item::IsGameMasterFile(const Game &parentGame) const {
	return boost::iequals(Data(), parentGame.MasterFile().Name());
}

bool Item::IsMasterFile(const Game &parentGame) const {
	if (IsGhosted(parentGame))
		return IsPluginMaster(parentGame.DataFolder() / fs::path(Data() + ".ghost"));
	return IsPluginMaster(parentGame.DataFolder() / Data());
}

bool Item::IsFalseFlagged(const Game &parentGame) const {
	std::string ext;
	if (IsGhosted(parentGame))
		ext = fs::path(Data()).stem().extension().string();
	else
		ext = fs::path(Data()).extension().string();
	return ((IsMasterFile(parentGame) && !boost::iequals(ext, ".esm")) ||
	        (!IsMasterFile(parentGame) && boost::iequals(ext, ".esm")));
}

bool Item::IsGhosted(const Game &parentGame) const {
	return fs::exists(parentGame.DataFolder() / fs::path(Data() + ".ghost"));
}

bool Item::Exists(const Game &parentGame) const {
	return (fs::exists(parentGame.DataFolder() / Data()) ||
	        fs::exists(parentGame.DataFolder() / fs::path(Data() + ".ghost")));
}

Version Item::GetVersion(const Game &parentGame) const {
	if (!IsPlugin())
		return Version();

	ModHeader header;

	// Read mod's header now...
	if (IsGhosted(parentGame))
		header = ReadHeader(parentGame.DataFolder() / fs::path(Data() + ".ghost"));
	else
		header = ReadHeader(parentGame.DataFolder() / Data());

	// The current mod's version if found, or empty otherwise.
	return Version(header.Version);
}

std::time_t Item::GetModTime(const Game &parentGame) const {  // Can throw exception.
	try {
		if (IsGhosted(parentGame))
			return fs::last_write_time(parentGame.DataFolder() / fs::path(Data() + ".ghost"));
		// MCP Note: Need to read up on try-catch to see if we can remove the else
		else
			return fs::last_write_time(parentGame.DataFolder() / Data());
	} catch(fs::filesystem_error e) {
		LOG_WARN("%s; Report the mod in question with a download link to an official BOSS thread.",
		         e.what());
		throw boss_error(BOSS_ERROR_FS_FILE_MOD_TIME_READ_FAIL, Data(),
		                 e.what());
	}
}

void Item::UnGhost(const Game &parentGame) const {  // Can throw exception.
	if (IsGhosted(parentGame)) {
		try {
			fs::rename(parentGame.DataFolder() / fs::path(Data() + ".ghost"),
			           parentGame.DataFolder() / Data());
		} catch (fs::filesystem_error e) {
			throw boss_error(BOSS_ERROR_FS_FILE_RENAME_FAIL, Data() + ".ghost",
			                 e.what());
		}
	}
}

void Item::SetModTime(const Game &parentGame,
                      const std::time_t modificationTime) const {
	try {
		if (IsGhosted(parentGame))
			fs::last_write_time(parentGame.DataFolder() / fs::path(Data() + ".ghost"),
			                    modificationTime);
		else
			fs::last_write_time(parentGame.DataFolder() / Data(),
			                    modificationTime);
	} catch(fs::filesystem_error e) {
		throw boss_error(BOSS_ERROR_FS_FILE_MOD_TIME_WRITE_FAIL, Data(),
		                 e.what());
	}
}

void Item::InsertMessage(const std::size_t pos, const Message message) {
	messages.insert(messages.begin() + pos, message);
}

void Item::ClearMessages() {
	messages.clear();
}

bool Item::EvalConditions(std::unordered_set<std::string> &setVars,
                          std::unordered_map<std::string, std::uint32_t> &fileCRCs,
                          std::unordered_set<std::string> &activePlugins,
                          bool *condResult,
                          ParsingError &errorBuffer, const Game &parentGame) {
	if (Type() == ENDGROUP)
		return true;

	LOG_TRACE("Evaluating conditions for item \"%s\"", Data().c_str());

	if (!conditionalData::EvalConditions(setVars, fileCRCs, activePlugins,
	                                     condResult, errorBuffer, parentGame))  // Plugin needs to know what previous plugin's condition eval result was.
		return false;

	if (Type() == BEGINGROUP)
		return true;

	// Eval attached messages.
	if (!messages.empty()) {
		std::vector<Message>::iterator messageIter = messages.begin();
		bool res = messageIter->EvalConditions(setVars, fileCRCs, activePlugins,
		                                       NULL, errorBuffer, parentGame);  // No previous message for this plugin.
		if (res)
			++messageIter;
		else
			messageIter = messages.erase(messageIter);
		// Eval the rest of the messages now that res has been initialised.
		while (messageIter != messages.end()) {
			res = messageIter->EvalConditions(setVars, fileCRCs, activePlugins,
			                                  &res, errorBuffer, parentGame);
			if (res)
				++messageIter;
			else
				messageIter = messages.erase(messageIter);
		}
	}

	return true;
}

}  // namespace boss
