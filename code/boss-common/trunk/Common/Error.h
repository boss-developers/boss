/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/
//Contains the BOSS exception class.

#ifndef __BOSS_ERROR_H__
#define __BOSS_ERROR_H__

#include <string>
#include <boost/cstdint.hpp>
#include "Common/DllDef.h"
#include <boost/format.hpp>
#include "Output/Output.h"

namespace boss {
	using namespace std;

	using boost::format;
	
	//Error Codes
	BOSS_COMMON_EXP const uint32_t BOSS_ERROR_OK = 0;
	BOSS_COMMON_EXP const uint32_t BOSS_ERROR_OBLIVION_AND_NEHRIM = 1;
	BOSS_COMMON_EXP const uint32_t BOSS_ERROR_NO_MASTER_FILE = 2;
	BOSS_COMMON_EXP const uint32_t BOSS_ERROR_FILE_READ_FAIL = 3;
	BOSS_COMMON_EXP const uint32_t BOSS_ERROR_FILE_WRITE_FAIL = 4;
	BOSS_COMMON_EXP const uint32_t BOSS_ERROR_FILE_NOT_UTF8 = 5;
	BOSS_COMMON_EXP const uint32_t BOSS_ERROR_FILE_NOT_FOUND = 6;
	BOSS_COMMON_EXP const uint32_t BOSS_ERROR_NO_GAME_DETECTED = 7;
	BOSS_COMMON_EXP const uint32_t BOSS_ERROR_FIND_ONLINE_MASTERLIST_REVISION_FAIL = 8;
	BOSS_COMMON_EXP const uint32_t BOSS_ERROR_FIND_ONLINE_MASTERLIST_DATE_FAIL = 9;
	BOSS_COMMON_EXP const uint32_t BOSS_ERROR_READ_UPDATE_FILE_LIST_FAIL = 10;
	BOSS_COMMON_EXP const uint32_t BOSS_ERROR_FILE_CRC_MISMATCH = 11;
	BOSS_COMMON_EXP const uint32_t BOSS_ERROR_FS_FILE_MOD_TIME_READ_FAIL = 12;
	BOSS_COMMON_EXP const uint32_t BOSS_ERROR_FS_FILE_RENAME_FAIL = 13;
	BOSS_COMMON_EXP const uint32_t BOSS_ERROR_FS_FILE_DELETE_FAIL = 14;
	BOSS_COMMON_EXP const uint32_t BOSS_ERROR_CURL_INIT_FAIL = 15;
	BOSS_COMMON_EXP const uint32_t BOSS_ERROR_CURL_SET_ERRBUFF_FAIL = 16;
	BOSS_COMMON_EXP const uint32_t BOSS_ERROR_CURL_SET_OPTION_FAIL = 17;
	BOSS_COMMON_EXP const uint32_t BOSS_ERROR_CURL_SET_PROXY_FAIL = 18;
	BOSS_COMMON_EXP const uint32_t BOSS_ERROR_CURL_SET_PROXY_TYPE_FAIL = 19;
	BOSS_COMMON_EXP const uint32_t BOSS_ERROR_CURL_SET_PROXY_AUTH_FAIL = 20;
	BOSS_COMMON_EXP const uint32_t BOSS_ERROR_CURL_SET_PROXY_AUTH_TYPE_FAIL = 21;
	BOSS_COMMON_EXP const uint32_t BOSS_ERROR_CURL_PERFORM_FAIL = 22;
	BOSS_COMMON_EXP const uint32_t BOSS_ERROR_CURL_USER_CANCEL = 23;
	BOSS_COMMON_EXP const uint32_t BOSS_ERROR_FILE_PARSE_FAIL = 24;
	BOSS_COMMON_EXP const uint32_t BOSS_ERROR_MAX = BOSS_ERROR_FILE_PARSE_FAIL;

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
				return "The modification date of \"" + errSubject + "\" cannot be read! Filesystem response: " + errString;
			case BOSS_ERROR_FS_FILE_RENAME_FAIL:
				return "\"" + errSubject + "\" cannot be renamed! Filesystem response: " + errString;
			case BOSS_ERROR_FS_FILE_DELETE_FAIL:
				return "\"" + errSubject + "\" cannot be deleted! Filesystem response: " + errString;
			case BOSS_ERROR_CURL_INIT_FAIL:
				return "cURL cannot be initialised!";
			case BOSS_ERROR_CURL_SET_ERRBUFF_FAIL:
				return "cURL's error buffer could not be set! cURL response: " + errString;
			case BOSS_ERROR_CURL_SET_OPTION_FAIL:
				return "A cURL option could not be set! cURL response: " + errString;
			case BOSS_ERROR_CURL_SET_PROXY_FAIL:
				return "Proxy hostname or port invalid! cURL response: " + errString;
			case BOSS_ERROR_CURL_SET_PROXY_TYPE_FAIL:
				return "Failed to set proxy type! cURL response: " + errString;
			case BOSS_ERROR_CURL_SET_PROXY_AUTH_FAIL:
				return "Proxy authentication username or password invalid! cURL response: " + errString;
			case BOSS_ERROR_CURL_SET_PROXY_AUTH_TYPE_FAIL:
				return "Failed to set proxy authentication type! cURL response: " + errString;
			case BOSS_ERROR_CURL_PERFORM_FAIL:
				return "cURL could not perform task! cURL response: " + errString;
			case BOSS_ERROR_CURL_USER_CANCEL:
				return "Cancelled by user.";
			case BOSS_ERROR_FILE_PARSE_FAIL:
				return "Parsing of \"" + errSubject + "\" failed!";
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
		//For parsing errors.
		inline ParsingError(const string inHeader, const string inFooter, const string inDetail)
			: header(inHeader), footer(inFooter), detail(inDetail) {}

		//For userlist syntax errors.
		inline ParsingError(const string inWholeMessage) 
			: wholeMessage(inWholeMessage) {}

		//Outputs correctly-formatted error message.
		inline string FormatFor(const string format) {
			if (!wholeMessage.empty()) {
				if (format == "html")
					return "<li class='error'>"+EscapeHTMLSpecial(wholeMessage);
				else
					return wholeMessage;
			} else {
				if (format == "html")
					return "<li><span class='error'>"+EscapeHTMLSpecial(header)+"</span><blockquote>"+EscapeHTMLSpecial(detail)+"</blockquote><span class='error'>"+EscapeHTMLSpecial(footer)+"</span>";
				else
					return "\n*  "+header+"\n\n"+detail+"\n\n"+footer;
			}
		}
	private:
		string header;
		string footer;
		string detail;
		string wholeMessage;
	};
}
#endif