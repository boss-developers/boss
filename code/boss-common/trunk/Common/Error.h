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

namespace boss {
	using namespace std;

	enum bossErrCode {
		BOSS_ERROR_OK,
		BOSS_ERROR_OBLIVION_AND_NEHRIM,
		BOSS_ERROR_NO_MASTER_FILE,
		BOSS_ERROR_FILE_OPEN_FAIL,
		BOSS_ERROR_FILE_NOT_UTF8,
		BOSS_ERROR_MASTERLIST_NOT_FOUND,
		BOSS_ERROR_NO_GAME_DETECTED,
		BOSS_ERROR_FIND_ONLINE_MASTERLIST_REVISION_FAIL,
		BOSS_ERROR_FIND_ONLINE_MASTERLIST_DATE_FAIL,
		BOSS_ERROR_READ_UPDATE_FILE_LIST_FAIL,
		BOSS_ERROR_FILE_CRC_MISMATCH,
		BOSS_ERROR_INVALID_PROXY_TYPE,
		BOSS_ERROR_FS_FILE_MOD_TIME_READ_FAIL,
		BOSS_ERROR_FS_FILE_RENAME_FAIL,
		BOSS_ERROR_FS_FILE_DELETE_FAIL,
		BOSS_ERROR_CURL_INIT_FAIL,
		BOSS_ERROR_CURL_SET_ERRBUFF_FAIL,
		BOSS_ERROR_CURL_SET_OPTION_FAIL,
		BOSS_ERROR_CURL_SET_PROXY_FAIL,
		BOSS_ERROR_CURL_SET_PROXY_TYPE_FAIL,
		BOSS_ERROR_CURL_PERFORM_FAIL,
		BOSS_ERROR_CURL_USER_CANCEL
	};

	class boss_error {
	public:
		//This will be unused.
		boss_error() {
			errCode = BOSS_ERROR_OK;
			externalErrString = "";
			errSubject = "";
		}
		//For general errors not referencing specific files.
		boss_error(bossErrCode internalErrCode) {
			errCode = internalErrCode;
			externalErrString = "";
			errSubject = "";
		}
		//For general errors referencing specific files.
		boss_error(bossErrCode internalErrCode, string internalErrSubject) {
			errCode = internalErrCode;
			externalErrString = "";
			errSubject = internalErrSubject;
		}
		//For errors from BOOST Filesystem functions.
		boss_error(bossErrCode internalErrCode, string internalErrSubject, string errString) {
			errCode = internalErrCode;
			externalErrString = errString;
			errSubject = internalErrSubject;
		}
		//For errors from cURL functions.
		boss_error(string externalErrString, bossErrCode internalErrCode) {
			errCode = internalErrCode;
			externalErrString = externalErrString;
			errSubject = "";
		}
		string getString() {
			switch(errCode) {
			case BOSS_ERROR_OK:
				return "No error.";
			case BOSS_ERROR_OBLIVION_AND_NEHRIM:
				return "Oblivion.esm and Nehrim.esm both detected!";
			case BOSS_ERROR_NO_MASTER_FILE:
				return "No game master .esm file found!"; 
			case BOSS_ERROR_FILE_OPEN_FAIL:
				return "\"" + errSubject + "\" cannot be opened!"; 
			case BOSS_ERROR_FILE_NOT_UTF8:
				return "\"" + errSubject + "\" is not encoded in valid UTF-8!"; 
			case BOSS_ERROR_MASTERLIST_NOT_FOUND:
				return "\"masterlist.txt\" cannot be found!"; 
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
			case BOSS_ERROR_INVALID_PROXY_TYPE:
				return "\"" + errSubject + "\" is not a valid proxy type!";
			case BOSS_ERROR_FS_FILE_MOD_TIME_READ_FAIL:
				return "The modification date of \"" + errSubject + "\" cannot be read! Filesystem response: " + externalErrString;
			case BOSS_ERROR_FS_FILE_RENAME_FAIL:
				return "\"" + errSubject + "\" cannot be renamed! Filesystem response: " + externalErrString;
			case BOSS_ERROR_FS_FILE_DELETE_FAIL:
				return "\"" + errSubject + "\" cannot be deleted! Filesystem response: " + externalErrString;
			case BOSS_ERROR_CURL_INIT_FAIL:
				return "cURL cannot be initialised!";
			case BOSS_ERROR_CURL_SET_ERRBUFF_FAIL:
				return "cURL's error buffer could not be set! cURL response: " + externalErrString;
			case BOSS_ERROR_CURL_SET_OPTION_FAIL:
				return "A cURL option could not be set! cURL response: " + externalErrString;
			case BOSS_ERROR_CURL_SET_PROXY_FAIL:
				return "Proxy hostname or port invalid! cURL response: " + externalErrString;
			case BOSS_ERROR_CURL_SET_PROXY_TYPE_FAIL:
				return "Proxy type invalid! cURL response: " + externalErrString;
			case BOSS_ERROR_CURL_PERFORM_FAIL:
				return "cURL could not perform task! cURL response: " + externalErrString;
			case BOSS_ERROR_CURL_USER_CANCEL:
				return "Cancelled by user.";
			default:
				return "No error.";
			}
		}
	private:
		bossErrCode errCode;
		string externalErrString;
		string errSubject;
	};
	
}
#endif