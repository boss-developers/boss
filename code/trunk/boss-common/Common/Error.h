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
#include "Common/DllDef.h"

namespace boss {
	using namespace std;

	using boost::format;
	
	//Error Codes. Also return codes more generally.
	//DO NOT CHANGE THEIR VALUES. THEY MUST BE INVARIANT ACROSS RELEASES FOR API USERS.
	BOSS_COMMON const uint32_t BOSS_OK											= 0;

	BOSS_COMMON const uint32_t BOSS_ERROR_NO_MASTER_FILE						= 1;  //Deprecated.
	BOSS_COMMON const uint32_t BOSS_ERROR_FILE_READ_FAIL						= 2;
	BOSS_COMMON const uint32_t BOSS_ERROR_FILE_WRITE_FAIL						= 3;
	BOSS_COMMON const uint32_t BOSS_ERROR_FILE_NOT_UTF8							= 4;
	BOSS_COMMON const uint32_t BOSS_ERROR_FILE_NOT_FOUND						= 5;
	BOSS_COMMON const uint32_t BOSS_ERROR_FILE_PARSE_FAIL						= 6;
	BOSS_COMMON const uint32_t BOSS_ERROR_CONDITION_EVAL_FAIL					= 7;
	BOSS_COMMON const uint32_t BOSS_ERROR_REGEX_EVAL_FAIL						= 8;
	BOSS_COMMON const uint32_t BOSS_ERROR_NO_GAME_DETECTED						= 9;
	BOSS_COMMON const uint32_t BOSS_ERROR_ENCODING_CONVERSION_FAIL				= 10;
	BOSS_COMMON const uint32_t BOSS_ERROR_PLUGIN_BEFORE_MASTER					= 39;
	BOSS_COMMON const uint32_t BOSS_ERROR_INVALID_SYNTAX						= 40;

	BOSS_COMMON const uint32_t BOSS_ERROR_FIND_ONLINE_MASTERLIST_REVISION_FAIL	= 11;
	BOSS_COMMON const uint32_t BOSS_ERROR_FIND_ONLINE_MASTERLIST_DATE_FAIL		= 12;
	BOSS_COMMON const uint32_t BOSS_ERROR_READ_UPDATE_FILE_LIST_FAIL			= 13;
	BOSS_COMMON const uint32_t BOSS_ERROR_FILE_CRC_MISMATCH						= 14;

	BOSS_COMMON const uint32_t BOSS_ERROR_FS_FILE_MOD_TIME_READ_FAIL			= 15;
	BOSS_COMMON const uint32_t BOSS_ERROR_FS_FILE_MOD_TIME_WRITE_FAIL			= 16;
	BOSS_COMMON const uint32_t BOSS_ERROR_FS_FILE_RENAME_FAIL					= 17;
	BOSS_COMMON const uint32_t BOSS_ERROR_FS_FILE_DELETE_FAIL					= 18;
	BOSS_COMMON const uint32_t BOSS_ERROR_FS_CREATE_DIRECTORY_FAIL				= 19;
	BOSS_COMMON const uint32_t BOSS_ERROR_FS_ITER_DIRECTORY_FAIL				= 20;

	BOSS_COMMON const uint32_t BOSS_ERROR_CURL_INIT_FAIL						= 21;
	BOSS_COMMON const uint32_t BOSS_ERROR_CURL_SET_ERRBUFF_FAIL					= 22;
	BOSS_COMMON const uint32_t BOSS_ERROR_CURL_SET_OPTION_FAIL					= 23;
	BOSS_COMMON const uint32_t BOSS_ERROR_CURL_SET_PROXY_FAIL					= 24;
	BOSS_COMMON const uint32_t BOSS_ERROR_CURL_SET_PROXY_TYPE_FAIL				= 25;
	BOSS_COMMON const uint32_t BOSS_ERROR_CURL_SET_PROXY_AUTH_FAIL				= 26;
	BOSS_COMMON const uint32_t BOSS_ERROR_CURL_SET_PROXY_AUTH_TYPE_FAIL			= 27;
	BOSS_COMMON const uint32_t BOSS_ERROR_CURL_PERFORM_FAIL						= 28;
	BOSS_COMMON const uint32_t BOSS_ERROR_CURL_USER_CANCEL						= 29;

	BOSS_COMMON const uint32_t BOSS_ERROR_GUI_WINDOW_INIT_FAIL					= 30;

	BOSS_COMMON const uint32_t BOSS_OK_NO_UPDATE_NECESSARY						= 31;
	BOSS_COMMON const uint32_t BOSS_ERROR_LO_MISMATCH							= 32;
	BOSS_COMMON const uint32_t BOSS_ERROR_NO_MEM								= 33;
	BOSS_COMMON const uint32_t BOSS_ERROR_INVALID_ARGS							= 34;
	BOSS_COMMON const uint32_t BOSS_ERROR_NETWORK_FAIL							= 35;
	BOSS_COMMON const uint32_t BOSS_ERROR_NO_INTERNET_CONNECTION				= 36;
	BOSS_COMMON const uint32_t BOSS_ERROR_NO_TAG_MAP							= 37;
	BOSS_COMMON const uint32_t BOSS_ERROR_PLUGINS_FULL							= 38;

	BOSS_COMMON const uint32_t BOSS_ERROR_MAX = BOSS_ERROR_INVALID_SYNTAX;

	class BOSS_COMMON boss_error {
	public:
		//For general errors not referencing specific files.
		boss_error(const uint32_t internalErrCode) ;

		//For general errors referencing specific files.
		boss_error(const uint32_t internalErrCode, const string internalErrSubject);

		//For errors from BOOST Filesystem functions.
		boss_error(const uint32_t internalErrCode, const string internalErrSubject, const string externalErrString);

		//For errors from cURL functions.
		boss_error(const string externalErrString, const uint32_t internalErrCode);

		//Returns the error code for the object.
		uint32_t getCode();

		//Returns the error string for the object.
		string getString();

	private:
		uint32_t errCode;
		string errString;
		string errSubject;
	};

	//Parsing error formats.
	static format MasterlistParsingErrorHeader("Masterlist Parsing Error: Expected a %1% at:");
	static format IniParsingErrorHeader("Ini Parsing Error: Expected a %1% at:");
	static format RuleListParsingErrorHeader("Userlist Parsing Error: Expected a %1% at:");
	static format RuleListSyntaxErrorMessage("Userlist Syntax Error: The rule beginning \"%1%: %2%\" %3%");
	static const string MasterlistParsingErrorFooter("Masterlist parsing aborted. Utility will end now.");
	static const string IniParsingErrorFooter("Ini parsing aborted. Some or all of the options may not have been set correctly.");
	static const string RuleListParsingErrorFooter("Userlist parsing aborted. No rules will be applied.");

	//RuleList syntax error strings.
	static const string ESortLineInForRule("includes a sort line in a rule with a FOR rule keyword.");
	static const string EAddingModGroup("tries to add a group.");
	static const string ESortingGroupEsms("tries to sort the group \"ESMs\".");
	static const string ESortingMasterEsm("tries to sort the master .ESM file.");
	static const string EReferencingModAndGroup("references a mod and a group.");
	static const string ESortingGroupBeforeEsms("tries to sort a group before the group \"ESMs\".");
	static const string ESortingModBeforeGameMaster("tries to sort a mod before the master .ESM file.");
	static const string EInsertingToTopOfEsms("tries to insert a mod into the top of the group \"ESMs\", before the master .ESM file.");
	static const string EInsertingGroupOrIntoMod("tries to insert a group or insert something into a mod.");
	static const string EAttachingMessageToGroup("tries to attach a message to a group.");
	static const string EMultipleSortLines("has more than one sort line.");
	static const string EMultipleReplaceLines("has more than one REPLACE-using message line.");
	static const string EReplaceNotFirst("has a REPLACE-using message line that is not the first message line.");
	static const string ESortNotSecond("has a sort line that is not the second line of the rule.");
	static const string ESortingToItself("tries to sort a mod or group relative to itself.");
	static const string EAttachingNonMessage("tries to attach an malformatted message.");
	static const string ESortingMasterAfterPlugin("tries to sort a plugin before a master file.");
	static const string ESortingPluginBeforeMaster("tries to sort a master file before a plugin.");
	
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

	private:
		string header;
		string footer;
		string detail;
		string wholeMessage;
	};
}
#endif