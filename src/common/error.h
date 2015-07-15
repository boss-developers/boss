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

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/
// Contains the BOSS exception class.

#ifndef COMMON_ERROR_H_
#define COMMON_ERROR_H_

#include <cstdint>

#include <string>

#include <boost/format.hpp>
#include <boost/locale.hpp>

#include "common/dll_def.h"

namespace boss {

namespace bloc = boost::locale;

// MCP Note: Possibly change these to enums?
// Return codes, mostly error codes.
BOSS_COMMON extern const std::uint32_t BOSS_OK;

BOSS_COMMON extern const std::uint32_t BOSS_ERROR_FILE_READ_FAIL;
BOSS_COMMON extern const std::uint32_t BOSS_ERROR_FILE_WRITE_FAIL;
BOSS_COMMON extern const std::uint32_t BOSS_ERROR_FILE_NOT_UTF8;
BOSS_COMMON extern const std::uint32_t BOSS_ERROR_FILE_NOT_FOUND;
BOSS_COMMON extern const std::uint32_t BOSS_ERROR_FILE_PARSE_FAIL;
BOSS_COMMON extern const std::uint32_t BOSS_ERROR_CONDITION_EVAL_FAIL;
BOSS_COMMON extern const std::uint32_t BOSS_ERROR_REGEX_EVAL_FAIL;
BOSS_COMMON extern const std::uint32_t BOSS_ERROR_NO_GAME_DETECTED;
BOSS_COMMON extern const std::uint32_t BOSS_ERROR_ENCODING_CONVERSION_FAIL;
BOSS_COMMON extern const std::uint32_t BOSS_ERROR_PLUGIN_BEFORE_MASTER;
BOSS_COMMON extern const std::uint32_t BOSS_ERROR_INVALID_SYNTAX;

BOSS_COMMON extern const std::uint32_t BOSS_ERROR_GIT_ERROR;

BOSS_COMMON extern const std::uint32_t BOSS_ERROR_FS_FILE_MOD_TIME_READ_FAIL;
BOSS_COMMON extern const std::uint32_t BOSS_ERROR_FS_FILE_MOD_TIME_WRITE_FAIL;
BOSS_COMMON extern const std::uint32_t BOSS_ERROR_FS_FILE_RENAME_FAIL;
BOSS_COMMON extern const std::uint32_t BOSS_ERROR_FS_FILE_DELETE_FAIL;
BOSS_COMMON extern const std::uint32_t BOSS_ERROR_FS_CREATE_DIRECTORY_FAIL;
BOSS_COMMON extern const std::uint32_t BOSS_ERROR_FS_ITER_DIRECTORY_FAIL;

BOSS_COMMON extern const std::uint32_t BOSS_ERROR_GUI_WINDOW_INIT_FAIL;

class BOSS_COMMON boss_error {
 public:
	// For general errors not referencing specific files.
	boss_error(const std::uint32_t internalErrCode);

	// For general errors referencing specific files.
	boss_error(const std::uint32_t internalErrCode, const std::string internalErrSubject);

	// For errors from BOOST Filesystem functions.
	boss_error(const std::uint32_t internalErrCode, const std::string internalErrSubject,
	           const std::string externalErrString);

	// For errors from other external functions.
	boss_error(const std::string externalErrString, const std::uint32_t internalErrCode);

	// Returns the error code for the object.
	std::uint32_t getCode() const;

	// Returns the error string for the object.
	std::string getString() const;

 private:
	std::uint32_t errCode;
	std::string errString;
	std::string errSubject;
};

// Parsing error formats.
static boost::format MasterlistParsingErrorHeader(bloc::translate("Masterlist Parsing Error: Expected a %1% at:"));
static boost::format IniParsingErrorHeader(bloc::translate("Ini Parsing Error: Expected a %1% at:"));
static boost::format RuleListParsingErrorHeader(bloc::translate("Userlist Parsing Error: Expected a %1% at:"));
static boost::format RuleListSyntaxErrorMessage(bloc::translate("Userlist Syntax Error: The rule beginning \"%1%: %2%\" %3%"));
static const std::string MasterlistParsingErrorFooter(bloc::translate("Masterlist parsing aborted. Utility will end now."));
static const std::string IniParsingErrorFooter(bloc::translate("Ini parsing aborted. Some or all of the options may not have been set correctly."));
static const std::string RuleListParsingErrorFooter(bloc::translate("Userlist parsing aborted. No rules will be applied."));

// RuleList syntax error strings.
static const std::string ESortLineInForRule(bloc::translate("includes a sort line in a rule with a FOR rule keyword."));
static const std::string EAddingModGroup(bloc::translate("tries to add a group."));
static const std::string ESortingGroupEsms(bloc::translate("tries to sort the group \"ESMs\"."));
static const std::string ESortingMasterEsm(bloc::translate("tries to sort the master .ESM file."));
static const std::string EReferencingModAndGroup(bloc::translate("references a mod and a group."));
static const std::string ESortingGroupBeforeEsms(bloc::translate("tries to sort a group before the group \"ESMs\"."));
static const std::string ESortingModBeforeGameMaster(bloc::translate("tries to sort a mod before the master .ESM file."));
static const std::string EInsertingToTopOfEsms(bloc::translate("tries to insert a mod into the top of the group \"ESMs\", before the master .ESM file."));
static const std::string EInsertingGroupOrIntoMod(bloc::translate("tries to insert a group or insert something into a mod."));
static const std::string EAttachingMessageToGroup(bloc::translate("tries to attach a message to a group."));
static const std::string EMultipleSortLines(bloc::translate("has more than one sort line."));
static const std::string EMultipleReplaceLines(bloc::translate("has more than one REPLACE-using message line."));
static const std::string EReplaceNotFirst(bloc::translate("has a REPLACE-using message line that is not the first message line."));
static const std::string ESortNotSecond(bloc::translate("has a sort line that is not the second line of the rule."));
static const std::string ESortingToItself(bloc::translate("tries to sort a mod or group relative to itself."));
static const std::string EAttachingNonMessage(bloc::translate("tries to attach an malformatted message."));
static const std::string ESortingMasterAfterPlugin(bloc::translate("tries to sort a plugin before a master file."));
static const std::string ESortingPluginBeforeMaster(bloc::translate("tries to sort a master file before a plugin."));

// Parsing error class.
class BOSS_COMMON ParsingError {
 public:
	ParsingError();

	// For parsing errors.
	ParsingError(const std::string inHeader, const std::string inDetail,
	             const std::string inFooter);

	// For userlist syntax errors.
	ParsingError(const std::string inWholeMessage);

	bool Empty() const;
	std::string Header() const;
	std::string Footer() const;
	std::string Detail() const;
	std::string WholeMessage() const;

	void Clear();

 private:
	std::string header;
	std::string footer;
	std::string detail;
	std::string wholeMessage;
};

}  // namespace boss
#endif  // COMMON_ERROR_H_
