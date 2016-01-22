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

	$Revision: 3184 $, $Date: 2011-08-26 20:52:13 +0100 (Fri, 26 Aug 2011) $
*/

#include "api/boss.h"

#include <clocale>
#include <ctime>

#include <iostream>
#include <locale>
#include <map>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/detail/utf8_codecvt_facet.hpp>

#include "base/fstream.h"
#include "common/conditional_data.h"
#include "common/error.h"
#include "common/game.h"
#include "common/globals.h"
#include "common/item_list.h"
#include "common/keywords.h"
#include "common/rule_line.h"
#include "support/helpers.h"
#include "support/logger.h"
#include "updating/updater.h"

////////////////////////
// Types (Internal)
////////////////////////

// Version string.
static const std::string boss_version = boss::IntToString(boss::BOSS_VERSION_MAJOR) + "." + boss::IntToString(boss::BOSS_VERSION_MINOR) + "." + boss::IntToString(boss::BOSS_VERSION_PATCH);

// Last error details buffer.
std::string lastErrorDetails = "";

// Error buffer and version string output pointers (for memory management).
uint8_t *extErrorPointer = NULL;
uint8_t *extVersionPointer = NULL;

// Database structure.
struct _boss_db_int {
	// Internal data storage.
	boss::Game game;
	boss::ItemList rawMasterlist;
	boss::ItemList loadOrder;
	boss::ItemList activePlugins;
	std::time_t activePluginsMTime;              // Holds the modification date of plugins.txt.
	std::time_t loadOrderMTime;                  // For timestamp-based system, holds modification date of loadorder.txt, otherwise it holds the modification date of the Data folder.
	std::map<uint32_t, std::string> bashTagMap;  // A hashmap containing all the Bash Tag strings found in the masterlist and userlist and their unique IDs.
	                                             // Ordered to make ensuring UIDs easy (check the UID of the last element then increment). Strings are case-preserved.

	// Externally-visible data pointers storage.
	BashTag *extTagMap;                          // Holds the pointer for the bashTagMap returned by GetBashTagMap().
	uint32_t *extAddedTagIds;
	uint32_t *extRemovedTagIds;
	uint8_t *extString;
	uint8_t **extStringArray;
	uint8_t **extStringArray2;
	BossMessage *extMessageArray;

	// Pointer array sizes.
	size_t extStringArraySize;
	size_t extStringArray2Size;
	size_t extMessageArraySize;

	// Constructor
	_boss_db_int()
	    : extTagMap(NULL),
	      extAddedTagIds(NULL),
	      extRemovedTagIds(NULL),
	      extString(NULL),
	      extStringArray(NULL),
	      extStringArray2(NULL),
	      extMessageArray(NULL),
	      extStringArraySize(0),
	      extStringArray2Size(0),
	      extMessageArraySize(0),
	      activePluginsMTime(0),
	      loadOrderMTime(0) {}

	~_boss_db_int() {
		delete[] extAddedTagIds;
		delete[] extRemovedTagIds;
		delete[] extString;

		if (extTagMap != NULL) {
			for (size_t i = 0; i < bashTagMap.size(); i++)
				delete[] extTagMap[i].name;  // Gotta clear those allocated strings.
			delete[] extTagMap;
		}

		if (extMessageArray != NULL) {
			for (size_t i = 0; i < extMessageArraySize; i++)
				delete[] extMessageArray[i].message;  // Gotta clear those allocated strings.
			delete[] extMessageArray;
		}

		if (extStringArray != NULL) {
			for (size_t i = 0; i < extStringArraySize; i++)
				delete[] extStringArray[i];  // Clear all the uint8_t strings created.
			delete[] extStringArray;  // Clear the string array.
		}

		if (extStringArray2 != NULL) {
			for (size_t i = 0; i < extStringArray2Size; i++)
				delete[] extStringArray2[i];  // Clear all the uint8_t strings created.
			delete[] extStringArray2;  // Clear the string array.
		}
	}

	// Get a Bash Tag's string name from its UID.
	std::string GetTagString(uint32_t uid) {
		std::map<uint32_t, std::string>::iterator mapPos = bashTagMap.find(uid);
		if (mapPos != bashTagMap.end())
			return mapPos->second;
		else
			return "";
	}

	// Get a Bash Tag's position in the bashTagMap from its string name.
	std::map<uint32_t, std::string>::iterator FindBashTag(std::string value) {
		std::map<uint32_t, std::string>::iterator mapPos = bashTagMap.begin();
		while (mapPos != bashTagMap.end()) {
			if (mapPos->second == value)
				break;
			++mapPos;
		}
		return mapPos;
	}
};

// The following are the possible codes that the API can return.
// Taken from common/error.h and extended.
BOSS_API const uint32_t BOSS_API_OK                           = boss::BOSS_OK;
BOSS_API const uint32_t BOSS_API_OK_NO_UPDATE_NECESSARY       = boss::BOSS_OK_NO_UPDATE_NECESSARY;
BOSS_API const uint32_t BOSS_API_WARN_BAD_FILENAME            = boss::BOSS_ERROR_ENCODING_CONVERSION_FAIL;
BOSS_API const uint32_t BOSS_API_WARN_LO_MISMATCH             = boss::BOSS_ERROR_LO_MISMATCH;
BOSS_API const uint32_t BOSS_API_ERROR_FILE_WRITE_FAIL        = boss::BOSS_ERROR_FILE_WRITE_FAIL;
BOSS_API const uint32_t BOSS_API_ERROR_FILE_DELETE_FAIL       = boss::BOSS_ERROR_FS_FILE_DELETE_FAIL;
BOSS_API const uint32_t BOSS_API_ERROR_FILE_NOT_UTF8          = boss::BOSS_ERROR_FILE_NOT_UTF8;
BOSS_API const uint32_t BOSS_API_ERROR_FILE_NOT_FOUND         = boss::BOSS_ERROR_FILE_NOT_FOUND;
BOSS_API const uint32_t BOSS_API_ERROR_FILE_RENAME_FAIL       = boss::BOSS_ERROR_FS_FILE_RENAME_FAIL;
BOSS_API const uint32_t BOSS_API_ERROR_TIMESTAMP_READ_FAIL    = boss::BOSS_ERROR_FS_FILE_MOD_TIME_READ_FAIL;
BOSS_API const uint32_t BOSS_API_ERROR_TIMESTAMP_WRITE_FAIL   = boss::BOSS_ERROR_FS_FILE_MOD_TIME_WRITE_FAIL;
BOSS_API const uint32_t BOSS_API_ERROR_CONDITION_EVAL_FAIL    = boss::BOSS_ERROR_CONDITION_EVAL_FAIL;
BOSS_API const uint32_t BOSS_API_ERROR_REGEX_EVAL_FAIL        = boss::BOSS_ERROR_REGEX_EVAL_FAIL;
BOSS_API const uint32_t BOSS_API_ERROR_PARSE_FAIL             = boss::BOSS_ERROR_FILE_PARSE_FAIL;
BOSS_API const uint32_t BOSS_API_ERROR_NO_MEM                 = boss::BOSS_ERROR_NO_MEM;
BOSS_API const uint32_t BOSS_API_ERROR_INVALID_ARGS           = boss::BOSS_ERROR_INVALID_ARGS;
BOSS_API const uint32_t BOSS_API_ERROR_NETWORK_FAIL           = boss::BOSS_ERROR_NETWORK_FAIL;
BOSS_API const uint32_t BOSS_API_ERROR_NO_INTERNET_CONNECTION = boss::BOSS_ERROR_NO_INTERNET_CONNECTION;
BOSS_API const uint32_t BOSS_API_ERROR_NO_TAG_MAP             = boss::BOSS_ERROR_NO_TAG_MAP;
BOSS_API const uint32_t BOSS_API_ERROR_PLUGINS_FULL           = boss::BOSS_ERROR_PLUGINS_FULL;
BOSS_API const uint32_t BOSS_API_ERROR_GAME_NOT_FOUND         = boss::BOSS_ERROR_NO_GAME_DETECTED;
BOSS_API const uint32_t BOSS_API_ERROR_PLUGIN_BEFORE_MASTER   = boss::BOSS_ERROR_PLUGIN_BEFORE_MASTER;
BOSS_API const uint32_t BOSS_API_ERROR_INVALID_SYNTAX         = boss::BOSS_ERROR_INVALID_SYNTAX
BOSS_API const uint32_t BOSS_API_RETURN_MAX                   = boss::BOSS_ERROR_MAX;
BOSS_API const uint32_t BOSS_API_ERROR_GIT_ERROR              = boss::BOSS_ERROR_GIT_ERROR;

// The following are the mod cleanliness states that the API can return.
BOSS_API const uint32_t BOSS_API_CLEAN_NO      = 0;
BOSS_API const uint32_t BOSS_API_CLEAN_YES     = 1;
BOSS_API const uint32_t BOSS_API_CLEAN_UNKNOWN = 2;

// The following are for signifying what load order method is being used:
BOSS_API const uint32_t BOSS_API_LOMETHOD_TIMESTAMP = boss::LOMETHOD_TIMESTAMP;
BOSS_API const uint32_t BOSS_API_LOMETHOD_TEXTFILE  = boss::LOMETHOD_TEXTFILE;

// The following are the games identifiers used by the API.
BOSS_API const uint32_t BOSS_API_GAME_OBLIVION  = boss::OBLIVION;
BOSS_API const uint32_t BOSS_API_GAME_FALLOUT3  = boss::FALLOUT3;
BOSS_API const uint32_t BOSS_API_GAME_FALLOUTNV = boss::FALLOUTNV;
BOSS_API const uint32_t BOSS_API_GAME_NEHRIM    = boss::NEHRIM;
BOSS_API const uint32_t BOSS_API_GAME_SKYRIM    = boss::SKYRIM;
//BOSS_API const uint32_t BOSS_API_GAME_MORROWIND = boss::MORROWIND;

// BOSS message types.
BOSS_API extern const uint32_t BOSS_API_MESSAGE_SAY             = boss::SAY;
BOSS_API extern const uint32_t BOSS_API_MESSAGE_TAG             = boss::TAG;
BOSS_API extern const uint32_t BOSS_API_MESSAGE_REQUIREMENT     = boss::REQ;
BOSS_API extern const uint32_t BOSS_API_MESSAGE_INCOMPATIBILITY = boss::INC;
BOSS_API extern const uint32_t BOSS_API_MESSAGE_DIRTY           = boss::DIRTY;
BOSS_API extern const uint32_t BOSS_API_MESSAGE_WARN            = boss::WARN;
BOSS_API extern const uint32_t BOSS_API_MESSAGE_ERROR           = boss::ERR;


//////////////////////////////
// Internal Functions
//////////////////////////////

void GetBashTagsFromString(const std::string message,
                           std::unordered_set<std::string> &tagsAdded,
                           std::unordered_set<std::string> &tagsRemoved) {
	// Need to collect Bash Tags. Search for the Bash Tag listing syntaxes.
	size_t pos1, pos2 = std::string::npos;
	std::string addedList, removedList;
	pos1 = message.find("{{BASH:");
	if (pos1 != std::string::npos)
		pos2 = message.find("}}", pos1);
	if (pos2 != std::string::npos)
		addedList = message.substr(pos1 + 7, pos2 - pos1 - 7);

	if (!addedList.empty()) {
		// Now we move through the list of added Tags.
		// Search the set of already added Tags for each one and only add if it's not found.
		std::string name;
		pos1 = 0;
		pos2 = addedList.find(",", pos1);
		while (pos2 != std::string::npos) {
			name = boost::trim_copy(addedList.substr(pos1, pos2 - pos1));
			if (tagsAdded.find(name) == tagsAdded.end())
				tagsAdded.insert(name);
			pos1 = pos2 + 1;
			pos2 = addedList.find(",", pos1);
		}
		name = trim_copy(addedList.substr(pos1));
		if (tagsAdded.find(name) == tagsAdded.end())
			tagsAdded.insert(name);
	}

	pos1 = message.find("[");
	pos2 = std::string::npos;
	if (pos1 != std::string::npos)
		pos2 = message.find("]", pos1);
	if (pos2 != std::string::npos)
		removedList = message.substr(pos1 + 1, pos2 - pos1 - 1);

	if (!removedList.empty()) {
		std::string name;
		pos1 = 0;
		pos2 = removedList.find(",", pos1);
		while (pos2 != std::string::npos) {
			name = boost::trim_copy(removedList.substr(pos1, pos2 - pos1));
			if (tagsRemoved.find(name) == tagsRemoved.end())
				tagsRemoved.insert(name);
			pos1 = pos2 + 1;
			pos2 = removedList.find(",", pos1);
		}
		name = boost::trim_copy(removedList.substr(pos1));
		if (tagsRemoved.find(name) == tagsRemoved.end())
			tagsRemoved.insert(name);
	}
}

// This allocates memory - don't forget to free it later.
uint8_t * StringToUint8_tString(std::string str) {
	size_t length = str.length() + 1;
	uint8_t *p = new uint8_t[length];

	for (size_t j = 0; j < str.length(); j++) {  // UTF-8, so this is code-point by code-point rather than char by char, but same result here.
		p[j] = str[j];
	}
	p[length - 1] = '\0';
	return p;
}

uint32_t ReturnCode(uint32_t returnCode, std::string details) {
	lastErrorDetails = details;
	return returnCode;
}

uint32_t ReturnCode(uint32_t returnCode) {
	lastErrorDetails = "";
	return returnCode;
}

uint32_t ReturnCode(boss::boss_error e) {
	lastErrorDetails = e.getString();
	return e.getCode();
}

// MCP Note: The updater class was part of the old cURL stuff that was removed in commit 273d4edc. Need to look into sorting this out to work with the new Git updater.
class APIMlistUpdater : public boss::MasterlistUpdater {
 protected:
	int progress(Updater *updater, double dlFraction, double dlTotal) {
		return 0;
	}
};

std::time_t GetLoadOrderMTime(const boss::Game& game) {
	try {
		if (game.GetLoadOrderMethod() == boss::LOMETHOD_TEXTFILE && boost::filesystem::exists(game.LoadOrderFile())) {
			// Load order is stored in game.LoadOrderFile(), but load order must also be reloaded if game.DataFolder() has been altered.
			std::time_t t1 = boost::filesystem::last_write_time(game.LoadOrderFile());
			std::time_t t2 = boost::filesystem::last_write_time(game.DataFolder());
			if (t1 > t2)  // Return later time.
				return t1;
			else
				return t2;
		} else {
			return boost::filesystem::last_write_time(game.DataFolder());
		}
	} catch(boost::filesystem::filesystem_error e) {
		throw boss::boss_error(boss::BOSS_ERROR_FS_FILE_MOD_TIME_READ_FAIL, game.DataFolder().string(), e.what());
	}
}

// Not really necessary, but it means we can handle the two mtime checks similarly.
std::time_t GetActivePluginsMTime(const boss::Game& game) {
	try {
		if (boost::filesystem::exists(game.ActivePluginsFile()))
			return boost::filesystem::last_write_time(game.ActivePluginsFile());
		else
			return 0;
	} catch(boost::filesystem::filesystem_error e) {
		throw boss::boss_error(boss::BOSS_ERROR_FS_FILE_MOD_TIME_READ_FAIL, game.ActivePluginsFile().string(), e.what());
	}
}


//////////////////////////////
// Error Handling Functions
//////////////////////////////

// Outputs a string giving the details of the last time an error or
// warning return code was returned by a function.
BOSS_API uint32_t GetLastErrorDetails(uint8_t **details) {
	if (details == NULL)  // Check for valid args.
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	delete[] extErrorPointer;
	extErrorPointer = NULL;

	try {
		extErrorPointer = StringToUint8_tString(lastErrorDetails);
	} catch (std::bad_alloc /*&e*/) {
		return ReturnCode(boss::boss_error(boss::BOSS_ERROR_NO_MEM));
	}
	*details = extErrorPointer;
	return ReturnCode(BOSS_API_OK);
}


//////////////////////////////
// Version Functions
//////////////////////////////

// Returns whether this version of BOSS supports the API from the given
// BOSS version. Abstracts BOSS API stability policy away from clients.
BOSS_API bool IsCompatibleVersion(const uint32_t bossVersionMajor,
                                  const uint32_t bossVersionMinor,
                                  const uint32_t bossVersionPatch) {
	if (bossVersionMajor < 2)  // The 1.9 API was different.
		return false;
	else if (bossVersionMajor == 2 && bossVersionMinor < 1)  // The 2.0 API was different.
		return false;
	if (bossVersionMajor <= boss::BOSS_VERSION_MAJOR && bossVersionMinor <= boss::BOSS_VERSION_MINOR)
		return true;
	else
		return false;
}

// Returns the version string for this version of BOSS.
// The string exists for the lifetime of the library.
BOSS_API uint32_t GetVersionString(uint8_t **bossVersionStr) {
	if (bossVersionStr == NULL)  // Check for valid args.
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	delete[] extVersionPointer;
	extVersionPointer = NULL;

	try {
		extVersionPointer = StringToUint8_tString(boss_version);
	} catch (std::bad_alloc /*&e*/) {
		return ReturnCode(boss::boss_error(boss::BOSS_ERROR_NO_MEM));
	}
	*bossVersionStr = extVersionPointer;
	return ReturnCode(BOSS_API_OK);
}


////////////////////////////////////
// Lifecycle Management Functions
////////////////////////////////////

// Explicitly manage database lifetime. Allows clients to free memory when
// they want/need to. clientGame sets the game the DB is for, and dataPath
// is the path to that game's Data folder. This function also checks that
// plugins.txt and loadorder.txt (if they both exist) are in sync.
BOSS_API uint32_t CreateBossDb(boss_db *db, const uint32_t clientGame,
                                            const uint8_t *gamePath) {
	if (db == NULL)  // Check for valid args.
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");
	else if (clientGame != boss::OBLIVION && clientGame != boss::FALLOUT3 &&
	         clientGame != boss::FALLOUTNV && clientGame != boss::NEHRIM &&
	         clientGame != boss::SKYRIM /*&& clientGame != boss::MORROWIND*/)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Invalid game specified.");

	// Set the locale to get encoding conversions working correctly.
	std::setlocale(LC_CTYPE, "");
	std::locale global_loc = std::locale();  // MCP Note: Which locale is this?
	std::locale loc(global_loc, new boost::filesystem::detail::utf8_codecvt_facet());
	boost::filesystem::path::imbue(loc);

	// Set game. Because this is a global and there may be multiple DBs for different games,
	// each time a DB's function is called, it should be reset.
	// If dataPath is null, then we need to look for the specified game.
	std::string game_path = "";
	if (gamePath != NULL)
		game_path = std::string(reinterpret_cast<const char *>(gamePath));

	boss::Game game;
	try {
		game = boss::Game(clientGame, game_path);  // This also checks to see if the game is installed if game_path is empty and throws an exception if it is not detected.
	} catch (boss::boss_error& e) {
		return ReturnCode(e);
	}

	// Now check if plugins.txt and loadorder.txt are in sync.
	uint32_t crc1 = 0, crc2 = 0;
	if (boost::filesystem::exists(game.ActivePluginsFile()) &&
	    boost::filesystem::exists(game.LoadOrderFile())) {
		// Load loadorder.txt and save a temporary filtered version.
		boss::ItemList loadorder;
		try {
			loadorder.Load(game, game.DataFolder());
			loadorder.SavePluginNames(game, game.LoadOrderFile().string() + ".new", true, true);
		} catch (boss::boss_error &e) {
			return ReturnCode(e);
		}

		crc1 = boss::GetCrc32(game.ActivePluginsFile());
		crc2 = boss::GetCrc32(game.LoadOrderFile().string() + ".new");

		// Now delete temporary filtered loadorder.txt.
		try {
			boost::filesystem::remove(game.LoadOrderFile().string() + ".new");
		} catch (boost::filesystem::filesystem_error e) {
			return ReturnCode(boss::boss_error(boss::BOSS_ERROR_FS_FILE_DELETE_FAIL, game.LoadOrderFile().string() + ".new", e.what()));
		}
	}

	boss_db retVal;
	try {
		retVal = new _boss_db_int;
	} catch (std::bad_alloc /*&e*/) {
		return ReturnCode(boss::boss_error(boss::BOSS_ERROR_NO_MEM));
	}
	retVal->game = game;
	*db = retVal;

	// Since plugins.txt is derived from loadorder.txt in the same manner as the temporary file created above,
	// with the derivation occurring whenever loadorder.txt is changed, if plugins.txt has not been changed
	// by something other than the API (eg. the launcher), then the CRCs will match. Otherwise they will differ.
	if (crc1 != crc2)
		return ReturnCode(BOSS_API_WARN_LO_MISMATCH);
	else
		return ReturnCode(BOSS_API_OK);
}

BOSS_API void DestroyBossDb(boss_db db) {
	delete db;  // Delete DB. Destructor handles memory deallocation.
}

BOSS_API void CleanUpAPI() {
	delete[] extErrorPointer;
	delete[] extVersionPointer;
	extErrorPointer = NULL;
	extVersionPointer = NULL;
}


///////////////////////////////////
// Database Loading Functions
///////////////////////////////////

// Loads the masterlist and userlist from the paths specified.
// Can be called multiple times. On error, the database is unchanged.
// Paths are case-sensitive if the underlying filesystem is case-sensitive.

// Masterlist and userlist loading are internally independent, but occur in
// same function for ease-of-use by clients.
BOSS_API uint32_t Load(boss_db db, const uint8_t *masterlistPath,
                                   const uint8_t *userlistPath) {
	std::ItemList masterlist;
	std::RuleList userlist;

	// Check for valid args.
	if (db == NULL || masterlistPath == NULL)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	// PATH SETTING
	boost::filesystem::path masterlist_path = boost::filesystem::path(reinterpret_cast<const char *>(masterlistPath));
	if (masterlist_path.empty())
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Masterlist path is empty.");
	boost::filesystem::path userlist_path;
	if (userlistPath != NULL) {
		userlist_path = boost::filesystem::path(reinterpret_cast<const char *>(userlistPath));
		if (userlist_path.empty())
			return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Userlist path is empty.");
	}

	// Parse masterlist and userlist.
	try {
		masterlist.Load(db->game, masterlist_path);
		if (userlistPath != NULL)
			userlist.Load(db->game, userlist_path);
	} catch (boss::boss_error &e) {
		return ReturnCode(e);  // BOSS_ERRORs map directly to BOSS_API_ERRORs.
	}

	// FREE CURRENT POINTERS
	// Free memory at pointers stored in structure.
	delete[] db->extAddedTagIds;
	delete[] db->extRemovedTagIds;
	delete[] db->extString;

	db->extAddedTagIds = NULL;
	db->extRemovedTagIds = NULL;
	db->extString = NULL;

	if (db->extTagMap != NULL) {
		for (size_t i = 0; i < db->bashTagMap.size(); i++)
			delete[] db->extTagMap[i].name;  // Gotta clear those allocated strings.
		delete[] db->extTagMap;
		db->extTagMap = NULL;
		db->bashTagMap.clear();
	}

	if (db->extMessageArray != NULL) {
		for (size_t i = 0; i < db->extMessageArraySize; i++)
			delete[] db->extMessageArray[i].message;  // Gotta clear those allocated strings.
		delete[] db->extMessageArray;
		db->extMessageArray = NULL;
		db->extMessageArraySize = 0;
	}

	if (db->extStringArray != NULL) {
		for (size_t i = 0; i < db->extStringArraySize; i++)
			delete[] db->extStringArray[i];  // Clear all the uint8_t strings created.
		delete[] db->extStringArray;  // Clear the string array.
		db->extStringArray = NULL;
		db->extStringArraySize = 0;
	}

	if (db->extStringArray2 != NULL) {
		for (size_t i = 0; i < db->extStringArray2Size; i++)
			delete[] db->extStringArray2[i];  // Clear all the uint8_t strings created.
		delete[] db->extStringArray2;  // Clear the string array.
		db->extStringArray2 = NULL;
		db->extStringArray2Size = 0;
	}

	// DB SET
	db->rawMasterlist = masterlist;
	db->game.masterlist = masterlist;  // Not actually filtered, but retrival functions assume filtered masterlist is populated.
	db->game.userlist = userlist;
	db->bashTagMap.clear();
	return ReturnCode(BOSS_API_OK);
}

// Re-evaluates all conditional lines and regex mods in rawMasterlist,
// putting the output into filteredMasterlist.
BOSS_API uint32_t EvalConditionals(boss_db db) {
	// Check for valid args.
	if (db == NULL)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	boss::ItemList masterlist = db->rawMasterlist;
	try {
		masterlist.EvalConditions(db->game);  // First evaluate conditionals.
		masterlist.EvalRegex(db->game);       // Now evaluate regex.
	} catch (boss::boss_error &e) {
		return ReturnCode(e);  // BOSS_ERRORs map directly to BOSS_API_ERRORs.
	}

	// Now set DB ItemList to function's ItemList.
	db->game.masterlist = masterlist;
	return ReturnCode(BOSS_API_OK);
}


//////////////////////////////////
// Network Functions
//////////////////////////////////

// MCP Note: These are probably no longer valid now that cURL is no longer in use so we'll probably need to sift through this and see what we can keep and what can be discarded

// Sets the proxy settings for BAPI globally, so that all subsequent BAPI network
// function calls are affected until SetProxy is called again or BAPI is unloaded.
uint32_t SetProxy(const uint8_t *hostname, const uint32_t port,
                  const uint8_t *username, const uint8_t *password) {
	if (hostname == NULL && (port != 0 || username != NULL || password != NULL))
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Hostname cannot be null if port is non-zero or username or password are non-null.");
	else if (username == NULL && (port != 0 || hostname != NULL || password != NULL))
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Username cannot be null if port is non-zero or hostname or password are non-null.");
	else if (password == NULL && (port != 0 || username != NULL || hostname != NULL))
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Password cannot be null if port is non-zero or username or hostname are non-null.");
	else if (port == 0 && (hostname != NULL || username != NULL || password != NULL))
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Port cannot be non-zero if hostname or username or password are non-null.");

	boss::gl_proxy_host = std::string(reinterpret_cast<const char *>(hostname));
	boss::gl_proxy_user = std::string(reinterpret_cast<const char *>(username));
	boss::gl_proxy_passwd = std::string(reinterpret_cast<const char *>(password));
	boss::gl_proxy_port = port;

	return ReturnCode(BOSS_API_OK);
}

// MCP Note: This should be changed to use the new updater.

// Checks if there is a masterlist at masterlistPath. If not,
// it downloads the latest masterlist for the given game to masterlistPath.
// If there is, it first compares online and local versions to see if an
// update is necessary.
BOSS_API uint32_t UpdateMasterlist(boss_db db, const uint8_t *masterlistPath) {
	if (db == NULL || masterlistPath == NULL)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	// PATH SETTING
	boost::filesystem::path masterlist_path = boost::filesystem::path(reinterpret_cast<const char *>(masterlistPath));

	if (masterlist_path.empty())
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Masterlist path is empty.");

	APIMlistUpdater mUpdater;
	try {
		if (!mUpdater.IsInternetReachable()) {
			return ReturnCode(boss::boss_error(BOSS_API_ERROR_NO_INTERNET_CONNECTION));
		} else {
			try {
				std::string localDate, remoteDate;
				uint32_t localRevision, remoteRevision;
				mUpdater.Update(db->game, masterlist_path, localRevision, localDate, remoteRevision, remoteDate);
				if (localRevision == remoteRevision)
					return ReturnCode(BOSS_API_OK_NO_UPDATE_NECESSARY);
				else
					return ReturnCode(BOSS_API_OK);
			} catch (boss::boss_error &e) {
				return ReturnCode(BOSS_API_ERROR_NETWORK_FAIL, e.getString());
			}
		}
	} catch (boss::boss_error &e) {
		return ReturnCode(BOSS_API_ERROR_NETWORK_FAIL, e.getString());
	}
}

// MCP Note: This should be changed to use the plugin submitter code or removed, as the submitter is part of the BOSS log (right?).

// Submits the given plugin as unrecognised to BOSS's unrecognised plugin tracker,
// using the same method as the BOSS Log plugin submitter. Whether or not the plugin
// is actually unrecognised is not checked, but recognised plugin submissions will be
// ignored and slow down the addition of unrecognised plugin that are submitted, so
// recognised plugins should not be submitted. Either link or info can be NULL, but
// not both. If link is NULL, load order suggestions and detail on what the plugin does
// is crucial for addition to the masterlist.
BOSS_API uint32_t SubmitUnrecognisedPlugin(boss_db db,
                                           const uint8_t *plugin,
                                           const uint8_t *link,
                                           const uint8_t *info) {
	if (db == NULL || plugin == NULL)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed for db or plugin.");
	else if (link == NULL && info == NULL)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "link and info cannot both be null.");

	std::string pluginStr = std::string(reinterpret_cast<const char *>(plugin));

	std::string linkStr, infoStr;
	if (link != NULL)
		linkStr = std::string(reinterpret_cast<const char *>(link));
	if (info != NULL)
		infoStr = std::string(reinterpret_cast<const char *>(info));

	if (linkStr.empty() && infoStr.empty())
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "link and info cannot both be empty strings.");

	std::string description;
	if (linkStr.empty())
		description = infoStr;
	else
		description = linkStr + "\\n\\n" + infoStr;
	std::string id;
	char *url = "http://bugzilla.darkcreations.org/jsonrpc.cgi";  // URL to send data to.

	// Now start curling.
	char errbuff[CURL_ERROR_SIZE];
	CURL *curl;  // cURL handle
	CURLcode ret;
	std::string buffer, JSON;

	try {
		curl = InitCurl(errbuff);  // Init curl.

		// Now set up the common curl options.
		curl_easy_setopt(curl, CURLOPT_URL, url);
		ret = curl_easy_setopt(curl, CURLOPT_POST, 1);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writer);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
		if (ret != CURLE_OK) {
			std::string err = errbuff;
			curl_easy_cleanup(curl);
			throw boss::boss_error(err, boss::BOSS_ERROR_CURL_SET_OPTION_FAIL);
		}
		// Need a structure containing the HTTP headers for curl to use.
		struct curl_slist *slist = NULL;
		slist = curl_slist_append(slist, "origin:null");
		if (slist == NULL) {
			curl_easy_cleanup(curl);
			throw boss::boss_error("Could not set HTTP headers.", boss::BOSS_ERROR_CURL_SET_OPTION_FAIL);
		}
		slist = curl_slist_append(slist, "content-type:application/json");
		if (slist == NULL) {
			curl_slist_free_all(slist);
			curl_easy_cleanup(curl);
			throw boss::boss_error("Could not set HTTP headers.", boss::BOSS_ERROR_CURL_SET_OPTION_FAIL);
		}
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
		if (ret != CURLE_OK) {
			std::string err = errbuff;
			curl_slist_free_all(slist);
			curl_easy_cleanup(curl);
			throw boss::boss_error(err, boss::BOSS_ERROR_CURL_SET_OPTION_FAIL);
		}

		// There are two stages to submission, with three possible requests.
		// The first is a bug search, so do that now.
		// Create JSON search data object.
		JSON = "{\"method\":\"Bug.search\",\"params\":[{\"Bugzilla_login\":\"bossguest@darkcreations.org\",\"Bugzilla_password\":\"bosspassword\",\"product\":\"BOSS\",\"component\":\"" + db->game.Name() + "\",\"summary\":\"";
		JSON += pluginStr + "\"}],\"id\":1}";

		for (uint32_t i = 0; i < 2; i++) {
			// Set POST data.
			ret = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, JSON.c_str());
			if (ret != CURLE_OK) {
				std::string err = errbuff;
				curl_slist_free_all(slist);
				curl_easy_cleanup(curl);
				throw boss::boss_error(err, boss::BOSS_ERROR_CURL_SET_OPTION_FAIL);
			}
			// Perform POST request.
			ret = curl_easy_perform(curl);
			if (ret != CURLE_OK) {
				std::string err = errbuff;
				curl_slist_free_all(slist);
				curl_easy_cleanup(curl);
				throw boss::boss_error(err, boss::BOSS_ERROR_CURL_PERFORM_FAIL);
			}
			// Check result.
			long int code;
			ret = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
			if (ret != CURLE_OK) {
				std::string err = errbuff;
				curl_slist_free_all(slist);
				curl_easy_cleanup(curl);
				throw boss::boss_error(err, std::BOSS_ERROR_CURL_PERFORM_FAIL);
			}
			if (code != 200) {
				curl_slist_free_all(slist);
				curl_easy_cleanup(curl);
				throw boss::boss_error("Server responded with code " + boss::IntToString(code) + ".", boss::BOSS_ERROR_CURL_PERFORM_FAIL);
			}

			// Now check returned data for errors and a returned bug ID.
			size_t pos1, pos2;
			pos1 = buffer.find("\"error\":");
			if (pos1 == std::string::npos) {
				curl_slist_free_all(slist);
				curl_easy_cleanup(curl);
				return ReturnCode(BOSS_API_ERROR_NETWORK_FAIL, "Could not parse return data.");
			}
			pos2 = buffer.find(',', pos1);
			if (pos2 == std::string::npos) {
				curl_slist_free_all(slist);
				curl_easy_cleanup(curl);
				return ReturnCode(BOSS_API_ERROR_NETWORK_FAIL, "Could not parse return data.");
			}
			if (buffer.substr(pos1 + 8, pos2 - pos1 - 8) != "null") {
				curl_slist_free_all(slist);
				curl_easy_cleanup(curl);
				return ReturnCode(BOSS_API_ERROR_NETWORK_FAIL, "Bugzilla API call failed.");
			}

			if (i == 0) {  // First loop is a bug search, so try extracting an ID (if there is one).
				pos1 = buffer.find("\"groups\"");  // There are 4 "id" strings in the returned JSON. The one we want comes after "groups".
				if (pos1 != std::string::npos) {  // Bug found. Extract id.
					pos1 = buffer.find("\"id\":", pos1);
					if (pos1 == std::string::npos) {
						curl_slist_free_all(slist);
						curl_easy_cleanup(curl);
						return ReturnCode(BOSS_API_ERROR_NETWORK_FAIL, "Could not parse return data.");
					}
					pos2 = buffer.find(',', pos1);
					if (pos2 == std::string::npos) {
						curl_slist_free_all(slist);
						curl_easy_cleanup(curl);
						return ReturnCode(BOSS_API_ERROR_NETWORK_FAIL, "Could not parse return data.");
					}
					id = buffer.substr(pos1+5, pos2-pos1-5);
				}
				// Now set up request for the second loop.
				if (id.empty()) {  // Create a new bug.
					// Create JSON bug creation data object.
					JSON = "{\"method\":\"Bug.create\",\"params\":[{\"Bugzilla_login\":\"bossguest@darkcreations.org\",\"Bugzilla_password\":\"bosspassword\",\"product\":\"BOSS\",\"component\":\"" + db->game.Name() + "\",\"summary\":\"";
					JSON += pluginStr + "\",\"version\":\"2.1\",\"description\":\"" + description + "\",\"op_sys\":\"Windows\",\"platform\":\"PC\",\"priority\":\"---\",\"severity\":\"enhancement\"}],\"id\":3}";
				} else {  // Adda a comment to the existing bug.
					JSON = "{\"method\":\"Bug.add_comment\",\"params\":[{\"Bugzilla_login\":\"bossguest@darkcreations.org\",\"Bugzilla_password\":\"bosspassword\",\"id\":";
					JSON += id + ",\"comment\":" + description + "}],\"id\":2}";
				}
			}
		}

		curl_slist_free_all(slist);
		curl_easy_cleanup(curl);
	} catch (boss::boss_error &e) {
		return ReturnCode(BOSS_API_ERROR_NETWORK_FAIL, e.getString());
	}

	return ReturnCode(BOSS_API_OK);
}

////////////////////////////////
// Plugin Sorting Functions
////////////////////////////////

// Returns which method BOSS is using for the load order.
BOSS_API uint32_t GetLoadOrderMethod(boss_db db, uint32_t *method) {
	if (db == NULL || method == NULL)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	*method = db->game.GetLoadOrderMethod();

	return ReturnCode(BOSS_API_OK);
}

// Sorts the mods in the data path, using the masterlist at the masterlist path,
// specified when the db was loaded using Load. Outputs a list of plugins, pointed to
// by sortedPlugins, of length pointed to by listLength. lastRecPos points to the
// position in the sortedPlugins list of the last plugin recognised by BOSS.
// If the trialOnly parameter is true, no plugins are actually redated.
// If trialOnly is false, then sortedPlugins, listLength and lastRecPos can be null
// pointers, in case you do not require the information. If one of them is null, the
// other two must also be null.
BOSS_API uint32_t SortMods(boss_db db,
                           const bool trialOnly,
                           uint8_t ***sortedPlugins,
                           size_t *sortedListLength,
                           uint8_t ***unrecognisedPlugins,
                           size_t *unrecListLength) {
	if (db == NULL ||
	   (trialOnly && (sortedPlugins == NULL ||
	                  sortedListLength == NULL ||
	                  unrecognisedPlugins == NULL ||
	                  unrecListLength == NULL)))
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	// Initialise vars.
	if (sortedPlugins != NULL)
		*sortedPlugins = NULL;
	if (unrecognisedPlugins != NULL)
		*unrecognisedPlugins = NULL;
	if (sortedListLength != NULL)
		*sortedListLength = 0;
	if (unrecListLength != NULL)
		*unrecListLength = 0;

	// Free memory if already used.
	if (db->extStringArray != NULL) {
		for (size_t i = 0; i < db->extStringArraySize; i++)
			delete[] db->extStringArray[i];  // Clear all the uint8_t strings created.
		delete[] db->extStringArray;  // Clear the string array.
		db->extStringArray = NULL;
		db->extStringArraySize = 0;
	}
	if (db->extStringArray2 != NULL) {
		for (size_t i = 0; i < db->extStringArray2Size; i++)
			delete[] db->extStringArray2[i];  // Clear all the uint8_t strings created.
		delete[] db->extStringArray2;  // Clear the string array.
		db->extStringArray2 = NULL;
		db->extStringArray2Size = 0;
	}

	// Set up working modlist.
	boss::gl_trial_run = trialOnly;
	std::vector<boss::Item> items, recognised, unrecognised;
	try {
		db->game.modlist.Load(db->game, db->game.DataFolder());
		db->game.masterlist.EvalConditions(db->game);  // In case it hasn't already been filtered.
		db->game.masterlist.EvalRegex(db->game);       // In case it hasn't already been filtered.
		db->game.ApplyMasterlist();
		db->game.ApplyUserlist();
		// Before the master partition is applied in SortPlugins(), record recognised and unrecognised plugins.
		items = db->game.modlist.Items();
		if (db->game.modlist.LastRecognisedPos() > 0) {
			recognised = std::vector<boss::Item>(items.begin(), items.begin() + db->game.modlist.LastRecognisedPos());
			unrecognised = std::vector<boss::Item>(items.begin() + db->game.modlist.LastRecognisedPos() + 1, items.end());
		}
		db->game.SortPlugins();
	} catch (boss::boss_error &e) {
		return ReturnCode(e);  // BOSS_ERRORs map directly to BOSS_API_ERRORs.
	}

	// Now create external arrays.
	db->extStringArraySize = recognised.size();
	db->extStringArray2Size = unrecognised.size();
	try {
		// All plugins.
		db->extStringArray = new uint8_t*[db->extStringArraySize];
		for (size_t i = 0; i < db->extStringArraySize; i++)
			db->extStringArray[i] = StringToUint8_tString(recognised[i].Name());
		// Unrecognised plugins.
		db->extStringArray2 = new uint8_t*[db->extStringArray2Size];
		for (size_t i = 0; i < db->extStringArray2Size; i++)
			db->extStringArray2[i] = StringToUint8_tString(unrecognised[i].Name());
	} catch (std::bad_alloc /*&e*/) {
		return ReturnCode(boss::boss_error(boss::BOSS_ERROR_NO_MEM));
	}

	// Set outputs.
	if (sortedPlugins != NULL)
		*sortedPlugins = db->extStringArray;
	if (sortedListLength != NULL)
		*sortedListLength = db->extStringArraySize;
	if (unrecognisedPlugins != NULL)
		*unrecognisedPlugins = db->extStringArray2;
	if (unrecListLength != NULL)
		*unrecListLength = db->extStringArray2Size;

	return ReturnCode(BOSS_API_OK);
}

// Gets a list of plugins in load order, with the number of plugins given by numPlugins.
BOSS_API uint32_t GetLoadOrder(boss_db db, uint8_t ***plugins,
                               size_t *numPlugins) {
	if (db == NULL || plugins == NULL || numPlugins == NULL)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	// Initialise vars.
	*numPlugins = 0;
	*plugins = NULL;

	// Free memory if already used.
	if (db->extStringArray != NULL) {
		for (size_t i = 0; i < db->extStringArraySize; i++)
			delete[] db->extStringArray[i];  // Clear all the uint8_t strings created.
		delete[] db->extStringArray;  // Clear the string array.
		db->extStringArray = NULL;
		db->extStringArraySize = 0;
	}

	try {
		if (GetLoadOrderMTime(db->game) != db->loadOrderMTime)
			db->loadOrder.Load(db->game, db->game.DataFolder());
	} catch (boss::boss_error &e) {
		return ReturnCode(e);  // BOSS_ERRORs map directly to BOSS_API_ERRORs.
	}

	// Need to throw out any repeats. Keep only the first instance of a plugin.
	std::unordered_set<std::string> hashset;
	std::vector<boss::Item> items = db->loadOrder.Items();
	std::vector<boss::Item>::iterator itemIter = items.begin();
	while (itemIter != items.end()) {
		// Check if plugin is in hashset. If not, add it.
		// If it is, remove it from the items vector.
		if (hashset.find(boost::to_lower_copy(itemIter->Name())) != hashset.end()) {  // Exists, remove it.
			itemIter = items.erase(itemIter);
		} else {
			hashset.insert(boost::to_lower_copy(itemIter->Name()));
			++itemIter;
		}
	}

	// Check array size. Exit if zero.
	if (items.empty())
		return ReturnCode(BOSS_API_OK);

	// Allocate memory.
	db->extStringArraySize = items.size();
	try {
		db->extStringArray = new uint8_t*[db->extStringArraySize];
		for (size_t i = 0; i < db->extStringArraySize; i++)
			db->extStringArray[i] = StringToUint8_tString(items[i].Name());
	} catch (std::bad_alloc /*&e*/) {
		return ReturnCode(boss::boss_error(boss::BOSS_ERROR_NO_MEM));
	}

	// Set outputs.
	*plugins = db->extStringArray;
	*numPlugins = db->extStringArraySize;

	return ReturnCode(BOSS_API_OK);
}

// Sets the load order to the given plugins list of length numPlugins.
BOSS_API uint32_t SetLoadOrder(boss_db db, uint8_t **plugins,
                               const size_t numPlugins) {
	if (db == NULL || plugins == NULL)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	// Check load order to see if it's valid.
	if (numPlugins > 0 && !boss::Item(std::string(reinterpret_cast<const char *>(plugins[0]))).IsGameMasterFile(db->game))
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Plugins may not be sorted before the game's master file.");

	// Create a vector to hold the new loadorder.
	db->loadOrder.Clear();
	// We need to loop through the plugin array given and enter each plugin into the vector.
	// Also check that the plugins being added actually exist.
	for (size_t i = 0; i < numPlugins; i++) {
		boss::Item plugin = boss::Item(std::string(reinterpret_cast<const char *>(plugins[i])));
		if (plugin.Exists(db->game))
			db->loadOrder.Insert(i, plugin);
		else
			return ReturnCode(boss::boss_error(boss::BOSS_ERROR_FILE_NOT_FOUND, plugin.Name()));
	}
	size_t loSize = db->loadOrder.Items().size();

	// Check to see if the masters before plugins rule is being obeyed.
	try {
		size_t pos = db->loadOrder.GetLastMasterPos(db->game);
		if (db->loadOrder.GetNextMasterPos(db->game, pos + 1) != loSize)  // Masters exist after the initial set of masters. Not allowed.
			return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Master files must load before other plugins.");

		// If Update.esm is installed, check if it is listed. If not, add it after the rest of the master files.
		if (db->game.Id() == boss::SKYRIM &&
		    boost::filesystem::exists(db->game.DataFolder() / "Update.esm") &&
		    db->loadOrder.FindItem("Update.esm", boss::MOD) == loSize) {
			db->loadOrder.Insert(pos + 1, boss::Item("Update.esm"));  // Previous master check ensures that GetLastMasterPos() will be not be loadOrder.size().
			loSize++;
		}
	} catch (boss::boss_error &e) {
		return ReturnCode(e);  // BOSS_ERRORs map directly to BOSS_API_ERRORs.
	}

	// Now iterate through the Data directory, adding any plugins to loadorder that aren't already in it.
	for (boost::filesystem::directory_iterator itr(db->game.DataFolder()); itr != boost::filesystem::directory_iterator(); ++itr) {
		if (boost::filesystem::is_regular_file(itr->status())) {
			boost::filesystem::path filename = itr->path().filename();
			std::string ext = filename.extension().string();
			if (boost::iequals(ext, ".ghost")) {
				filename = filename.stem();
				ext = filename.extension().string();
			}
			if (boost::iequals(ext, ".esp") || boost::iequals(ext, ".esm")) {
				LOG_TRACE("-- Found mod: '%s'", filename.string().c_str());
				// Add file to modlist. If the filename has a '.ghost' extension, remove it.
				const boss::Item tempItem = boss::Item(filename.string());
				if (db->loadOrder.FindItem(tempItem.Name(), boss::MOD) == loSize) {  // If the plugin is not present, add it.
					db->loadOrder.Insert(loSize, tempItem);
					loSize++;
				}
			}
		}
	}
	try {
		db->loadOrder.ApplyMasterPartition(db->game);  // Apply partition to sort those just added.
	} catch (boss::boss_error &e) {
		return ReturnCode(e);  // BOSS_ERRORs map directly to BOSS_API_ERRORs.
	}

	if (db->game.GetLoadOrderMethod() == boss::LOMETHOD_TEXTFILE) {  // Skyrim.
		// Now save the new loadorder. Also update the plugins.txt.
		try {
			db->loadOrder.SavePluginNames(db->game, db->game.LoadOrderFile(), false, false);
			db->loadOrder.SavePluginNames(db->game, db->game.ActivePluginsFile(), true, true);
		} catch (boss::boss_error &e) {
			return ReturnCode(e);  // BOSS_ERRORs map directly to BOSS_API_ERRORs.
		}
		// Now update cached mtime for plugins.txt.
		db->activePluginsMTime = GetActivePluginsMTime(db->game);
	} else {  // Non-skyrim.
		// Get the master time to derive dates from.
		std::time_t masterTime;
		try {
			masterTime = db->game.MasterFile().GetModTime(db->game);
		} catch (boss::boss_error &e) {
			return ReturnCode(e);
		}

		// Loop through given array and set the modification time for each one.
		std::vector<boss::Item> items = db->loadOrder.Items();
		for (size_t i = 0; i < loSize; i++) {
			if (!items[i].IsGameMasterFile(db->game)) {
				try {
					items[i].SetModTime(db->game, masterTime + i * 60);  // time_t is an integer number of seconds, so adding 60 on increases it by a minute.
				} catch(boss::boss_error &e) {
					return ReturnCode(e);
				}
			}
		}
	}
	// Now update cached mtime for load order.
	db->loadOrderMTime = GetLoadOrderMTime(db->game);

	return ReturnCode(BOSS_API_OK);
}

// Returns the contents of plugins.txt.
BOSS_API uint32_t GetActivePlugins(boss_db db, uint8_t ***plugins,
                                   size_t *numPlugins) {
	if (db == NULL || plugins == NULL || numPlugins == NULL)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	// Initialise vars.
	*numPlugins = 0;
	*plugins = NULL;

	// Free memory if already used.
	if (db->extStringArray != NULL) {
		for (size_t i = 0; i < db->extStringArraySize; i++)
			delete[] db->extStringArray[i];  // Clear all the uint8_t strings created.
		delete[] db->extStringArray;  // Clear the string array.
		db->extStringArray = NULL;
		db->extStringArraySize = 0;
	}

	// Load plugins.txt.
	try {
		if (boost::filesystem::exists(db->game.ActivePluginsFile()) &&
		   (GetActivePluginsMTime(db->game) != db->activePluginsMTime ||
		    GetLoadOrderMTime(db->game) != db->loadOrderMTime))
			db->activePlugins.Load(db->game, db->game.ActivePluginsFile());
	} catch (boss::boss_error &e) {
		return ReturnCode(e);  // BOSS_ERRORs map directly to BOSS_API_ERRORs.
	}

	// If Skyrim, we want to also output Skyrim.esm, Update.esm, if they are missing.
	size_t size = db->activePlugins.Items().size();
	if (db->game.Id() == boss::SKYRIM) {
		// Check if Skyrim.esm is missing.
		if (db->activePlugins.FindItem("Skyrim.esm", boss::MOD) == size) {
			db->activePlugins.Insert(0, boss::Item("Skyrim.esm"));
			size++;
		}
		// If Update.esm is installed, check if it is listed. If not, add it after the rest of the master files.
		if (boost::filesystem::exists(db->game.DataFolder() / "Update.esm") &&
		    db->activePlugins.FindItem("Update.esm", boss::MOD) == size) {
			try {
				db->activePlugins.Insert(db->activePlugins.GetLastMasterPos(db->game) + 1, boss::Item("Update.esm"));  // Previous master check ensures that GetLastMasterPos() will be not be loadorder.size().
				size++;
			} catch (boss::boss_error &e) {
				return ReturnCode(e);  // BOSS_ERRORs map directly to BOSS_API_ERRORs.
			}
		}
	}

	// Check array size. Exit if zero.
	std::vector<boss::Item> items = db->activePlugins.Items();
	if (items.empty())
		return ReturnCode(BOSS_API_OK);

	// Allocate memory.
	db->extStringArraySize = size;
	try {
		db->extStringArray = new uint8_t*[db->extStringArraySize];
		for (size_t i = 0; i < db->extStringArraySize; i++)
			db->extStringArray[i] = StringToUint8_tString(items[i].Name());
	} catch (std::bad_alloc /*&e*/) {
		return ReturnCode(boss::boss_error(boss::BOSS_ERROR_NO_MEM));
	}

	// Set outputs.
	*plugins = db->extStringArray;
	*numPlugins = db->extStringArraySize;

	return ReturnCode(BOSS_API_OK);
}

// Edits plugins.txt so that it lists the given plugins in load order.
// Encoding is handled by the saving code and doesn't need to be explicitly catered for here.
BOSS_API uint32_t SetActivePlugins(boss_db db, uint8_t **plugins,
                                   const size_t numPlugins) {
	if (db == NULL || plugins == NULL)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	if (numPlugins > 255)
		return ReturnCode(boss::boss_error(BOSS_API_ERROR_PLUGINS_FULL));

	// Check load order to see if it's valid.
	if (numPlugins > 0 &&
	    !boss::Item(std::string(reinterpret_cast<const char *>(plugins[0]))).IsGameMasterFile(db->game))
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Plugins may not be sorted before the game's master file.");

	// Fill the ItemList with the input.
	// Check that plugins being added actually exist.
	db->activePlugins.Clear();
	for (size_t i = 0; i < numPlugins; i++) {
		boss::Item plugin = boss::Item(std::string(reinterpret_cast<const char *>(plugins[i])));
		if (plugin.Exists(db->game)) {
			db->activePlugins.Insert(i, plugin);
			try {
				plugin.UnGhost(db->game);
			} catch (boss::boss_error &e) {
				return ReturnCode(e);
			}
		} else {
			return ReturnCode(boss::boss_error(boss::BOSS_ERROR_FILE_NOT_FOUND, plugin.Name()));
		}
	}

	// If Update.esm is installed, check if it is listed. If not, add it (order is decided later).
	size_t size = db->activePlugins.Items().size();
	if (db->game.Id() == boss::SKYRIM &&
	    boost::filesystem::exists(db->game.DataFolder() / "Update.esm") &&
	    db->activePlugins.FindItem("Update.esm", boss::MOD) == size) {
		db->activePlugins.Insert(size, boss::Item("Update.esm"));
	}

	// Now save plugins.txt.
	boss::boss_error conversionStatus(BOSS_API_OK);  // An error object that will be set to BOSS_API_WARN_BAD_FILENAME if one occurs.
	try {
		db->activePlugins.SavePluginNames(db->game, db->game.ActivePluginsFile(), false, true);  // False to ensure newly-added plugins are actually added.
	} catch (boss::boss_error &e) {
		if (e.getCode() == boss::BOSS_ERROR_ENCODING_CONVERSION_FAIL)
			conversionStatus = e;
		else
			return ReturnCode(e);  // BOSS_ERRORs map directly to BOSS_API_ERRORs.
	}

	// Now if running for textfile-based load order system, reorder plugins.txt, deriving the order from loadorder.txt.
	if (db->game.GetLoadOrderMethod() == boss::LOMETHOD_TEXTFILE) {
		// Now get the load order from loadorder.txt.
		try {
			if (GetLoadOrderMTime(db->game) != db->loadOrderMTime)
				db->loadOrder.Load(db->game, db->game.DataFolder());
			// Save the load order and derive plugins.txt order from it.
			db->loadOrder.SavePluginNames(db->game, db->game.LoadOrderFile(), false, false);
			db->loadOrder.SavePluginNames(db->game, db->game.ActivePluginsFile(), true, true);
			// Now update cached mtime.
			db->loadOrderMTime = GetLoadOrderMTime(db->game);
		} catch (boss::boss_error &e) {
			return ReturnCode(e);  // BOSS_ERRORs map directly to BOSS_API_ERRORs.
		}
	}
	// Now update cached mtimes.
	db->activePluginsMTime = GetActivePluginsMTime(db->game);

	return ReturnCode(conversionStatus);
}

// Gets the load order of the specified plugin, giving it as index. The first position
// in the load order is 0.
BOSS_API uint32_t GetPluginLoadOrder(boss_db db, const uint8_t *plugin,
                                     size_t *index) {
	if (db == NULL || plugin == NULL || index == NULL)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	// Initialise vars.
	*index = 0;

	// Now get the load order.
	try {
		if (GetLoadOrderMTime(db->game) != db->loadOrderMTime)
			db->loadOrder.Load(db->game, db->game.DataFolder());
	} catch (boss::boss_error &e) {
		return ReturnCode(e);  // BOSS_ERRORs map directly to BOSS_API_ERRORs.
	}

	// Now search for the given plugin.
	std::string pluginStr = std::string(reinterpret_cast<const char *>(plugin));
	size_t pos = db->loadOrder.FindItem(pluginStr, boss::MOD);
	if (pos == db->loadOrder.Items().size())
		return ReturnCode(boss::boss_error(boss::BOSS_ERROR_FILE_NOT_FOUND, pluginStr));

	// Set output.
	*index = pos;

	return ReturnCode(BOSS_API_OK);
}

// Sets the load order of the specified plugin, removing it from its current position
// if it has one. The first position in the load order is 0. If the index specified is
// greater than the number of plugins in the load order, the plugin will be inserted at
// the end of the load order.
BOSS_API uint32_t SetPluginLoadOrder(boss_db db, const uint8_t *plugin,
                                     size_t index) {
	if (db == NULL || plugin == NULL)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	std::string pluginStr = std::string(reinterpret_cast<const char *>(plugin));

	// Check to see if the plugin is being set to the first position in the load order. Only the game master file should be set there.
	if (index == 0 &&
	    !boost::iequals(pluginStr, db->game.MasterFile().Name()))  // Invalid.
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Plugins may not be sorted before the game's master file.");


	// Now get the current load order.
	try {
		if (GetLoadOrderMTime(db->game) != db->loadOrderMTime)
			db->loadOrder.Load(db->game, db->game.DataFolder());
		// Check to see if the masters before plugins rule is being obeyed.
		if (boss::Item(pluginStr).IsMasterFile(db->game) &&
		    index > db->loadOrder.GetLastMasterPos(db->game) + 1)  // Sorting master after plugin, not allowed.
			return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Masters may not be sorted after non-master plugins.");
		else if (!boss::Item(pluginStr).IsMasterFile(db->game) &&
		         index <= db->loadOrder.GetLastMasterPos(db->game))  // Sorting plugin before master, not allowed.
			return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Non-master plugins may not be sorted before master plugins.");
	} catch (boss::boss_error &e) {
		return ReturnCode(e);  // BOSS_ERRORs map directly to BOSS_API_ERRORs.
	}

	// Now search for the given plugin.
	size_t pos = db->loadOrder.FindItem(pluginStr, boss::MOD);
	if (pos == index)
		return ReturnCode(BOSS_API_OK);
	if (pos != db->loadOrder.Items().size())  // Plugin found. Erase it.
		db->loadOrder.Erase(pos);

	// Now insert the plugin into its new position.
	if (index >= db->loadOrder.Items().size())
		index = db->loadOrder.Items().size() - 1;
	db->loadOrder.Insert(index, boss::Item(pluginStr));

	if (db->game.GetLoadOrderMethod() == boss::LOMETHOD_TEXTFILE) {  // Skyrim.
		// Now write out the new loadorder.txt. Also update the plugins.txt.
		try {
			db->loadOrder.SavePluginNames(db->game, db->game.LoadOrderFile(), false, false);
			db->loadOrder.SavePluginNames(db->game, db->game.ActivePluginsFile(), true, true);
			// Now update cached mtime.
			db->activePluginsMTime = GetActivePluginsMTime(db->game);
		} catch (boss::boss_error &e) {
			return ReturnCode(e);
		}
	} else {  // Non-skyrim. Scan data directory, and arrange plugins found in timestamp load order.
		/*
		 * Optimised algorithm.
		 * 1.  Get the plugin's current position.
		 * 2.  Compare the difference between the plugin's current position and the new position against the size of the load order.
		 * 3.  If >= N - 3 (estimate of efficiency) where N is the size of the load order, redate all plugins and exit. Otherwise continue.
		 * 4.  Read plugin timestamp at current position.
		 * 5.  Read plugin timestamp at new position.
		 * 6.  Set the time delta to the difference in timestamps divided by the number of plugins between the two positions.
		 * 7.  Redate the plugins between the two positions inclusively, using the time delta calculated.
		 */

		// Get the master time to derive dates from.
		std::time_t masterTime;
		try {
			masterTime = db->game.MasterFile().GetModTime(db->game);
		} catch (boss::boss_error &e) {
			return ReturnCode(e);
		}

		// Now set the new timestamps.
		std::vector<boss::Item> items = db->loadOrder.Items();
		size_t max = items.size();

		if (index - pos >= max - 3 || pos - index >= max - 3) {  // Equivalent to abs(), which doesn't have size_t overloads.
			for (size_t i = 0; i < max; i++) {
				try {
					if (!items[i].IsGameMasterFile(db->game))
						items[i].SetModTime(db->game, masterTime + i * 60);  // time_t is an integer number of seconds, so adding 60 on increases it by a minute.
				} catch(boss::boss_error &e) {
					return ReturnCode(e);
				}
			}
		} else {
			try {
				std::time_t currTime = items[pos].GetModTime(db->game);
				std::time_t newTime = items[index].GetModTime(db->game);
				std::time_t deltaTime = (currTime - newTime) / (pos - index);  // Will always be > 0.
				size_t start;
				std::time_t startTime;
				if (pos < index) {
					start = pos;
					max = index;
					startTime = currTime;
				} else {
					start = index;
					max = pos;
					startTime = newTime;
				}
				for (size_t i = start; i < max; i++) {
					try {
						if (!items[i].IsGameMasterFile(db->game))
							items[i].SetModTime(db->game, startTime + (i - start) * deltaTime);  // time_t is an integer number of seconds, so adding 60 on increases it by a minute.
					} catch(boss::boss_error &e) {
						return ReturnCode(e);
					}
				}
			} catch(boss::boss_error &e) {
				return ReturnCode(e);
			}
		}
	}
	// Now update cached mtime.
	db->loadOrderMTime = GetLoadOrderMTime(db->game);

	return ReturnCode(BOSS_API_OK);
}

// Gets what plugin is at the specified load order position.
BOSS_API uint32_t GetIndexedPlugin(boss_db db, const size_t index,
                                   uint8_t **plugin) {
	if (db == NULL || plugin == NULL)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	// Initialise vars.
	*plugin = NULL;

	// Free memory if already used.
	delete[] db->extString;
	db->extString = NULL;

	// Now get the load order.
	try {
		if (GetLoadOrderMTime(db->game) != db->loadOrderMTime)
			db->loadOrder.Load(db->game, db->game.DataFolder());
	} catch (boss::boss_error &e) {
		return ReturnCode(e);  // BOSS_ERRORs map directly to BOSS_API_ERRORs.
	}

	// Check that the index is within bounds.
	if (index >= db->loadOrder.Items().size())
		return ReturnCode(boss::boss_error(BOSS_API_ERROR_INVALID_ARGS, "Given index is larger than the index of the last plugin in the load order."));

	// Allocate memory.
	try {
		db->extString = StringToUint8_tString(db->loadOrder.ItemAt(index).Name());
	} catch (std::bad_alloc /*&e*/) {
		return ReturnCode(boss::boss_error(boss::BOSS_ERROR_NO_MEM));
	}

	// Set outputs.
	*plugin = db->extString;

	return ReturnCode(BOSS_API_OK);
}

/*
 * If (active), adds the plugin to plugins.txt in its load order if it is not already present.
 * If (!active), removes the plugin from plugins.txt if it is present.
 * Encoding is handled by the saving code and doesn't need to be explicitly catered for here.
 */
BOSS_API uint32_t SetPluginActive(boss_db db, const uint8_t *plugin,
                                  const bool active) {
	if (db == NULL || plugin == NULL)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	// Catch Skyrim.esm and Update.esm for Skyrim.
	std::string pluginStr = std::string(reinterpret_cast<const char *>(plugin));
	if (db->game.Id() == boss::SKYRIM) {
		if (boost::iequals(pluginStr, "Skyrim.esm")) {
			if (active)
				return ReturnCode(BOSS_API_OK);
			else
				return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Skyrim.esm cannot be deactivated.");
		} else if (boost::filesystem::exists(db->game.DataFolder() / "Update.esm") &&
		           boost::iequals(pluginStr, "Update.esm")) {
			if (active)
				return ReturnCode(BOSS_API_OK);
			else
				return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Update.esm cannot be deactivated.");
		}
	}

	// Check that plugin exists if activating it.
	if (active && !boss::Item(pluginStr).Exists(db->game))
		return ReturnCode(boss::boss_error(boss::BOSS_ERROR_FILE_NOT_FOUND, pluginStr));

	// Unghost if ghosted.
	try {
		boss::Item(pluginStr).UnGhost(db->game);
	} catch (boss::boss_error &e) {
		return ReturnCode(e);
	}

	// Load plugins.txt.
	try {
		if (boost::filesystem::exists(db->game.ActivePluginsFile()) &&
		    GetActivePluginsMTime(db->game) != db->activePluginsMTime)
			db->activePlugins.Load(db->game, db->game.ActivePluginsFile());
	} catch (boss::boss_error &e) {
		return ReturnCode(e);  // BOSS_ERRORs map directly to BOSS_API_ERRORs.
	}

	// If Update.esm is installed, check if it is listed. If not, add it (order is decided later).
	size_t size = db->activePlugins.Items().size();
	if (db->game.Id() == boss::SKYRIM &&
	    boost::filesystem::exists(db->game.DataFolder() / "Update.esm") &&
	    db->activePlugins.FindItem("Update.esm", boss::MOD) == size) {
		db->activePlugins.Insert(size, boss::Item("Update.esm"));
	}

	// Check if the given plugin is in plugins.txt.
	if (db->activePlugins.FindItem(pluginStr, boss::MOD) != db->activePlugins.Items().size() && !active)  // Exists, but shouldn't.
		db->activePlugins.Erase(db->activePlugins.FindItem(pluginStr, boss::MOD));
	else if (db->activePlugins.FindItem(pluginStr, boss::MOD) == db->activePlugins.Items().size() && active)  // Doesn't exist, but should.
		db->activePlugins.Insert(db->activePlugins.Items().size(), pluginStr);

	// Check that there aren't too many plugins in plugins.txt.
	if (db->activePlugins.Items().size() > 255)
		return ReturnCode(boss::boss_error(BOSS_API_ERROR_PLUGINS_FULL));
	else if (db->game.GetLoadOrderMethod() == boss::LOMETHOD_TEXTFILE &&
	         db->activePlugins.Items().size() > 254)  // textfile-based system doesn't list Skyrim.esm in plugins.txt.
		return ReturnCode(boss::boss_error(BOSS_API_ERROR_PLUGINS_FULL));

	// Now save the change.
	try {
		db->activePlugins.SavePluginNames(db->game, db->game.ActivePluginsFile(), false, true);  // Must be false because we're not adding a currently active file, if we're adding something.
		if (db->game.GetLoadOrderMethod() == boss::LOMETHOD_TEXTFILE) {
			// Now get the current load order.
			if (GetLoadOrderMTime(db->game) != db->loadOrderMTime)
				db->loadOrder.Load(db->game, db->game.DataFolder());
			// Save the load order and derive plugins.txt order from it.
			db->loadOrder.SavePluginNames(db->game, db->game.LoadOrderFile(), false, false);
			db->loadOrder.SavePluginNames(db->game, db->game.ActivePluginsFile(), true, true);
			// Now update cached mtime.
			db->loadOrderMTime = GetLoadOrderMTime(db->game);
		}
	} catch (boss::boss_error &e) {
		return ReturnCode(e);
	}
	// Now update cached mtimes.
	db->activePluginsMTime = GetActivePluginsMTime(db->game);

	return ReturnCode(BOSS_API_OK);
}

// Checks to see if the given plugin is listed in plugins.txt.
BOSS_API uint32_t IsPluginActive(boss_db db, const uint8_t *plugin,
                                 bool *isActive) {
	if (db == NULL || plugin == NULL || isActive == NULL)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	std::string pluginStr = std::string(reinterpret_cast<const char *>(plugin));

	// Check if it's Skyrim, and Skyrim.esm/Update.esm, which are special cases.
	if (db->game.Id() == boss::SKYRIM) {
		if (boost::iequals(pluginStr, "Skyrim.esm") ||
		   (boost::filesystem::exists(db->game.DataFolder() / "Update.esm") &&
		    boost::iequals(pluginStr, "Update.esm"))) {
			*isActive = true;
			return ReturnCode(BOSS_API_OK);
		}
	}

	// Load plugins.txt. A hashset would be more efficient.
	boss::ItemList pluginsList;
	try {
		if (boost::filesystem::exists(db->game.ActivePluginsFile()) &&
		    GetActivePluginsMTime(db->game) != db->activePluginsMTime)
			db->activePlugins.Load(db->game, db->game.ActivePluginsFile());
	} catch (boss::boss_error &e) {
		return ReturnCode(e);  // BOSS_ERRORs map directly to BOSS_API_ERRORs.
	}

	// Check if the given plugin is in plugins.txt.
	if (db->activePlugins.FindItem(pluginStr, boss::MOD) != db->activePlugins.Items().size())
		*isActive = true;
	else
		*isActive = false;

	return ReturnCode(BOSS_API_OK);
}

// Checks to see if the given plugin is a master (using master bit flag value).
BOSS_API uint32_t IsPluginMaster(boss_db db, const uint8_t *plugin,
                                 bool *isMaster) {
	if (db == NULL || plugin == NULL || isMaster == NULL)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	*isMaster = boss::Item(std::string(reinterpret_cast<const char *>(plugin))).IsMasterFile(db->game);

	return ReturnCode(BOSS_API_OK);
}


//////////////////////////
// DB Access Functions
//////////////////////////

// Returns an array of the Bash Tags encounterred when loading the masterlist
// and userlist, and the number of tags in the returned array. The array and
// its contents are static and should not be freed by the client.
BOSS_API uint32_t GetBashTagMap(boss_db db, BashTag **tagMap,
                                size_t *numTags) {
	if (db == NULL || tagMap == NULL || numTags == NULL)  // Check for valid args.
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	if (db->extTagMap != NULL) {  // Check to see if bashTagMap is already allocated. An empty bashTagMap is a valid option, if no tags exist.
		*numTags = db->bashTagMap.size();  // Set size.
		if (*numTags != 0)
			*tagMap = db->extTagMap;
		else
			*tagMap = NULL;  // Don't return pointers to zero-length arrays, return NULL instead.
	} else {
		// Need to build internal Bash Tag map then feed it to the outside world.
		// This involves iterating through all mods to get the tags they add and remove, in the masterlist and userlist.
		// Off we go!
		std::unordered_set<std::string> tagsAdded, tagsRemoved;  // These are just so that we can use the general function, and will be combined later.

		// Initialise outputs.
		*tagMap = NULL;
		*numTags = 0;

		std::vector<boss::Item> items = db->game.masterlist.Items();
		size_t imax = items.size();
		for (size_t i = 0; i < imax; i++) {
			if (items[i].Messages().empty())
				continue;
			std::vector<boss::Message> messages = items[i].Messages();
			size_t jmax = messages.size();
			for (size_t j = 0; j < jmax; j++) {
				if (messages[j].Key() == boss::TAG)
					GetBashTagsFromString(messages[j].Data(), tagsAdded, tagsRemoved);
			}
		}
		std::vector<boss::Rule> rules = db->game.userlist.Rules();
		imax = rules.size();
		for (size_t i = 0; i < imax; i++) {
			std::vector<boss::RuleLine> lines = rules[i].Lines();
			size_t jmax = lines.size();
			for (size_t j = 0; j < jmax; j++) {
				if (lines[j].ObjectAsMessage().Key() == boss::TAG) {
					GetBashTagsFromString(lines[j].Object(), tagsAdded, tagsRemoved);
				}
			}
		}
		// Now tagsAdded and tagsRemoved each contain a list of unique tag strings.
		// Time to combine. :D
		uint32_t UID = 0;
		std::unordered_set<std::string>::iterator tagStringIter;
		for (tagStringIter = tagsAdded.begin(); tagStringIter != tagsAdded.end(); ++tagStringIter) {
			if (db->FindBashTag(*tagStringIter) == db->bashTagMap.end()) {  // Tag not found in bashTagMap. Add it!
				db->bashTagMap.insert(std::pair<uint32_t, std::string>(UID, *tagStringIter));
				UID++;  // Now increment UID to keep it U.
			}
		}
		for (tagStringIter = tagsRemoved.begin(); tagStringIter != tagsRemoved.end(); ++tagStringIter) {
			if (db->FindBashTag(*tagStringIter) == db->bashTagMap.end()) {  // Tag not found in bashTagMap. Add it!
				db->bashTagMap.insert(std::pair<uint32_t, std::string>(UID, *tagStringIter));
				UID++;  // Now increment UID to keep it U.
			}
		}

		// Check array size. Exit if zero. However, we need GetModBashTags to be able to tell if this function has been run or not.
		// An empty bashTagMap and a null extTagMap are indistinguishable output from this function not being run at all.
		// So allocate memory to a zero length array for extTagMap.
		if (db->bashTagMap.empty()) {
			try {
				db->extTagMap = new BashTag[0];
			} catch (std::bad_alloc /*&e*/) {
				return ReturnCode(boss::boss_error(boss::BOSS_ERROR_NO_MEM));
			}
			return ReturnCode(BOSS_API_OK);
		}

		// Now to convert for the outside world.
		size_t mapSize = db->bashTagMap.size();  // Set size.

		// Allocate memory, then loop through internal bashTagMap and fill output elements.
		try {
			db->extTagMap = new BashTag[mapSize];
			for (size_t i = 0; i < mapSize; i++) {
				uint32_t ii = uint32_t(i);
				db->extTagMap[i].id = ii;
				db->extTagMap[i].name = StringToUint8_tString(db->bashTagMap[ii]);
			}
		} catch (std::bad_alloc /*&e*/) {
			return ReturnCode(boss::boss_error(boss::BOSS_ERROR_NO_MEM));
		}

		*tagMap = db->extTagMap;
		*numTags = mapSize;
	}
	return ReturnCode(BOSS_API_OK);
}

// Returns arrays of Bash Tag UIDs for Bash Tags suggested for addition and removal
// by BOSS's masterlist and userlist, and the number of tags in each array.
// The returned arrays are valid until the db is destroyed or until the Load
// function is called.  The arrays should not be freed by the client. modName is
// case-insensitive. If no Tags are found for an array, the array pointer (*tagIds)
// will be NULL. The userlistModified bool is true if the userlist contains Bash Tag
// suggestion message additions.
BOSS_API uint32_t GetModBashTags(boss_db db,
                                 const uint8_t *plugin,
                                 uint32_t **tagIds_added,
                                 size_t *numTags_added,
                                 uint32_t **tagIds_removed,
                                 size_t *numTags_removed,
                                 bool *userlistModified) {
	// Check for valid args.
	if (db == NULL || plugin == NULL || userlistModified == NULL ||
	    numTags_added == NULL || numTags_removed == NULL ||
	    tagIds_removed == NULL || tagIds_added == NULL)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	// Convert modName.
	std::string mod(reinterpret_cast<const char *>(plugin));

	if (mod.empty())
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Plugin name is empty.");

	// Initialise pointers to null and zero tag counts.
	*numTags_added = 0;
	*numTags_removed = 0;
	*tagIds_removed = NULL;
	*tagIds_added = NULL;
	*userlistModified = false;

	if (db->extTagMap == NULL)
		return ReturnCode(BOSS_API_ERROR_NO_TAG_MAP);

	// Bash Tag temporary internal holders.
	std::unordered_set<std::string> tagsAdded, tagsRemoved;

	// Now search filtered masterlist for mod.
	size_t pos = db->game.masterlist.FindItem(mod, boss::MOD);
	if (pos != db->game.masterlist.Items().size()) {
		std::vector<boss::Message> messages = db->game.masterlist.ItemAt(pos).Messages();
		for (std::vector<boss::Message>::iterator messageIter = messages.begin(); messageIter != messages.end(); ++messageIter) {
			if (messageIter->Key() == boss::TAG)
				GetBashTagsFromString(messageIter->Data(), tagsAdded, tagsRemoved);
		}
	}

	// Now search userlist for mod.
	pos = db->game.userlist.FindRule(mod, true);
	if (pos != db->game.userlist.Rules().size()) {
		std::vector<boss::RuleLine> lines = db->game.userlist.RuleAt(pos).Lines();
		for (std::vector<boss::RuleLine>::iterator lineIter = lines.begin(); lineIter != lines.end(); ++lineIter) {
			if (lineIter->Key() == boss::REPLACE &&
			   (!tagsAdded.empty() || !tagsRemoved.empty())) {
				tagsAdded.clear();
				tagsRemoved.clear();
				*userlistModified = true;
			}
			if (lineIter->ObjectAsMessage().Key() == boss::TAG) {
				GetBashTagsFromString(lineIter->Object(), tagsAdded, tagsRemoved);
				*userlistModified = true;
			}
		}
	}

	// Now we convert strings to UIDs.
	std::vector<uint32_t> tagsAddedUIDs, tagsRemovedUIDs;
	std::unordered_set<string>::iterator tagStringIter;
	for (tagStringIter = tagsAdded.begin(); tagStringIter != tagsAdded.end(); ++tagStringIter) {
		std::map<uint32_t, std::string>::iterator mapPos = db->FindBashTag(*tagStringIter);
		if (mapPos != db->bashTagMap.end())  // Tag found in bashTagMap. Get the UID.
			tagsAddedUIDs.push_back(mapPos->first);
	}
	for (tagStringIter = tagsRemoved.begin(); tagStringIter != tagsRemoved.end(); ++tagStringIter) {
		std::map<uint32_t, std::string>::iterator mapPos = db->FindBashTag(*tagStringIter);
		if (mapPos != db->bashTagMap.end())  // Tag found in bashTagMap. Get the UID.
			tagsRemovedUIDs.push_back(mapPos->first);
	}
	// Now set the sizes (we kept the two separate just in case some tags didn't have UIDs, which would change the size of the outputted array.
	size_t numAdded = tagsAddedUIDs.size();
	size_t numRemoved = tagsRemovedUIDs.size();

	// Free memory.
	delete[] db->extAddedTagIds;
	delete[] db->extRemovedTagIds;
	db->extAddedTagIds = NULL;
	db->extRemovedTagIds = NULL;

	// Allocate memory.
	try {
		if (numAdded != 0) {
			db->extAddedTagIds = new uint32_t[numAdded];
			for (size_t i = 0; i < numAdded; i++)
				db->extAddedTagIds[i] = tagsAddedUIDs[i];
		}
		if (numRemoved != 0) {
			db->extRemovedTagIds = new uint32_t[numRemoved];
			for (size_t i = 0; i < numRemoved; i++)
				db->extRemovedTagIds[i] = tagsRemovedUIDs[i];
		}
	} catch (std::bad_alloc /*&e*/) {
		return ReturnCode(boss::boss_error(boss::BOSS_ERROR_NO_MEM));
	}

	// Set outputs.
	*tagIds_added = db->extAddedTagIds;
	*tagIds_removed = db->extRemovedTagIds;
	*numTags_added = numAdded;
	*numTags_removed = numRemoved;

	return ReturnCode(BOSS_API_OK);
}

// Returns the message associated with a dirty mod and whether the mod needs
// cleaning. If a mod has no dirty mmessage, *message will be NULL. modName is
// case-insensitive. The return values for needsCleaning are:
//   BOSS_API_CLEAN_NO
//   BOSS_API_CLEAN_YES
//   BOSS_API_CLEAN_UNKNOWN
// The message string is valid until the db is destroyed or until a Load
// function is called. The string should not be freed by the client.
BOSS_API uint32_t GetDirtyMessage(boss_db db, const uint8_t *plugin,
                                  uint8_t **message, uint32_t *needsCleaning) {
	// Check for valid args.
	if (db == NULL || plugin == NULL || message == NULL ||
	    needsCleaning == NULL)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	// Convert modName.
	std::string mod(reinterpret_cast<const char *>(plugin));

	if (mod.empty())
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Plugin name is empty.");

	// Initialise pointers.
	*message = NULL;
	*needsCleaning = BOSS_API_CLEAN_UNKNOWN;

	// Now search filtered masterlist for mod.
	size_t pos = db->game.masterlist.FindItem(mod, boss::MOD);
	if (pos != db->game.masterlist.Items().size()) {
		std::vector<boss::Message> messages = db->game.masterlist.ItemAt(pos).Messages();
		for (std::vector<boss::Message>::iterator messageIter = messages.begin(); messageIter != messages.end(); ++messageIter) {
			if (messageIter->Key() == boss::DIRTY) {
				try {
					db->extString = StringToUint8_tString(messageIter->Data());
				} catch (std::bad_alloc /*&e*/) {
					return ReturnCode(boss::boss_error(boss::BOSS_ERROR_NO_MEM));
				}
				 *message = db->extString;

				if (messageIter->Data().find("Do not clean.") != std::string::npos)  // Mod should not be cleaned.
					*needsCleaning = BOSS_API_CLEAN_NO;
				else  // Mod should be cleaned.
					*needsCleaning = BOSS_API_CLEAN_YES;
				break;
			}
		}
	}

	return ReturnCode(BOSS_API_OK);
}

// Returns the messages attached to the given plugin. Messages are valid until Load,
// DestroyBossDb or GetPluginMessages are next called. plugin is case-insensitive.
// If no messages are attached, *messages will be NULL and numMessages will equal 0.
BOSS_API uint32_t GetPluginMessages(boss_db db,
                                    const uint8_t *plugin,
                                    BossMessage **messages,
                                    size_t *numMessages) {
	// Check for valid args.
	if (db == NULL || plugin == NULL || messages == NULL ||
	    numMessages == NULL)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	// Convert modName.
	std::string mod(reinterpret_cast<const char *>(plugin));

	if (mod.empty())
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Plugin name is empty.");

	// Initialise pointers.
	*messages = NULL;
	*numMessages = 0;

	// Clear previously allocated memory.
	if (db->extMessageArray != NULL) {
		for (size_t i = 0; i < db->extMessageArraySize; i++)
			delete[] db->extMessageArray[i].message;  // Gotta clear those allocated strings.
		delete[] db->extMessageArray;
		db->extMessageArray = NULL;
		db->extMessageArraySize = 0;
	}

	// Now search filtered masterlist for mod.
	std::vector<boss::Message> modMessages;
	size_t pos = db->game.masterlist.FindItem(mod, boss::MOD);
	if (pos != db->game.masterlist.Items().size())
		modMessages = db->game.masterlist.ItemAt(pos).Messages();

	if (modMessages.empty())
		return ReturnCode(BOSS_API_OK);

	// Allocate memory, then loop through internal bashTagMap and fill output elements.
	db->extMessageArraySize = modMessages.size();
	try {
		db->extMessageArray = new BossMessage[db->extMessageArraySize];
		for (size_t i = 0; i < db->extMessageArraySize; i++) {
			db->extMessageArray[i].type = modMessages[i].Key();
			db->extMessageArray[i].message = StringToUint8_tString(modMessages[i].Data());
		}
	} catch (std::bad_alloc /*&e*/) {
		return ReturnCode(boss::boss_error(boss::BOSS_ERROR_NO_MEM));
	}

	*messages = db->extMessageArray;
	*numMessages = db->extMessageArraySize;

	return ReturnCode(BOSS_API_OK);
}

// Checks if the given mod is present in BOSS's masterlist for the DB's game.
BOSS_API uint32_t IsRecognised(boss_db db, const uint8_t * plugin,
                               bool *recognised) {
	if (db == NULL || plugin == NULL || recognised == NULL)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	// Search filtered masterlist.
	if (db->game.masterlist.FindItem(std::string(reinterpret_cast<const char *>(plugin)), boss::MOD) != db->game.masterlist.Items().size())
		*recognised = true;
	else
		*recognised = false;

	return ReturnCode(BOSS_API_OK);
}

// Writes a minimal masterlist that only contains mods that have Bash Tag suggestions,
// and/or dirty messages, plus the Tag suggestions and/or messages themselves and their
// conditions, in order to create the Wrye Bash taglist. outputFile is the path to use
// for output. If outputFile already exists, it will only be overwritten if overwrite is true.
BOSS_API uint32_t DumpMinimal(boss_db db, const uint8_t *outputFile,
                              const bool overwrite) {
	// Check for valid args.
	if (db == NULL || outputFile == NULL)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	std::string path(reinterpret_cast<const char *>(outputFile));
	if (!boost::filesystem::exists(path) || overwrite) {
		boss::fstream::ofstream mlist(path.c_str());
		if (mlist.fail()) {
			return ReturnCode(boss::boss_error(boss::BOSS_ERROR_FILE_WRITE_FAIL, path));
		} else {
			// Iterate through items, printing out all relevant info.
			std::vector<boss::Item> items = db->rawMasterlist.Items();  // Filtered works, but not raw.
			for (std::vector<boss::Item>::iterator itemIter = items.begin(); itemIter != items.end(); ++itemIter) {
				if (itemIter->Type() == boss::MOD || itemIter->Type() == boss::REGEX) {
					bool namePrinted = false;
					std::vector<boss::Message> messages = itemIter->Messages();
					for (std::vector<boss::Message>::iterator messageIter = messages.begin(); messageIter != messages.end(); ++messageIter) {
						if (messageIter->Key() == boss::TAG ||
						    messageIter->Key() == boss::DIRTY) {
							if (!namePrinted) {
								if (!itemIter->Conditions().empty()) {
									mlist << itemIter->Conditions() << ' ';
									if (itemIter->Type() == boss::MOD)
										mlist << "MOD: ";
								}
								if (itemIter->Type() == boss::REGEX)
									mlist << "REGEX: ";
								mlist << itemIter->Name() << std::endl;  // Print the mod name.
								namePrinted = true;
							}
							if (!messageIter->Conditions().empty())
								mlist << ' ' << messageIter->Conditions();
							mlist << " " << messageIter->KeyToString() << ": " << messageIter->Data() << std::endl;
						}
					}
				}
			}
			mlist.close();
		}
		return ReturnCode(BOSS_API_OK);
	} else {
		return ReturnCode(boss::boss_error(BOSS_API_ERROR_FILE_WRITE_FAIL, path));
	}
}
