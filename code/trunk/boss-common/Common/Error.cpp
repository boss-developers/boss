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

#include "Common/Error.h"
#include <boost/algorithm/string.hpp>


namespace boss {
	using namespace std;

	//Return codes, mostly error codes.
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

	////////////////////////////////
	// boss_error Class Functions
	////////////////////////////////

	//For general errors not referencing specific files.
	boss_error::boss_error(const uint32_t internalErrCode) 
			: errCode(internalErrCode), errString(""), errSubject("") 
	{}

	//For general errors referencing specific files.
	boss_error::boss_error(const uint32_t internalErrCode, const string internalErrSubject) 
			: errCode(internalErrCode), errString(""), errSubject(internalErrSubject) 
	{}

	//For errors from BOOST Filesystem functions.
	boss_error::boss_error(const uint32_t internalErrCode, const string internalErrSubject, const string externalErrString) 
			: errCode(internalErrCode), errString(externalErrString), errSubject(internalErrSubject) 
	{}

	//For errors from cURL functions.
	boss_error::boss_error(const string externalErrString, const uint32_t internalErrCode) 
			: errCode(internalErrCode), errString(externalErrString), errSubject("") 
	{}

	//Returns the error code for the object.
	uint32_t boss_error::getCode() const { 
		return errCode; 
	}

	//Returns the error string for the object.
	string boss_error::getString() const {
		if (errCode == BOSS_OK)
			return "No error.";
		else if (errCode == BOSS_ERROR_NO_MASTER_FILE)
			return "No game master .esm file found!"; 
		else if (errCode == BOSS_ERROR_FILE_READ_FAIL)
			return "\"" + errSubject + "\" cannot be read!"; 
		else if (errCode == BOSS_ERROR_FILE_WRITE_FAIL)
			return "\"" + errSubject + "\" cannot be written to!"; 
		else if (errCode == BOSS_ERROR_FILE_NOT_UTF8)
			return "\"" + errSubject + "\" is not encoded in valid UTF-8!"; 
		else if (errCode == BOSS_ERROR_FILE_NOT_FOUND)
			return "\"" + errSubject + "\" cannot be found!";
		else if (errCode == BOSS_ERROR_CONDITION_EVAL_FAIL)
			return "Evaluation of conditional \"" + errSubject + "\" failed!";
		else if (errCode == BOSS_ERROR_REGEX_EVAL_FAIL)
			return "\"" + errSubject + "\" is not a valid regular expression. Item skipped.";
		else if (errCode == BOSS_ERROR_NO_GAME_DETECTED)
			return "No game detected!"; 
		else if (errCode == BOSS_ERROR_ENCODING_CONVERSION_FAIL)
			return "\"" + errSubject + "\" cannot be converted from UTF-8 to \"" + errString + "\".";
		else if (errCode == BOSS_ERROR_PLUGIN_BEFORE_MASTER)
			return "Master file \"" + errSubject +  "\" loading after non-master plugins!";
		else if (errCode == BOSS_ERROR_INVALID_SYNTAX)
			return errString;
		else if (errCode == BOSS_ERROR_FIND_ONLINE_MASTERLIST_REVISION_FAIL)
			return "Cannot find online masterlist revision number!"; 
		else if (errCode == BOSS_ERROR_FIND_ONLINE_MASTERLIST_DATE_FAIL)
			return "Cannot find online masterlist revision date!"; 
		else if (errCode == BOSS_ERROR_READ_UPDATE_FILE_LIST_FAIL)
			return "Cannot read list of files to be updated!"; 
		else if (errCode == BOSS_ERROR_FILE_CRC_MISMATCH)
			return "Downloaded file \"" + errSubject + "\" failed verification test!"; 
		else if (errCode == BOSS_ERROR_FS_FILE_MOD_TIME_READ_FAIL)
			return "The modification date of \"" + errSubject + "\" cannot be read! Filesystem response: \"" + errString + "\".";
		else if (errCode == BOSS_ERROR_FS_FILE_RENAME_FAIL)
			return "\"" + errSubject + "\" cannot be renamed! Filesystem response: \"" + errString + "\".";
		else if (errCode == BOSS_ERROR_FS_FILE_DELETE_FAIL)
			return "\"" + errSubject + "\" cannot be deleted! Filesystem response: \"" + errString + "\".";
		else if (errCode == BOSS_ERROR_FS_CREATE_DIRECTORY_FAIL)
			return "\"" + errSubject + "\" cannot be created! Filesystem response: \"" + errString + "\".";
		else if (errCode == BOSS_ERROR_FS_ITER_DIRECTORY_FAIL)
			return "\"" + errSubject + "\" cannot be scanned! Filesystem response: \"" + errString + "\".";
		else if (errCode == BOSS_ERROR_CURL_INIT_FAIL)
			return "cURL cannot be initialised!";
		else if (errCode == BOSS_ERROR_CURL_SET_ERRBUFF_FAIL)
			return "cURL's error buffer could not be set! cURL response: \"" + errString + "\".";
		else if (errCode == BOSS_ERROR_CURL_SET_OPTION_FAIL)
			return "A cURL option could not be set! cURL response: \"" + errString + "\".";
		else if (errCode == BOSS_ERROR_CURL_SET_PROXY_FAIL)
			return "Proxy hostname or port invalid! cURL response: \"" + errString + "\".";
		else if (errCode == BOSS_ERROR_CURL_SET_PROXY_TYPE_FAIL)
			return "Failed to set proxy type! cURL response: \"" + errString + "\".";
		else if (errCode == BOSS_ERROR_CURL_SET_PROXY_AUTH_FAIL)
			return "Proxy authentication username or password invalid! cURL response: \"" + errString + "\".";
		else if (errCode == BOSS_ERROR_CURL_SET_PROXY_AUTH_TYPE_FAIL)
			return "Failed to set proxy authentication type! cURL response: \"" + errString + "\".";
		else if (errCode == BOSS_ERROR_CURL_PERFORM_FAIL)
			return "cURL could not perform task! cURL response: \"" + errString + "\".";
		else if (errCode == BOSS_ERROR_CURL_USER_CANCEL)
			return "Cancelled by user.";
		else if (errCode == BOSS_ERROR_FILE_PARSE_FAIL)
			return "Parsing of \"" + errSubject + "\" failed!";
		else if (errCode == BOSS_ERROR_FS_FILE_MOD_TIME_WRITE_FAIL)
			return "The modification date of \"" + errSubject + "\" cannot be written! Filesystem response: \"" + errString + "\".";
		else if (errCode == BOSS_ERROR_GUI_WINDOW_INIT_FAIL)
			return "The window \"" + errSubject + "\" failed to initialise. Details: \"" + errString + "\".";
		else
			return "No error.";
	}


	//////////////////////////////////
	// ParsingError Class Functions
	//////////////////////////////////

	ParsingError::ParsingError() 
		: header(""), footer(""), detail(""), wholeMessage("") 
	{}

	//For parsing errors.
	ParsingError::ParsingError(const string inHeader, const string inDetail, const string inFooter)
		: header(inHeader), detail(inDetail), footer(inFooter) 
	{}

	//For userlist syntax errors.
	ParsingError::ParsingError(const string inWholeMessage) 
		: wholeMessage(inWholeMessage) 
	{}

	bool ParsingError::Empty() const { 
		return (header.empty() && footer.empty() && detail.empty() && wholeMessage.empty()); 
	}

	string ParsingError::Header() const { 
		return header; 
	}

	string ParsingError::Footer() const { 
		return footer; 
	}

	string ParsingError::Detail() const { 
		return detail; 
	}

	string ParsingError::WholeMessage() const { 
		return wholeMessage; 
	}

	void ParsingError::Clear() {
		header.clear();
		footer.clear();
		detail.clear();
		wholeMessage.clear();
	}
}