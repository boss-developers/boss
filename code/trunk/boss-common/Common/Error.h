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
//Contains the BOSS exception class.

#ifndef __BOSS_ERROR_H__
#define __BOSS_ERROR_H__

#include <string>
#include <boost/cstdint.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include "Common/DllDef.h"

namespace boss {
	using namespace std;

	namespace loc = boost::locale;
	
	//Return codes, mostly error codes.
	BOSS_COMMON extern const uint32_t BOSS_OK;

	BOSS_COMMON extern const uint32_t BOSS_ERROR_NO_MASTER_FILE;  //Deprecated.
	BOSS_COMMON extern const uint32_t BOSS_ERROR_FILE_READ_FAIL;
	BOSS_COMMON extern const uint32_t BOSS_ERROR_FILE_WRITE_FAIL;
	BOSS_COMMON extern const uint32_t BOSS_ERROR_FILE_NOT_UTF8;
	BOSS_COMMON extern const uint32_t BOSS_ERROR_FILE_NOT_FOUND;
	BOSS_COMMON extern const uint32_t BOSS_ERROR_FILE_PARSE_FAIL;
	BOSS_COMMON extern const uint32_t BOSS_ERROR_CONDITION_EVAL_FAIL;
	BOSS_COMMON extern const uint32_t BOSS_ERROR_REGEX_EVAL_FAIL;
	BOSS_COMMON extern const uint32_t BOSS_ERROR_NO_GAME_DETECTED;
	BOSS_COMMON extern const uint32_t BOSS_ERROR_ENCODING_CONVERSION_FAIL;
	BOSS_COMMON extern const uint32_t BOSS_ERROR_PLUGIN_BEFORE_MASTER;
	BOSS_COMMON extern const uint32_t BOSS_ERROR_INVALID_SYNTAX;

	BOSS_COMMON extern const uint32_t BOSS_ERROR_FIND_ONLINE_MASTERLIST_REVISION_FAIL;
	BOSS_COMMON extern const uint32_t BOSS_ERROR_FIND_ONLINE_MASTERLIST_DATE_FAIL;
	BOSS_COMMON extern const uint32_t BOSS_ERROR_READ_UPDATE_FILE_LIST_FAIL;
	BOSS_COMMON extern const uint32_t BOSS_ERROR_FILE_CRC_MISMATCH;

	BOSS_COMMON extern const uint32_t BOSS_ERROR_FS_FILE_MOD_TIME_READ_FAIL;
	BOSS_COMMON extern const uint32_t BOSS_ERROR_FS_FILE_MOD_TIME_WRITE_FAIL;
	BOSS_COMMON extern const uint32_t BOSS_ERROR_FS_FILE_RENAME_FAIL;
	BOSS_COMMON extern const uint32_t BOSS_ERROR_FS_FILE_DELETE_FAIL;
	BOSS_COMMON extern const uint32_t BOSS_ERROR_FS_CREATE_DIRECTORY_FAIL;
	BOSS_COMMON extern const uint32_t BOSS_ERROR_FS_ITER_DIRECTORY_FAIL;

	BOSS_COMMON extern const uint32_t BOSS_ERROR_CURL_INIT_FAIL;
	BOSS_COMMON extern const uint32_t BOSS_ERROR_CURL_SET_ERRBUFF_FAIL;
	BOSS_COMMON extern const uint32_t BOSS_ERROR_CURL_SET_OPTION_FAIL;
	BOSS_COMMON extern const uint32_t BOSS_ERROR_CURL_SET_PROXY_FAIL;
	BOSS_COMMON extern const uint32_t BOSS_ERROR_CURL_SET_PROXY_TYPE_FAIL;
	BOSS_COMMON extern const uint32_t BOSS_ERROR_CURL_SET_PROXY_AUTH_FAIL;
	BOSS_COMMON extern const uint32_t BOSS_ERROR_CURL_SET_PROXY_AUTH_TYPE_FAIL;
	BOSS_COMMON extern const uint32_t BOSS_ERROR_CURL_PERFORM_FAIL;
	BOSS_COMMON extern const uint32_t BOSS_ERROR_CURL_USER_CANCEL;

	BOSS_COMMON extern const uint32_t BOSS_ERROR_GUI_WINDOW_INIT_FAIL;

	BOSS_COMMON extern const uint32_t BOSS_OK_NO_UPDATE_NECESSARY;
	BOSS_COMMON extern const uint32_t BOSS_ERROR_LO_MISMATCH;
	BOSS_COMMON extern const uint32_t BOSS_ERROR_NO_MEM;
	BOSS_COMMON extern const uint32_t BOSS_ERROR_INVALID_ARGS;
	BOSS_COMMON extern const uint32_t BOSS_ERROR_NETWORK_FAIL;
	BOSS_COMMON extern const uint32_t BOSS_ERROR_NO_INTERNET_CONNECTION;
	BOSS_COMMON extern const uint32_t BOSS_ERROR_NO_TAG_MAP;
	BOSS_COMMON extern const uint32_t BOSS_ERROR_PLUGINS_FULL;

	BOSS_COMMON extern const uint32_t BOSS_ERROR_MAX;

	class BOSS_COMMON boss_error {
	public:
		//For general errors not referencing specific files.
		boss_error(const uint32_t internalErrCode);

		//For general errors referencing specific files.
		boss_error(const uint32_t internalErrCode, const string internalErrSubject);

		//For errors from BOOST Filesystem functions.
		boss_error(const uint32_t internalErrCode, const string internalErrSubject, const string externalErrString);

		//For errors from cURL functions.
		boss_error(const string externalErrString, const uint32_t internalErrCode);

		//Returns the error code for the object.
		uint32_t getCode() const;

		//Returns the error string for the object.
		string getString() const;

	private:
		uint32_t errCode;
		string errString;
		string errSubject;
	};

	//Parsing error formats.
	static boost::format MasterlistParsingErrorHeader(loc::translate("Masterlist Parsing Error: Expected a %1% at:"));
	static boost::format IniParsingErrorHeader(loc::translate("Ini Parsing Error: Expected a %1% at:"));
	static boost::format RuleListParsingErrorHeader(loc::translate("Userlist Parsing Error: Expected a %1% at:"));
	static boost::format RuleListSyntaxErrorMessage(loc::translate("Userlist Syntax Error: The rule beginning \"%1%: %2%\" %3%"));
	static const string MasterlistParsingErrorFooter(loc::translate("Masterlist parsing aborted. Utility will end now."));
	static const string IniParsingErrorFooter(loc::translate("Ini parsing aborted. Some or all of the options may not have been set correctly."));
	static const string RuleListParsingErrorFooter(loc::translate("Userlist parsing aborted. No rules will be applied."));

	//RuleList syntax error strings.
	static const string ESortLineInForRule(loc::translate("includes a sort line in a rule with a FOR rule keyword."));
	static const string EAddingModGroup(loc::translate("tries to add a group."));
	static const string ESortingGroupEsms(loc::translate("tries to sort the group \"ESMs\"."));
	static const string ESortingMasterEsm(loc::translate("tries to sort the master .ESM file."));
	static const string EReferencingModAndGroup(loc::translate("references a mod and a group."));
	static const string ESortingGroupBeforeEsms(loc::translate("tries to sort a group before the group \"ESMs\"."));
	static const string ESortingModBeforeGameMaster(loc::translate("tries to sort a mod before the master .ESM file."));
	static const string EInsertingToTopOfEsms(loc::translate("tries to insert a mod into the top of the group \"ESMs\", before the master .ESM file."));
	static const string EInsertingGroupOrIntoMod(loc::translate("tries to insert a group or insert something into a mod."));
	static const string EAttachingMessageToGroup(loc::translate("tries to attach a message to a group."));
	static const string EMultipleSortLines(loc::translate("has more than one sort line."));
	static const string EMultipleReplaceLines(loc::translate("has more than one REPLACE-using message line."));
	static const string EReplaceNotFirst(loc::translate("has a REPLACE-using message line that is not the first message line."));
	static const string ESortNotSecond(loc::translate("has a sort line that is not the second line of the rule."));
	static const string ESortingToItself(loc::translate("tries to sort a mod or group relative to itself."));
	static const string EAttachingNonMessage(loc::translate("tries to attach an malformatted message."));
	static const string ESortingMasterAfterPlugin(loc::translate("tries to sort a plugin before a master file."));
	static const string ESortingPluginBeforeMaster(loc::translate("tries to sort a master file before a plugin."));
	
	//Parsing error class.
	class BOSS_COMMON ParsingError {
	public:
		ParsingError();

		//For parsing errors.
		ParsingError(const string inHeader, const string inDetail, const string inFooter);

		//For userlist syntax errors.
		ParsingError(const string inWholeMessage);

		bool Empty() const;
		string Header() const;
		string Footer() const;
		string Detail() const;
		string WholeMessage() const;

		void Clear();

	private:
		string header;
		string footer;
		string detail;
		string wholeMessage;
	};
}
#endif