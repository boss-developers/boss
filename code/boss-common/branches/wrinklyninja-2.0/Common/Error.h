/*	Better Oblivion Sorting Software
	
	A "one-click" program for users that quickly optimises and avoids 
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge, 
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

    Copyright (C) 2009-2012    BOSS Development Team.

	This file is part of Better Oblivion Sorting Software.

    Better Oblivion Sorting Software is free software: you can redistribute 
	it and/or modify it under the terms of the GNU General Public License 
	as published by the Free Software Foundation, either version 3 of 
	the License, or (at your option) any later version.

    Better Oblivion Sorting Software is distributed in the hope that it will 
	be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Better Oblivion Sorting Software.  If not, see 
	<http://www.gnu.org/licenses/>.

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/
//Contains the BOSS exception class.

#ifndef __BOSS_ERROR_H__
#define __BOSS_ERROR_H__

#include <string>
#include <boost/cstdint.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include "Common/DllDef.h"

namespace boss {
	using namespace std;

	using boost::format;
	
	//Error Codes
	enum : uint32_t {
		BOSS_ERROR_OK,
		BOSS_ERROR_OBLIVION_AND_NEHRIM,
		BOSS_ERROR_NO_MASTER_FILE,
		BOSS_ERROR_FILE_READ_FAIL,
		BOSS_ERROR_FILE_WRITE_FAIL,
		BOSS_ERROR_FILE_NOT_UTF8,
		BOSS_ERROR_FILE_NOT_FOUND,
		BOSS_ERROR_FILE_PARSE_FAIL,
		BOSS_ERROR_CONDITION_EVAL_FAIL,
		BOSS_ERROR_NO_GAME_DETECTED,
		BOSS_ERROR_FIND_ONLINE_MASTERLIST_REVISION_FAIL,
		BOSS_ERROR_FIND_ONLINE_MASTERLIST_DATE_FAIL,
		BOSS_ERROR_READ_UPDATE_FILE_LIST_FAIL,
		BOSS_ERROR_FILE_CRC_MISMATCH,
		BOSS_ERROR_FS_FILE_MOD_TIME_READ_FAIL,
		BOSS_ERROR_FS_FILE_MOD_TIME_WRITE_FAIL,
		BOSS_ERROR_FS_FILE_RENAME_FAIL,
		BOSS_ERROR_FS_FILE_DELETE_FAIL,
		BOSS_ERROR_CURL_INIT_FAIL,
		BOSS_ERROR_CURL_SET_ERRBUFF_FAIL,
		BOSS_ERROR_CURL_SET_OPTION_FAIL,
		BOSS_ERROR_CURL_SET_PROXY_FAIL,
		BOSS_ERROR_CURL_SET_PROXY_TYPE_FAIL,
		BOSS_ERROR_CURL_SET_PROXY_AUTH_FAIL,
		BOSS_ERROR_CURL_SET_PROXY_AUTH_TYPE_FAIL,
		BOSS_ERROR_CURL_PERFORM_FAIL,
		BOSS_ERROR_CURL_USER_CANCEL,
		BOSS_ERROR_GUI_WINDOW_INIT_FAIL,
		BOSS_ERROR_MAX = BOSS_ERROR_GUI_WINDOW_INIT_FAIL
	};

	class boss_error {
	public:
		//For general errors not referencing specific files.
		inline boss_error(const uint32_t internalErrCode) 
			: errCode(internalErrCode), errString(""), errSubject("") {}

		//For general errors referencing specific files.
		inline boss_error(const uint32_t internalErrCode, const string internalErrSubject) 
			: errCode(internalErrCode), errString(""), errSubject(internalErrSubject) {}

		//For errors from BOOST Filesystem functions.
		inline boss_error(const uint32_t internalErrCode, const string internalErrSubject, const string externalErrString) 
			: errCode(internalErrCode), errString(externalErrString), errSubject(internalErrSubject) {}

		//For errors from cURL functions.
		inline boss_error(const string externalErrString, const uint32_t internalErrCode) 
			: errCode(internalErrCode), errString(externalErrString), errSubject("") {}

		//Returns the error string for the object.
		inline string getString() {
			switch(errCode) {
			case BOSS_ERROR_OK:
				return "No error.";
			case BOSS_ERROR_OBLIVION_AND_NEHRIM:
				return "Oblivion.esm and Nehrim.esm both detected!";
			case BOSS_ERROR_NO_MASTER_FILE:
				return "No game master .esm file found!"; 
			case BOSS_ERROR_FILE_READ_FAIL:
				return "\"" + errSubject + "\" cannot be read!"; 
			case BOSS_ERROR_FILE_WRITE_FAIL:
				return "\"" + errSubject + "\" cannot be written to!"; 
			case BOSS_ERROR_FILE_NOT_UTF8:
				return "\"" + errSubject + "\" is not encoded in valid UTF-8!"; 
			case BOSS_ERROR_FILE_NOT_FOUND:
				return "\"" + errSubject + "\" cannot be found!";
			case BOSS_ERROR_CONDITION_EVAL_FAIL:
				return "Evaluation of conditional \"" + errSubject + "\" failed!";
			case BOSS_ERROR_NO_GAME_DETECTED:
				return "No game detected!"; 
			case BOSS_ERROR_FIND_ONLINE_MASTERLIST_REVISION_FAIL:
				return "Cannot find online masterlist revision number!"; 
			case BOSS_ERROR_FIND_ONLINE_MASTERLIST_DATE_FAIL:
				return "Cannot find online masterlist revision date!"; 
			case BOSS_ERROR_READ_UPDATE_FILE_LIST_FAIL:
				return "Cannot read list of files to be updated!"; 
			case BOSS_ERROR_FILE_CRC_MISMATCH:
				return "Downloaded file \"" + errSubject + "\" failed verification test!"; 
			case BOSS_ERROR_FS_FILE_MOD_TIME_READ_FAIL:
				return "The modification date of \"" + errSubject + "\" cannot be read! Filesystem response: \"" + errString + "\".";
			case BOSS_ERROR_FS_FILE_RENAME_FAIL:
				return "\"" + errSubject + "\" cannot be renamed! Filesystem response: \"" + errString + "\".";
			case BOSS_ERROR_FS_FILE_DELETE_FAIL:
				return "\"" + errSubject + "\" cannot be deleted! Filesystem response: \"" + errString + "\".";
			case BOSS_ERROR_CURL_INIT_FAIL:
				return "cURL cannot be initialised!";
			case BOSS_ERROR_CURL_SET_ERRBUFF_FAIL:
				return "cURL's error buffer could not be set! cURL response: \"" + errString + "\".";
			case BOSS_ERROR_CURL_SET_OPTION_FAIL:
				return "A cURL option could not be set! cURL response: \"" + errString + "\".";
			case BOSS_ERROR_CURL_SET_PROXY_FAIL:
				return "Proxy hostname or port invalid! cURL response: \"" + errString + "\".";
			case BOSS_ERROR_CURL_SET_PROXY_TYPE_FAIL:
				return "Failed to set proxy type! cURL response: \"" + errString + "\".";
			case BOSS_ERROR_CURL_SET_PROXY_AUTH_FAIL:
				return "Proxy authentication username or password invalid! cURL response: \"" + errString + "\".";
			case BOSS_ERROR_CURL_SET_PROXY_AUTH_TYPE_FAIL:
				return "Failed to set proxy authentication type! cURL response: \"" + errString + "\".";
			case BOSS_ERROR_CURL_PERFORM_FAIL:
				return "cURL could not perform task! cURL response: \"" + errString + "\".";
			case BOSS_ERROR_CURL_USER_CANCEL:
				return "Cancelled by user.";
			case BOSS_ERROR_FILE_PARSE_FAIL:
				return "Parsing of \"" + errSubject + "\" failed!";
			case BOSS_ERROR_FS_FILE_MOD_TIME_WRITE_FAIL:
				return "The modification date of \"" + errSubject + "\" cannot be written! Filesystem response: \"" + errString + "\".";
			case BOSS_ERROR_GUI_WINDOW_INIT_FAIL:
				return "The window \"" + errSubject + "\" failed to initialise. Details: \"" + errString + "\".";
			default:
				return "No error.";
			}
		}

		//Returns the error code for the object.
		inline uint32_t getCode() { return errCode; }
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
	
	//Parsing error class.
	class ParsingError {
	public:
		inline ParsingError() : header(""), footer(""), detail(""), wholeMessage("") {}
		//For parsing errors.
		inline ParsingError(const string inHeader, const string inDetail, const string inFooter)
			: header(inHeader), detail(inDetail), footer(inFooter) {}

		//For userlist syntax errors.
		inline ParsingError(const string inWholeMessage) 
			: wholeMessage(inWholeMessage) {}

		//Outputs correctly-formatted error message.
		string FormatFor(const uint32_t format);

		inline bool Empty() { return (header.empty() && footer.empty() && detail.empty() && wholeMessage.empty()); }
	private:
		string header;
		string footer;
		string detail;
		string wholeMessage;
	};
}
#endif