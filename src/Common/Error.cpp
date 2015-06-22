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

	using boost::format;
	using boost::locale::translate;

	//Return codes, mostly error codes.
	BOSS_COMMON const uint32_t BOSS_OK                                  = 0;
	BOSS_COMMON const uint32_t BOSS_ERROR_FILE_READ_FAIL                = 2;
	BOSS_COMMON const uint32_t BOSS_ERROR_FILE_WRITE_FAIL               = 3;
	BOSS_COMMON const uint32_t BOSS_ERROR_FILE_NOT_UTF8                 = 4;
	BOSS_COMMON const uint32_t BOSS_ERROR_FILE_NOT_FOUND                = 5;
	BOSS_COMMON const uint32_t BOSS_ERROR_FILE_PARSE_FAIL               = 6;
	BOSS_COMMON const uint32_t BOSS_ERROR_CONDITION_EVAL_FAIL           = 7;
	BOSS_COMMON const uint32_t BOSS_ERROR_REGEX_EVAL_FAIL               = 8;
	BOSS_COMMON const uint32_t BOSS_ERROR_NO_GAME_DETECTED              = 9;
	BOSS_COMMON const uint32_t BOSS_ERROR_ENCODING_CONVERSION_FAIL      = 10;
	BOSS_COMMON const uint32_t BOSS_ERROR_PLUGIN_BEFORE_MASTER          = 39;
	BOSS_COMMON const uint32_t BOSS_ERROR_INVALID_SYNTAX                = 40;

	BOSS_COMMON const uint32_t BOSS_ERROR_GIT_ERROR                     = 41;

	BOSS_COMMON const uint32_t BOSS_ERROR_FS_FILE_MOD_TIME_READ_FAIL    = 15;
	BOSS_COMMON const uint32_t BOSS_ERROR_FS_FILE_MOD_TIME_WRITE_FAIL   = 16;
	BOSS_COMMON const uint32_t BOSS_ERROR_FS_FILE_RENAME_FAIL           = 17;
	BOSS_COMMON const uint32_t BOSS_ERROR_FS_FILE_DELETE_FAIL           = 18;
	BOSS_COMMON const uint32_t BOSS_ERROR_FS_CREATE_DIRECTORY_FAIL      = 19;
	BOSS_COMMON const uint32_t BOSS_ERROR_FS_ITER_DIRECTORY_FAIL        = 20;

	BOSS_COMMON const uint32_t BOSS_ERROR_GUI_WINDOW_INIT_FAIL          = 30;

	////////////////////////////////
	// boss_error Class Functions
	////////////////////////////////

	//For general errors not referencing specific files.
	boss_error::boss_error(const uint32_t internalErrCode)
	    : errCode(internalErrCode),
	      errString(""),
	      errSubject("") {}

	//For general errors referencing specific files.
	boss_error::boss_error(const uint32_t internalErrCode,
	                       const string internalErrSubject)
	    : errCode(internalErrCode),
	      errString(""),
	      errSubject(internalErrSubject) {}

	//For errors from BOOST Filesystem functions.
	boss_error::boss_error(const uint32_t internalErrCode,
	                       const string internalErrSubject,
	                       const string externalErrString)
	    : errCode(internalErrCode),
	      errString(externalErrString),
	      errSubject(internalErrSubject) {}

	//For errors from other external functions.
	boss_error::boss_error(const string externalErrString,
	                       const uint32_t internalErrCode)
	    : errCode(internalErrCode),
	      errString(externalErrString),
	      errSubject("") {}

	//Returns the error code for the object.
	uint32_t boss_error::getCode() const {
		return errCode;
	}

	//Returns the error string for the object.
	string boss_error::getString() const {
		// TODO(MCP): Convert this to a switch-statement.
		if (errCode == BOSS_OK)
			return translate("No error.");
		else if (errCode == BOSS_ERROR_FILE_READ_FAIL)
			return (format(translate("\"%1%\" cannot be read!")) % errSubject).str();
		else if (errCode == BOSS_ERROR_FILE_WRITE_FAIL)
			return (format(translate("\"%1%\" cannot be written to!")) % errSubject).str();
		else if (errCode == BOSS_ERROR_FILE_NOT_UTF8)
			return (format(translate("\"%1%\" is not encoded in valid UTF-8!")) % errSubject).str();
		else if (errCode == BOSS_ERROR_FILE_NOT_FOUND)
			return (format(translate("\"%1%\" cannot be found!")) % errSubject).str();
		else if (errCode == BOSS_ERROR_CONDITION_EVAL_FAIL)
			return (format(translate("Evaluation of conditional \"%1%\" for item \"%2%\" failed!")) % errSubject % errString).str();
		else if (errCode == BOSS_ERROR_REGEX_EVAL_FAIL)
			return (format(translate("\"%1%\" is not a valid regular expression. Item skipped.")) % errSubject).str();
		else if (errCode == BOSS_ERROR_NO_GAME_DETECTED)
			return translate("No game detected!");
		else if (errCode == BOSS_ERROR_ENCODING_CONVERSION_FAIL)
			return (format(translate("\"%1%\" cannot be converted from UTF-8 to \"%2%\".")) % errSubject % errString).str();
		else if (errCode == BOSS_ERROR_PLUGIN_BEFORE_MASTER)
			return (format(translate("Master file \"%1%\" loading after non-master plugins!")) % errSubject).str();
		else if (errCode == BOSS_ERROR_INVALID_SYNTAX)
			return errString;
		else if (errCode == BOSS_ERROR_GIT_ERROR)
			return (format(translate("Git operation failed. Error: %1%")) % errString).str();
		else if (errCode == BOSS_ERROR_FS_FILE_MOD_TIME_READ_FAIL)
			return (format(translate("The modification date of \"%1%\" cannot be read! Filesystem response: \"%2%\".")) % errSubject % errString).str();
		else if (errCode == BOSS_ERROR_FS_FILE_RENAME_FAIL)
			return (format(translate("\"%1%\" cannot be renamed! Filesystem response: \"%2%\".")) % errSubject % errString).str();
		else if (errCode == BOSS_ERROR_FS_FILE_DELETE_FAIL)
			return (format(translate("\"%1%\" cannot be deleted! Filesystem response: \"%2%\".")) % errSubject % errString).str();
		else if (errCode == BOSS_ERROR_FS_CREATE_DIRECTORY_FAIL)
			return (format(translate("\"%1%\" cannot be created! Filesystem response: \"%2%\".")) % errSubject % errString).str();
		else if (errCode == BOSS_ERROR_FS_ITER_DIRECTORY_FAIL)
			return (format(translate("\"%1%\" cannot be scanned! Filesystem response: \"%2%\".")) % errSubject % errString).str();
		else if (errCode == BOSS_ERROR_FILE_PARSE_FAIL)
			return (format(translate("Parsing of \"%1%\" failed!")) % errSubject).str();
		else if (errCode == BOSS_ERROR_FS_FILE_MOD_TIME_WRITE_FAIL)
			return (format(translate("The modification date of \"%1%\" cannot be written! Filesystem response: \"%2%\".")) % errSubject % errString).str();
		else if (errCode == BOSS_ERROR_GUI_WINDOW_INIT_FAIL)
			return (format(translate("The window \"%1%\" failed to initialise. Details: \"%2%\".")) % errSubject % errString).str();
		else
			return translate("No error.");
	}


	//////////////////////////////////
	// ParsingError Class Functions
	//////////////////////////////////

	ParsingError::ParsingError()
	    : header(""),
	      footer(""),
	      detail(""),
	      wholeMessage("") {}

	//For parsing errors.
	ParsingError::ParsingError(const string inHeader,
	                           const string inDetail,
	                           const string inFooter)
	    : header(inHeader),
	      detail(inDetail),
	      footer(inFooter) {}

	//For userlist syntax errors.
	ParsingError::ParsingError(const string inWholeMessage)
	    : wholeMessage(inWholeMessage) {}

	bool ParsingError::Empty() const {
		return (header.empty() &&
		        footer.empty() &&
		        detail.empty() &&
		        wholeMessage.empty());
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