/*	BOSS

	A "one-click" program for users that quickly optimises and avoids 
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge, 
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

    Copyright (C) 2011    BOSS Development Team.

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


#include "BOSS-API.h"
#include "BOSS-Common.h"
#include <boost/algorithm/string.hpp>
#include <boost/unordered_set.hpp>
#include <map>
#include <clocale>
#include <boost/filesystem/detail/utf8_codecvt_facet.hpp>
#include <boost/filesystem.hpp>

using namespace std;
using namespace boss;

using boost::algorithm::trim_copy;
using boost::algorithm::to_lower_copy;

////////////////////////
// Types (Internal)
////////////////////////

// Version string.
static const string boss_version = IntToString(BOSS_VERSION_MAJOR)+"."+IntToString(BOSS_VERSION_MINOR)+"."+IntToString(BOSS_VERSION_PATCH);

// Last error details buffer.
string lastErrorDetails = "";

//Error buffer and version string output pointers (for memory management).
uint8_t * extErrorPointer = NULL;
uint8_t * extVersionPointer = NULL;

// Structure for a single plugin's data.
struct modEntry {
	string name;						//Plugin filename, case NOT preserved.
	vector<uint32_t> bashTagsAdded;		//Vector of Bash Tags added unique IDs.
	vector<uint32_t> bashTagsRemoved;	//Vector of Bash Tags removed unique IDs.
	string cleaningMessage;				//Doesn't need to be a vector because there will only be one valid cleaning message outputted by the parser.
};

// Database structure.
struct _boss_db_int {
	//Internal data storage.
	fs::path db_data_path;
	uint32_t db_game;
	ItemList rawMasterlist, filteredMasterlist;
	RuleList userlist;
	map<uint32_t,string> bashTagMap;				//A hashmap containing all the Bash Tag strings found in the masterlist and userlist and their unique IDs.
													//Ordered to make ensuring UIDs easy (check the UID of the last element then increment). Strings are case-preserved.
	//Externally-visible data pointers storage.
	BashTag * extTagMap;				//Holds the pointer for the bashTagMap returned by GetBashTagMap().
	uint32_t * extAddedTagIds;
	uint32_t * extRemovedTagIds;
	uint8_t * extString;
	uint8_t ** extStringArray;

	//Pointer array sizes.
	size_t extStringArraySize;

	//Constructor
	_boss_db_int() {
		extTagMap = NULL;
		extAddedTagIds = NULL;
		extRemovedTagIds = NULL;
		extString = NULL;
		extStringArray = NULL;
	}

	//Get a Bash Tag's string name from its UID.
	string GetTagString(uint32_t uid) {
		map<uint32_t, string>::iterator mapPos = bashTagMap.find(uid);
		if (mapPos != bashTagMap.end())
			return mapPos->second;
		else
			return "";
	}

	//Get a Bash Tag's position in the bashTagMap from its string name.
	map<uint32_t, string>::iterator FindBashTag(string value) {
		map<uint32_t, string>::iterator mapPos = bashTagMap.begin();
		while (mapPos != bashTagMap.end()) {
			if (mapPos->second == value)
				break;
			++mapPos;
		}
		return mapPos;
	}
};

// The following are the possible codes that the API can return.
// Taken from BOSS-Common's Error.h and extended.
BOSS_API const uint32_t BOSS_API_OK								= BOSS_OK;
BOSS_API const uint32_t BOSS_API_OK_NO_UPDATE_NECESSARY			= BOSS_OK_NO_UPDATE_NECESSARY;
BOSS_API const uint32_t BOSS_API_WARN_BAD_FILENAME				= BOSS_ERROR_ENCODING_CONVERSION_FAIL;
BOSS_API const uint32_t BOSS_API_WARN_LO_MISMATCH				= BOSS_ERROR_LO_MISMATCH;
BOSS_API const uint32_t BOSS_API_ERROR_FILE_WRITE_FAIL			= BOSS_ERROR_FILE_WRITE_FAIL;
BOSS_API const uint32_t BOSS_API_ERROR_FILE_DELETE_FAIL			= BOSS_ERROR_FS_FILE_DELETE_FAIL;
BOSS_API const uint32_t BOSS_API_ERROR_FILE_NOT_UTF8			= BOSS_ERROR_FILE_NOT_UTF8;
BOSS_API const uint32_t BOSS_API_ERROR_FILE_NOT_FOUND			= BOSS_ERROR_FILE_NOT_FOUND;
BOSS_API const uint32_t BOSS_API_ERROR_MASTER_TIME_READ_FAIL	= BOSS_ERROR_FS_FILE_MOD_TIME_READ_FAIL;
BOSS_API const uint32_t BOSS_API_ERROR_MOD_TIME_READ_FAIL		= BOSS_ERROR_FS_FILE_MOD_TIME_READ_FAIL;
BOSS_API const uint32_t BOSS_API_ERROR_MOD_TIME_WRITE_FAIL		= BOSS_ERROR_FS_FILE_MOD_TIME_WRITE_FAIL;
BOSS_API const uint32_t BOSS_API_ERROR_CONDITION_EVAL_FAIL		= BOSS_ERROR_CONDITION_EVAL_FAIL;
BOSS_API const uint32_t BOSS_API_ERROR_REGEX_EVAL_FAIL			= BOSS_ERROR_REGEX_EVAL_FAIL;
BOSS_API const uint32_t BOSS_API_ERROR_PARSE_FAIL				= BOSS_ERROR_FILE_PARSE_FAIL;
BOSS_API const uint32_t BOSS_API_ERROR_NO_MEM					= BOSS_ERROR_NO_MEM;
BOSS_API const uint32_t BOSS_API_ERROR_INVALID_ARGS				= BOSS_ERROR_INVALID_ARGS;
BOSS_API const uint32_t BOSS_API_ERROR_NETWORK_FAIL				= BOSS_ERROR_NETWORK_FAIL;
BOSS_API const uint32_t BOSS_API_ERROR_NO_INTERNET_CONNECTION	= BOSS_ERROR_NO_INTERNET_CONNECTION;
BOSS_API const uint32_t BOSS_API_ERROR_NO_TAG_MAP				= BOSS_ERROR_NO_TAG_MAP;
BOSS_API const uint32_t BOSS_API_ERROR_PLUGINS_FULL				= BOSS_ERROR_PLUGINS_FULL;
BOSS_API const uint32_t BOSS_API_ERROR_GAME_NOT_FOUND			= BOSS_ERROR_NO_GAME_DETECTED;
BOSS_API const uint32_t BOSS_API_RETURN_MAX						= BOSS_API_ERROR_NO_TAG_MAP;

// The following are the mod cleanliness states that the API can return.
BOSS_API const uint32_t BOSS_API_CLEAN_NO		= 0;
BOSS_API const uint32_t BOSS_API_CLEAN_YES		= 1;
BOSS_API const uint32_t BOSS_API_CLEAN_UNKNOWN	= 2;

// The following are for signifying what load order method is being used:
BOSS_API const uint32_t BOSS_API_LOMETHOD_TIMESTAMP	= 0;
BOSS_API const uint32_t BOSS_API_LOMETHOD_TEXTFILE	= 1;

// The following are the games identifiers used by the API.
BOSS_API const uint32_t BOSS_API_GAME_OBLIVION	= OBLIVION;
BOSS_API const uint32_t BOSS_API_GAME_FALLOUT3	= FALLOUT3;
BOSS_API const uint32_t BOSS_API_GAME_FALLOUTNV	= FALLOUTNV;
BOSS_API const uint32_t BOSS_API_GAME_NEHRIM	= NEHRIM;
BOSS_API const uint32_t BOSS_API_GAME_SKYRIM	= SKYRIM;


//////////////////////////////
// Internal Functions
//////////////////////////////

void GetBashTagsFromString(const string message, boost::unordered_set<string>& tagsAdded, boost::unordered_set<string>& tagsRemoved) {
	//Need to collect Bash Tags. Search for the Bash Tag listing syntaxes.
	size_t pos1,pos2 = string::npos;
	string addedList, removedList;
	pos1 = message.find("{{BASH:");
	if (pos1 != string::npos)
		pos2 = message.find("}}", pos1);
	if (pos2 != string::npos)
		addedList = message.substr(pos1+7,pos2-pos1-7);

	if (!addedList.empty()) {
		//Now we move through the list of added Tags.
		//Search the set of already added Tags for each one and only add if it's not found.
		string name;
		pos1 = 0;
		pos2 = addedList.find(",", pos1);
		while (pos2 != string::npos) {
			name = trim_copy(addedList.substr(pos1,pos2-pos1));
			if (tagsAdded.find(name) == tagsAdded.end())
				tagsAdded.insert(name);
			pos1 = pos2+1;
			pos2 = addedList.find(",", pos1);
		}
		name = trim_copy(addedList.substr(pos1));
		if (tagsAdded.find(name) == tagsAdded.end())
			tagsAdded.insert(name);
	}

	pos1 = message.find("[");
	pos2 = string::npos;
	if (pos1 != string::npos)
		pos2 = message.find("]", pos1);
	if (pos2 != string::npos)
		removedList = message.substr(pos1+1,pos2-pos1-1);

	if (!removedList.empty()) {
		string name;
		pos1 = 0;
		pos2 = removedList.find(",", pos1);
		while (pos2 != string::npos) {
			name = trim_copy(removedList.substr(pos1,pos2-pos1));
			if (tagsRemoved.find(name) == tagsRemoved.end())
				tagsRemoved.insert(name);
			pos1 = pos2+1;
			pos2 = removedList.find(",", pos1);
		}
		name = trim_copy(removedList.substr(pos1));
		if (tagsRemoved.find(name) == tagsRemoved.end())
				tagsRemoved.insert(name);
	}
}

//This allocates memory - don't forget to free it later.
uint8_t * StringToUint8_tString(string str) {
	uint8_t * p = (uint8_t*)malloc((str.length() + 1) * sizeof(uint8_t));
	if (p == NULL)
		return p;
	for (size_t j=0; j < str.length(); j++) {
		p[j] = str[j];
	}
	p[str.length()] = '\0';
	return p;
}

void DestroyPointers(boss_db db) {
	free(db->extTagMap);
	free(db->extAddedTagIds);
	free(db->extRemovedTagIds);
	free(db->extString);

	if (db->extStringArray != NULL) {
		for (size_t i=0; i<db->extStringArraySize; i++)
			free(db->extStringArray[i]);  //Clear all the uint8_t strings created.
		free(db->extStringArray);  //Clear the string array.
	}
}

uint32_t ReturnCode(uint32_t returnCode, string details) {
	lastErrorDetails = details;
	return returnCode;
}

uint32_t ReturnCode(uint32_t returnCode) {
	lastErrorDetails = "";
	return returnCode;
}


//////////////////////////////
// Error Handling Functions
//////////////////////////////

// Outputs a string giving the details of the last time an error or 
// warning return code was returned by a function.
BOSS_API uint32_t GetLastErrorDetails(const uint8_t ** details) {
	if (details == NULL)  //Check for valid args.
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	free(extErrorPointer);

	extErrorPointer = StringToUint8_tString(lastErrorDetails);
	if (extErrorPointer == NULL)
		return ReturnCode(BOSS_API_ERROR_NO_MEM, "Memory allocation failed.");
	*details = extErrorPointer;
	return ReturnCode(BOSS_API_OK);
}


//////////////////////////////
// Version Functions
//////////////////////////////

// Returns whether this version of BOSS supports the API from the given 
// BOSS version. Abstracts BOSS API stability policy away from clients.
BOSS_API bool IsCompatibleVersion (const uint32_t bossVersionMajor, const uint32_t bossVersionMinor, const uint32_t bossVersionPatch) {
	if (bossVersionMajor < 2)  //The 1.9 API was different.
		return false;
	if (bossVersionMajor <= BOSS_VERSION_MAJOR && bossVersionMinor <= BOSS_VERSION_MINOR)
		return true;
	else
		return false;
}

// Returns the version string for this version of BOSS.
// The string exists for the lifetime of the library.
BOSS_API uint32_t GetVersionString (const uint8_t ** bossVersionStr) {
	if (bossVersionStr == NULL) //Check for valid args.
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");
	
	free(extVersionPointer);

	extVersionPointer = StringToUint8_tString(boss_version);
	if (extVersionPointer == NULL)
		return ReturnCode(BOSS_API_ERROR_NO_MEM, "Memory allocation failed.");
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
BOSS_API uint32_t CreateBossDb  (boss_db * db, const uint32_t clientGame,
											   const uint8_t * dataPath) {
	if (db == NULL) //Check for valid args.
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");
	else if (clientGame != OBLIVION && clientGame != FALLOUT3 && clientGame != FALLOUTNV && clientGame != NEHRIM && clientGame != SKYRIM)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Invalid game specified.");

	boss_db retVal = new _boss_db_int;
	if (retVal == NULL)
		return ReturnCode(BOSS_API_ERROR_NO_MEM, "Memory allocation failed.");

	//Set the locale to get encoding conversions working correctly.
	setlocale(LC_CTYPE, "");
	locale global_loc = locale();
	locale loc(global_loc, new boost::filesystem::detail::utf8_codecvt_facet());
	boost::filesystem::path::imbue(loc);

	//Set game. Because this is a global and there may be multiple DBs for different games,
	//each time a DB's function is called, it should be reset. Same with dataPath.
	retVal->db_game = clientGame;
	if (dataPath != NULL) {
		retVal->db_data_path = fs::path(reinterpret_cast<const char *>(dataPath));
		if (data_path.empty()) {
			DestroyBossDb(retVal);
			return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Data path is empty.");
		}
	} else {
		//If dataPath is null, then we need to look for the specified game.
		if (fs::exists(data_path / GetGameMasterFile(clientGame)))
			gl_current_game = clientGame;
		else if (clientGame == OBLIVION && (RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Bethesda Softworks\\Oblivion") || RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Wow6432Node\\Bethesda Softworks\\Oblivion")))  //Look for Oblivion.
			gl_current_game = clientGame;
		else if (clientGame == NEHRIM && RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Nehrim - At Fate's Edge_is1"))  //Look for Nehrim.
			gl_current_game = clientGame;
		else if (clientGame == SKYRIM && (RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Bethesda Softworks\\Skyrim") || RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Wow6432Node\\Bethesda Softworks\\Skyrim")))  //Look for Skyrim.
			gl_current_game = clientGame;
		else if (clientGame == FALLOUT3 && (RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Bethesda Softworks\\Fallout3") || RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Wow6432Node\\Bethesda Softworks\\Fallout3")))  //Look for Fallout 3.
			gl_current_game = clientGame;
		else if (clientGame == FALLOUTNV && (RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Bethesda Softworks\\FalloutNV") || RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Wow6432Node\\Bethesda Softworks\\FalloutNV")))  //Look for Fallout New Vegas.
			gl_current_game = clientGame;
		else  //The game has not been found.
			return ReturnCode(BOSS_API_ERROR_GAME_NOT_FOUND);

		//If we've gotten here, then the game has been found.
		SetDataPath(clientGame);
		retVal->db_data_path = data_path;
	}
	gl_current_game = retVal->db_game;

	*db = retVal;

	//Now check if plugins.txt and loadorder.txt are in sync.
	if (fs::exists(plugins_path()) && fs::exists(loadorder_path())) {
		//Load loadorder.txt and save a temporary filtered version.
		ItemList loadorder;
		try {
			loadorder.Load(data_path);
			loadorder.SavePluginNames(loadorder_path().string() + ".new", true, true);
		} catch (boss_error e) {
			DestroyBossDb(retVal);
			return ReturnCode(e.getCode(), e.getString());  //BOSS_ERRORs map directly to BOSS_API_ERRORs.
		}

		uint32_t crc1 = GetCrc32(plugins_path());
		uint32_t crc2 = GetCrc32(loadorder_path().string() + ".new");

		//Now delete temporary filtered loadorder.txt.
		try {
			fs::remove(loadorder_path().string() + ".new");
		} catch (fs::filesystem_error e) {
			DestroyBossDb(retVal);
			return ReturnCode(BOSS_API_ERROR_FILE_DELETE_FAIL, loadorder_path().string() + ".new");
		}

		//Since plugins.txt is derived from loadorder.txt in the same manner as the temporary file created above,
		//with the derivation occurring whenever loadorder.txt is changed, if plugins.txt has not been changed
		//by something other than the API (eg. the launcher), then the CRCs will match. Otherwise they will differ.
		if (crc1 != crc2)
			return ReturnCode(BOSS_API_WARN_LO_MISMATCH);
	}
	return ReturnCode(BOSS_API_OK);
}

BOSS_API void     DestroyBossDb (boss_db db) {
	if (db == NULL)
		return;

	//Free memory at pointers stored in structure.
	DestroyPointers(db);
	
	//Now delete DB.
	delete db;
}

BOSS_API void	  CleanUpAPI() {
	free(extErrorPointer);
	free(extVersionPointer);
}


///////////////////////////////////
// Database Loading Functions
///////////////////////////////////

// Loads the masterlist and userlist from the paths specified.
// Can be called multiple times. On error, the database is unchanged.
// Paths are case-sensitive if the underlying filesystem is case-sensitive.

// Masterlist and userlist loading are internally independent, but occur in
// same function for ease-of-use by clients.
BOSS_API uint32_t Load (boss_db db, const uint8_t * masterlistPath,
									const uint8_t * userlistPath) {
	ItemList masterlist;
	RuleList userlist;
	
	//Check for valid args.
	if (db == NULL || masterlistPath == NULL)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	//Set globals again in case they've been changed.
	data_path = db->db_data_path;
	gl_current_game = db->db_game;
	
	//PATH SETTING
	fs::path masterlist_path = fs::path(reinterpret_cast<const char *>(masterlistPath));
	if (masterlist_path.empty())
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Masterlist path is empty.");
	fs::path userlist_path;
	if (userlistPath != NULL) {
		userlist_path = fs::path(reinterpret_cast<const char *>(userlistPath));	
		if (userlist_path.empty())
			return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Userlist path is empty.");
	}
		
	//PARSING - Masterlist
	try {
		masterlist.Load(masterlist_path);
	} catch (boss_error e) {
		return ReturnCode(e.getCode(), e.getString());  //BOSS_ERRORs map directly to BOSS_API_ERRORs.
	}

	//PARSING - Userlist
	if (userlistPath != NULL) {
		try {
			userlist.Load(userlist_path);
		} catch (boss_error e) {
			db->userlist.rules.clear();  //If userlist has parsing errors, empty it so no rules are applied.
			return ReturnCode(e.getCode(), e.getString());  //BOSS_ERRORs map directly to BOSS_API_ERRORs.
		}
	}

	//FREE CURRENT POINTERS
	//Free memory at pointers stored in structure.
	DestroyPointers(db);
	
	//DB SET
	db->rawMasterlist = masterlist;
	db->filteredMasterlist = masterlist;  //Not actually filtered, but retrival functions assume filtered masterlist is populated.
	db->userlist = userlist;
	db->bashTagMap.clear();
	return ReturnCode(BOSS_API_OK);
}

// Re-evaluates all conditional lines and regex mods in rawMasterlist, 
// putting the output into filteredMasterlist.
BOSS_API uint32_t EvalConditionals(boss_db db) {
	//Check for valid args.
	if (db == NULL)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");
		
	//Set globals again in case they've been changed.
	data_path = db->db_data_path;
	gl_current_game = db->db_game;

	//First re-evaluate conditionals.
	ItemList masterlist = db->rawMasterlist;
	try {
		masterlist.EvalConditions();
	} catch (boss_error e) {
		return ReturnCode(e.getCode(), e.getString());  //BOSS_ERRORs map directly to BOSS_API_ERRORs.
	}

	vector<modEntry> matches;
	//Build modlist. Not using boss::ItemList::Load() as that builds to a vector<item> in load order, which isn't necessary.
	//Famous last words! Both are necessary in these dark times...
	ItemList modlist;
	try {
		modlist.Load(data_path);  //I seem to recall this might need exception handling. Oh well.
	} catch (boss_error e) {
		return ReturnCode(e.getCode(), e.getString());  //BOSS_ERRORs map directly to BOSS_API_ERRORs.
	}
	boost::unordered_set<string> hashset;  //Holds installed mods for checking against masterlist
	boost::unordered_set<string>::iterator setPos;
	for (fs::directory_iterator itr(data_path); itr!=fs::directory_iterator(); ++itr) {
		const fs::path filename = itr->path().filename();
		const string ext = to_lower_copy(itr->path().extension().string());
		if (fs::is_regular_file(itr->status()) && (ext==".esp" || ext==".esm" || ext==".ghost"))	//Add file to hashset.
			hashset.insert(to_lower_copy(filename.string()));
	}

	//Now evaluate regular expressions.
	vector<Item> items = masterlist.Items();
	size_t max = items.size();
	for (size_t i=0; i<max; i++) {
		if (items[i].Type() == REGEX) {
			//First thing's first, make a copy of the entry, then remove it from the masterlist.
			Item regexItem = items[i];
			items.erase(items.begin()+i);
			//Now form a regex.
			boost::regex reg;
			try {
				reg = boost::regex(to_lower_copy(regexItem.Name())+"(.ghost)?", boost::regex::extended|boost::regex::icase);  //Ghost extension is added so ghosted mods will also be found.
			} catch (boost::regex_error e) {
				return ReturnCode(BOSS_API_ERROR_REGEX_EVAL_FAIL, to_lower_copy(regexItem.Name())+"(.ghost)?");
			}
			//Now start looking.
			setPos = hashset.begin();
			do {
				setPos = FindRegexMatch(hashset, reg, setPos);
				if (setPos == hashset.end())  //Exit if the mod hasn't been found.
					break;
				string mod = *setPos;
				//Look for mod in modlist. Replace with case-preserved mod name.
				size_t modlistPos = modlist.FindItem(mod);
				if (modlistPos != modlist.Items().size())
					mod = modlist.Items()[modlistPos].Name();
				//Now do the adding/removing.
				//Create new temporary item to hold current found mod.
				Item tempItem = Item(mod, MOD, regexItem.Messages());
				//Now insert it in the position of the regex mod.
				items.insert(items.begin()+i, tempItem);
				++setPos;
			} while (setPos != hashset.end());
		}
	}
	masterlist.Items(items);

	//Now set DB ItemList to function's ItemList.
	db->filteredMasterlist = masterlist;
	return ReturnCode(BOSS_API_OK);
}


//////////////////////////////////
// Masterlist Updating
//////////////////////////////////

// Checks if there is a masterlist at masterlistPath. If not,
// it downloads the latest masterlist for the given game to masterlistPath.
// If there is, it first compares online and local versions to see if an
// update is necessary.
BOSS_API uint32_t UpdateMasterlist(boss_db db, const uint8_t * masterlistPath) {
	if (db == NULL || masterlistPath == NULL)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	//Set globals again in case they've been changed.
	data_path = db->db_data_path;
	gl_current_game = db->db_game;

	//PATH SETTING
	fs::path masterlist_path = fs::path(reinterpret_cast<const char *>(masterlistPath));

	if (masterlist_path.empty())
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Masterlist path is empty.");

	bool connection = false;
	try {
		connection = CheckConnection();
	} catch (boss_error e) {
		return ReturnCode(BOSS_API_ERROR_NETWORK_FAIL, e.getString());
	}
	if (!connection)
		return ReturnCode(BOSS_API_ERROR_NO_INTERNET_CONNECTION);
	else {
		try {
			string localDate, remoteDate;
			uint32_t localRevision, remoteRevision;
			uiStruct ui;
			UpdateMasterlist(ui, localRevision, localDate, remoteRevision, remoteDate);
			if (localRevision == remoteRevision)
				return ReturnCode(BOSS_API_OK_NO_UPDATE_NECESSARY);
			else
				return ReturnCode(BOSS_API_OK);
		} catch (boss_error e) {
			return ReturnCode(BOSS_API_ERROR_NETWORK_FAIL, e.getString());
		}
	}
}


////////////////////////////////
// Plugin Sorting Functions
////////////////////////////////

//Returns which method BOSS is using for the load order.
BOSS_API uint32_t GetLoadOrderMethod(boss_db db, uint32_t *method) {
	if (db->db_game == BOSS_API_GAME_SKYRIM && Version(GetExeDllVersion(data_path.parent_path() / "TESV.exe")) >= Version("1.4.26.0"))
		return BOSS_API_LOMETHOD_TEXTFILE;
	else
		return BOSS_API_LOMETHOD_TIMESTAMP;
}

// Sorts the mods in the data path, using the masterlist at the masterlist path,
// specified when the db was loaded using Load. Outputs a list of plugins, pointed to
// by sortedPlugins, of length pointed to by listLength. lastRecPos points to the 
// position in the sortedPlugins list of the last plugin recognised by BOSS.
// If the trialOnly parameter is true, no plugins are actually redated.
// If trialOnly is false, then sortedPlugins, listLength and lastRecPos can be null
// pointers, in case you do not require the information. If one of them is null, the
// other two must also be null.
BOSS_API uint32_t SortMods(boss_db db, const bool trialOnly, uint8_t *** sortedPlugins, 
								size_t * listLength, 
								size_t * lastRecPos) {
	if (db == NULL || (trialOnly == true && (sortedPlugins == NULL || listLength == NULL || lastRecPos == NULL)))
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	//Set globals again in case they've been changed.
	data_path = db->db_data_path;
	gl_current_game = db->db_game;

	ItemList modlist;
	time_t masterTime, modfiletime = 0;

	try {
		masterTime = GetMasterTime();
	} catch (boss_error e) {
		return ReturnCode(e.getCode(), e.getString());  //BOSS_ERRORs map directly to BOSS_API_ERRORs.
	}

	//Build modlist and masterlist.
	try {
		modlist.Load(data_path);
	} catch (boss_error e) {
		return ReturnCode(e.getCode(), e.getString());  //BOSS_ERRORs map directly to BOSS_API_ERRORs.
	}

	//Set up working modlist.
	//The function below changes the input, so make copies.
	ItemList masterlist = db->filteredMasterlist;
	try {
		masterlist.EvalConditions();
	} catch (boss_error e) {
		return ReturnCode(e.getCode(), e.getString());
	}
	RuleList userlist = db->userlist;
	BuildWorkingModlist(modlist, masterlist, userlist);
	string dummy;
	ApplyUserRules(modlist, userlist, dummy);  //This needs to be done to get sensible ordering as the userlist has been taken into account in the working modlist.

	//Initialise vars.
	if (listLength != NULL)
		*listLength = 0;
	if (sortedPlugins != NULL)
		*sortedPlugins = NULL;
	if (lastRecPos != NULL)
		*lastRecPos = 0;

	//Free memory if already used.
	if (db->extStringArray != NULL) {
		for (size_t i=0; i<db->extStringArraySize; i++)
			free(db->extStringArray[i]);  //Clear all the uint8_t strings created.
		free(db->extStringArray);  //Clear the string array.
	}

	vector<Item> items = modlist.Items();
	vector<uint8_t *> mods;
	size_t max = items.size();
	bool isSkyrim1426 = (gl_current_game == SKYRIM && Version(GetExeDllVersion(data_path.parent_path() / "TESV.exe")) >= Version("1.4.26.0"));
	for (size_t i=0; i < max; i++) {
		if (items[i].Type() == MOD && items[i].Exists()) {  //Only act on mods that exist.
			if (!trialOnly && !items[i].IsMasterFile() && !isSkyrim1426) {
				try {
					items[i].SetModTime(masterTime + mods.size()*60);  //time_t is an integer number of seconds, so adding 60 on increases it by a minute.
				} catch(boss_error e) {
					return ReturnCode(BOSS_API_ERROR_MOD_TIME_WRITE_FAIL, items[i].Name());
				}
			}
			uint8_t * p = StringToUint8_tString(items[i].Name());
			if (p == NULL)
				return ReturnCode(BOSS_API_ERROR_NO_MEM, "Memory allocation failed.");
			mods.push_back(p);
		}
		if (i == modlist.LastRecognisedPos() && lastRecPos != NULL)
			*lastRecPos = mods.size()-1;
	}

	//Skyrim >= 1.4.26 load order setting.
	if (!trialOnly && isSkyrim1426) {
		try {
			modlist.SavePluginNames(loadorder_path(), false, false);
			modlist.SavePluginNames(plugins_path(), true, true);
		} catch (boss_error e) {
			return ReturnCode(e.getCode(), e.getString());  //BOSS_ERRORs map directly to BOSS_API_ERRORs.
		}
	}

	//Now create external array.
	db->extStringArraySize = mods.size();
	db->extStringArray = (uint8_t**)malloc(db->extStringArraySize * sizeof(uint8_t*));
	if (db->extStringArray == NULL)
		return ReturnCode(BOSS_API_ERROR_NO_MEM, "Memory allocation failed.");
	for (size_t i=0; i < db->extStringArraySize; i++)
		db->extStringArray[i] = mods[i];
	
	if (sortedPlugins != NULL)
		*sortedPlugins = db->extStringArray;
	if (listLength != NULL)
		*listLength = db->extStringArraySize;

	return ReturnCode(BOSS_API_OK);
}

// Gets a list of plugins in load order, with the number of plugins given by numPlugins.
BOSS_API uint32_t GetLoadOrder(boss_db db, uint8_t *** plugins, size_t * numPlugins) {
	if (db == NULL || plugins == NULL || numPlugins == NULL)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	//Set globals again in case they've been changed.
	data_path = db->db_data_path;
	gl_current_game = db->db_game;

	//Initialise vars.
	*numPlugins = 0;
	*plugins = NULL;

	//Free memory if already used.
	if (db->extStringArray != NULL) {
		for (size_t i=0; i<db->extStringArraySize; i++)
			free(db->extStringArray[i]);  //Clear all the uint8_t strings created.
		free(db->extStringArray);  //Clear the string array.
	}

	ItemList loadorder;
	try {
		loadorder.Load(data_path);
	} catch (boss_error e) {
		return ReturnCode(e.getCode(), e.getString());  //BOSS_ERRORs map directly to BOSS_API_ERRORs.
	}
	vector<Item> items = loadorder.Items();

	//Allocate memory.
	db->extStringArraySize = items.size();
	db->extStringArray = (uint8_t**)malloc(db->extStringArraySize * sizeof(uint8_t*));
	if (db->extStringArray == NULL)
		return ReturnCode(BOSS_API_ERROR_NO_MEM, "Memory allocation failed.");

	//Set indices.
	for (size_t i=0; i < db->extStringArraySize; i++) {
		db->extStringArray[i] = StringToUint8_tString(items[i].Name());
		if (db->extStringArray[i] == NULL)
			return ReturnCode(BOSS_API_ERROR_NO_MEM, "Memory allocation failed.");
	}
	
	//Set outputs.
	*plugins = db->extStringArray;
	*numPlugins = db->extStringArraySize;

	return ReturnCode(BOSS_API_OK);
}

// Sets the load order to the given plugins list of length numPlugins.
BOSS_API uint32_t SetLoadOrder(boss_db db, uint8_t ** plugins, const size_t numPlugins) {
	if (db == NULL || plugins == NULL)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	//Set globals again in case they've been changed.
	data_path = db->db_data_path;
	gl_current_game = db->db_game;

	if (gl_current_game == SKYRIM && Version(GetExeDllVersion(data_path.parent_path() / "TESV.exe")) >= Version("1.4.26.0")) { //Skyrim.
		//Create a vector to hold the new loadorder.
		ItemList loadorder;
		size_t loSize = loadorder.Items().size();

		//We need to loop through the plugin array given and enter each plugin into the vector.
		for (size_t i=0; i < numPlugins; i++) {
			loadorder.Insert(loSize, Item(string(reinterpret_cast<const char *>(plugins[i]))));
			loSize++;
		}

		//Now iterate through the Data directory, adding any plugins to loadorder that aren't already in it.
		for (fs::directory_iterator itr(data_path); itr!=fs::directory_iterator(); ++itr) {
			const fs::path filename = itr->path().filename();
			const string ext = boost::algorithm::to_lower_copy(itr->path().extension().string());
			if (fs::is_regular_file(itr->status()) && (ext==".esp" || ext==".esm" || ext==".ghost")) {
				LOG_TRACE("-- Found mod: '%s'", filename.string().c_str());
				//Add file to modlist. If the filename has a '.ghost' extension, remove it.
				Item tempItem;
				if (ext == ".ghost")
					tempItem = Item(filename.stem().string());
				else
					tempItem = Item(filename.string());
				if (loadorder.FindItem(filename.string()) == loSize) {  //If the plugin is not present, add it.
					loadorder.Insert(loSize, tempItem);
					loSize++;
				}
			}
		}

		//Now save the new loadorder. Also update the plugins.txt.
		try {
			loadorder.SavePluginNames(loadorder_path(), false, false);
			loadorder.SavePluginNames(plugins_path(), true, true);
		} catch (boss_error e) {
			return ReturnCode(e.getCode(), e.getString());  //BOSS_ERRORs map directly to BOSS_API_ERRORs.
		}
	} else {  //Non-skyrim.
		//Get the master time to derive dates from.
		time_t masterTime;
		try {
			masterTime = GetMasterTime();
		} catch (boss_error e) {
			return ReturnCode(e.getCode(), e.getString());
		}

		//Loop through given array and set the modification time for each one.
		for (size_t i=0; i < numPlugins; i++) {
			try {
				Item(string(reinterpret_cast<const char *>(plugins[i]))).SetModTime(masterTime + i*60);  //time_t is an integer number of seconds, so adding 60 on increases it by a minute.
			} catch(boss_error e) {
				return ReturnCode(e.getCode(), string(reinterpret_cast<const char *>(plugins[i])));
			}
		}
	}

	return ReturnCode(BOSS_API_OK);
}

// Returns the contents of plugins.txt.
BOSS_API uint32_t GetActivePlugins(boss_db db, uint8_t *** plugins, size_t * numPlugins) {
	if (db == NULL || plugins == NULL || numPlugins == NULL)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	//Set globals again in case they've been changed.
	data_path = db->db_data_path;
	gl_current_game = db->db_game;

	//Initialise vars.
	*numPlugins = 0;
	*plugins = NULL;

	//Free memory if already used.
	if (db->extStringArray != NULL) {
		for (size_t i=0; i<db->extStringArraySize; i++)
			free(db->extStringArray[i]);  //Clear all the uint8_t strings created.
		free(db->extStringArray);  //Clear the string array.
	}

	//Load plugins.txt.
	ItemList pluginsTxt;
	try {
		pluginsTxt.Load(plugins_path());
	} catch (boss_error e) {
		return ReturnCode(e.getCode(), e.getString());  //BOSS_ERRORs map directly to BOSS_API_ERRORs.
	}
	vector<Item> items = pluginsTxt.Items();
	
	//Allocate memory.
	db->extStringArraySize = items.size();
	//If textlist-based, we want to also output Skyrim.esm, Update.esm.
	//First make sure they are currently missing though.
	if (gl_current_game == SKYRIM && Version(GetExeDllVersion(data_path.parent_path() / "TESV.exe")) >= Version("1.4.26.0")) { //Skyrim.
		if (pluginsTxt.FindItem("Skyrim.esm") == items.size() && pluginsTxt.FindItem("Update.esm") == items.size()) {
			db->extStringArraySize += 2;
			items.insert(items.begin(), Item("Update.esm"));  //Adding in reverse order.
			items.insert(items.begin(), Item("Skyrim.esm"));
		} else if (pluginsTxt.FindItem("Skyrim.esm") == items.size()) {
			db->extStringArraySize++;
			items.insert(items.begin(), Item("Skyrim.esm"));
		} else if (pluginsTxt.FindItem("Update.esm") == items.size()) {
			db->extStringArraySize++;
			items.insert(items.begin()+1, Item("Update.esm"));
		}
	}
	
	db->extStringArray = (uint8_t**)malloc(db->extStringArraySize * sizeof(uint8_t*));
	if (db->extStringArray == NULL)
		return ReturnCode(BOSS_API_ERROR_NO_MEM, "Memory allocation failed.");

	//Set indices.
	for (size_t i=0; i < db->extStringArraySize; i++) {
		db->extStringArray[i] = StringToUint8_tString(items[i].Name());
		if (db->extStringArray[i] == NULL)
			return ReturnCode(BOSS_API_ERROR_NO_MEM, "Memory allocation failed.");
	}
	
	//Set outputs.
	*plugins = db->extStringArray;
	*numPlugins = db->extStringArraySize;

	return ReturnCode(BOSS_API_OK);
}

// Edits plugins.txt so that it lists the given plugins in load order.
// Encoding is handled by the saving code and doesn't need to be explicitly catered for here.
BOSS_API uint32_t SetActivePlugins(boss_db db, uint8_t ** plugins, const size_t numPlugins) {
	if (db == NULL || plugins == NULL)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	if (numPlugins > 255)
		return ReturnCode(BOSS_API_ERROR_PLUGINS_FULL);

	//Set globals again in case they've been changed.
	data_path = db->db_data_path;
	gl_current_game = db->db_game;

	//Open up plugins.txt.
	ofstream file(plugins_path().c_str(), ios_base::trunc);
	if (file.fail())
		return ReturnCode(BOSS_API_ERROR_FILE_WRITE_FAIL, plugins_path().string());

	//Write the plugins to it.
	Transcoder trans;
	trans.SetEncoding(1252);
	string badFilename;
	string plugin;
	for (size_t i=0; i < numPlugins; i++) {
		try {
			plugin = trans.Utf8ToEnc(string(reinterpret_cast<const char *>(plugins[i])));
		} catch (boss_error e) {
			badFilename = string(reinterpret_cast<const char *>(plugins[i]));
		}
		if (gl_current_game == SKYRIM && Version(GetExeDllVersion(data_path.parent_path() / "TESV.exe")) >= Version("1.4.26.0") && (plugin == "Skyrim.esm" || plugin == "Update.esm"))  //These don't go in plugins.txt.
			continue;
		file << plugin << endl;
	}
	file.close();

	//Now if running for textfile-based load order system, reorder plugins.txt, deriving the order from loadorder.txt.
	if (gl_current_game == SKYRIM && Version(GetExeDllVersion(data_path.parent_path() / "TESV.exe")) >= Version("1.4.26.0")) { //Skyrim.

		//Now get the load order from loadorder.txt.
		ItemList loadorder;
		try {
			loadorder.Load(loadorder_path());
		} catch (boss_error e) {
			return ReturnCode(e.getCode(), e.getString());  //BOSS_ERRORs map directly to BOSS_API_ERRORs.
		}
		size_t loSize = loadorder.Items().size();

		for (size_t i=0; i < numPlugins; i++) {
			file << reinterpret_cast<const char *>(plugins[i]) << endl;
			if (loadorder.FindItem(string(reinterpret_cast<const char *>(plugins[i]))) == loSize) {  //File does not have a defined load order. Add it to the end of loadorder.
				loadorder.Insert(loSize, Item(string(reinterpret_cast<const char *>(plugins[i]))));
				loSize++;
			}
		}

		//Save the load order and derive plugins.txt order from it.
		try {
			loadorder.SavePluginNames(loadorder_path(), false, false);
			loadorder.SavePluginNames(plugins_path(), true, true);
		} catch (boss_error e) {
			return ReturnCode(e.getCode(), e.getString());
		}
	}

	if (!badFilename.empty())
		return ReturnCode(BOSS_API_WARN_BAD_FILENAME, badFilename);
	else
		return ReturnCode(BOSS_API_OK);
}

// Gets the load order of the specified plugin, giving it as index. The first position 
// in the load order is 0.
BOSS_API uint32_t GetPluginLoadOrder(boss_db db, const uint8_t * plugin, size_t * index) {
	if (db == NULL || plugin == NULL || index == NULL)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	//Set globals again in case they've been changed.
	data_path = db->db_data_path;
	gl_current_game = db->db_game;

	//Initialise vars.
	*index = 0;

	//Now get the load order.
	ItemList loadorder;
	try {
		loadorder.Load(data_path);
	} catch (boss_error e) {
		return ReturnCode(e.getCode(), e.getString());  //BOSS_ERRORs map directly to BOSS_API_ERRORs.
	}

	//Now search for the given plugin.
	size_t pos = loadorder.FindItem(string(reinterpret_cast<const char *>(plugin)));
	if (pos == loadorder.Items().size())
		return ReturnCode(BOSS_API_ERROR_FILE_NOT_FOUND);

	//Set output.
	*index = pos;
	
	return ReturnCode(BOSS_API_OK);
}

// Sets the load order of the specified plugin, removing it from its current position 
// if it has one. The first position in the load order is 0. If the index specified is
//greater than the number of plugins in the load order, the plugin will be inserted at
//the end of the load order.
BOSS_API uint32_t SetPluginLoadOrder(boss_db db, const uint8_t * plugin, size_t index) {
	if (db == NULL || plugin == NULL)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	//Set globals again in case they've been changed.
	data_path = db->db_data_path;
	gl_current_game = db->db_game;

	//Now get the load order from loadorder.txt.
	ItemList loadorder;
	try {
		loadorder.Load(data_path);
	} catch (boss_error e) {
		return ReturnCode(e.getCode(), e.getString());  //BOSS_ERRORs map directly to BOSS_API_ERRORs.
	}

	//Now search for the given plugin.
	size_t pos = loadorder.FindItem(string(reinterpret_cast<const char *>(plugin)));
	if (pos != loadorder.Items().size())  //Plugin found. Erase it.
		loadorder.Erase(pos);

	//Now insert the plugin into its new position.
	if (index >= loadorder.Items().size())
		index = loadorder.Items().size()-1;
	loadorder.Insert(index, Item(string(reinterpret_cast<const char *>(plugin))));


	if (gl_current_game == SKYRIM && Version(GetExeDllVersion(data_path.parent_path() / "TESV.exe")) >= Version("1.4.26.0")) { //Skyrim.
		//Now write out the new loadorder.txt. Also update the plugins.txt.
		try {
			loadorder.SavePluginNames(loadorder_path(), false, false);
			loadorder.SavePluginNames(plugins_path(), true, true);
		} catch (boss_error e) {
			return ReturnCode(e.getCode(), e.getString());
		}
	} else {  //Non-skyrim. Scan data directory, and arrange plugins found in timestamp load order.
		//Get the master time to derive dates from.
		time_t masterTime;
		try {
			masterTime = GetMasterTime();
		} catch (boss_error e) {
			return ReturnCode(BOSS_API_ERROR_MASTER_TIME_READ_FAIL, e.getString());
		}

		//Now set the new timestamps.
		vector<Item> items = loadorder.Items();
		size_t max = items.size();
		for (size_t i=0; i < max; i++) {
			try {
				items[i].SetModTime(masterTime + i*60);  //time_t is an integer number of seconds, so adding 60 on increases it by a minute.
			} catch(boss_error e) {
				return ReturnCode(BOSS_API_ERROR_MOD_TIME_WRITE_FAIL, items[i].Name());
			}
		}
	}

	return ReturnCode(BOSS_API_OK);
}

// Gets what plugin is at the specified load order position.
BOSS_API uint32_t GetIndexedPlugin(boss_db db, const size_t index, uint8_t ** plugin) {
	if (db == NULL || plugin == NULL)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	//Set globals again in case they've been changed.
	data_path = db->db_data_path;
	gl_current_game = db->db_game;

	//Initialise vars.
	*plugin = NULL;

	//Free memory if already used.
	free(db->extString);

	//Now get the load order.
	ItemList loadorder;
	try {
		loadorder.Load(data_path);
	} catch (boss_error e) {
		return ReturnCode(e.getCode(), e.getString());  //BOSS_ERRORs map directly to BOSS_API_ERRORs.
	}

	//Check that the index is within bounds.
	if (index >= loadorder.Items().size())
		return ReturnCode(BOSS_API_ERROR_FILE_NOT_FOUND);

	//Allocate memory.
	db->extString = StringToUint8_tString(loadorder.Items()[index].Name());
	if (db->extString == NULL)
		return ReturnCode(BOSS_API_ERROR_NO_MEM, "Memory allocation failed.");
	
	//Set outputs.
	*plugin = db->extString;

	return ReturnCode(BOSS_API_OK);
}

/* If (active), adds the plugin to plugins.txt in its load order if it is not already present.
 If (!active), removes the plugin from plugins.txt if it is present. 
 Encoding is handled by the saving code and doesn't need to be explicitly catered for here.*/
BOSS_API uint32_t SetPluginActive(boss_db db, const uint8_t * plugin, const bool active) {
	if (db == NULL || plugin == NULL)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	//Set globals again in case they've been changed.
	data_path = db->db_data_path;
	gl_current_game = db->db_game;

	//Load plugins.txt.
	ItemList pluginsList;
	try {
		pluginsList.Load(plugins_path());
	} catch (boss_error e) {
		return ReturnCode(e.getCode(), e.getString());  //BOSS_ERRORs map directly to BOSS_API_ERRORs.
	}

	string pluginStr = string(reinterpret_cast<const char *>(plugin));

	//Check if the given plugin is in plugins.txt.
	if (pluginsList.FindItem(pluginStr) != pluginsList.Items().size() && !active) //Exists, but shouldn't.
		pluginsList.Erase(pluginsList.FindItem(pluginStr));
	else if (pluginsList.FindItem(pluginStr) == pluginsList.Items().size() && active)  //Doesn't exist, but should.
		pluginsList.Insert(pluginsList.Items().size(), pluginStr);

	//Check that there aren't too many plugins in plugins.txt.
	if (pluginsList.Items().size() > 255)
		return ReturnCode(BOSS_API_ERROR_PLUGINS_FULL);
	else if (gl_current_game == SKYRIM && Version(GetExeDllVersion(data_path.parent_path() / "TESV.exe")) >= Version("1.4.26.0") && pluginsList.Items().size() > 253)  //textfile-based system doesn't list Skyrim.esm, Update.esm in plugins.txt.
		return ReturnCode(BOSS_API_ERROR_PLUGINS_FULL);

	//Now save the change.
	try {
		pluginsList.SavePluginNames(plugins_path(), false, true);  //Must be false because we're not adding a currently active file, if we're adding something.
	} catch (boss_error e) {
		return ReturnCode(e.getCode(), e.getString());
	}

	return ReturnCode(BOSS_API_OK);
}

// Checks to see if the given plugin is listed in plugins.txt.
BOSS_API uint32_t IsPluginActive(boss_db db, const uint8_t * plugin, bool * isActive) {
	if (db == NULL || plugin == NULL || isActive == NULL)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	//Set globals again in case they've been changed.
	data_path = db->db_data_path;
	gl_current_game = db->db_game;

	string pluginStr = string(reinterpret_cast<const char *>(plugin));

	//Check if it's textlist-based, and Skyrim.esm/Update.esm, which are special cases.
	if (gl_current_game == SKYRIM && Version(GetExeDllVersion(data_path.parent_path() / "TESV.exe")) >= Version("1.4.26.0")) { //Skyrim.
		if (pluginStr == "Skyrim.esm" || pluginStr == "Update.esm") {
			*isActive = true;
			return ReturnCode(BOSS_API_OK);
		}
	}

	//Load plugins.txt. A hashset would be more efficient.
	ItemList pluginsList;
	try {
		pluginsList.Load(plugins_path());
	} catch (boss_error e) {
		return ReturnCode(e.getCode(), e.getString());  //BOSS_ERRORs map directly to BOSS_API_ERRORs.
	}
	
	//Check if the given plugin is in plugins.txt.
	if (pluginsList.FindItem(pluginStr) != pluginsList.Items().size())
		*isActive = true;
	else
		*isActive = false;

	return ReturnCode(BOSS_API_OK);
}



//////////////////////////
// DB Access Functions
//////////////////////////

// Returns an array of the Bash Tags encounterred when loading the masterlist
// and userlist, and the number of tags in the returned array. The array and
// its contents are static and should not be freed by the client.
BOSS_API uint32_t GetBashTagMap (boss_db db, BashTag ** tagMap, size_t * numTags) {
	if (db == NULL || tagMap == NULL || numTags == NULL)  //Check for valid args.
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	//Set globals again in case they've been changed.
	data_path = db->db_data_path;
	gl_current_game = db->db_game;

	if (db->extTagMap != NULL && !db->bashTagMap.empty()) {  //Check to see if bashTagMap is already populated.
		*numTags = db->bashTagMap.size();  //Set size.
		*tagMap = db->extTagMap;
	} else {
		//Need to build internal Bash Tag map then feed it to the outside world.
		//This involves iterating through all mods to get the tags they add and remove, in the masterlist and userlist.
		//Off we go!
		boost::unordered_set<string> tagsAdded, tagsRemoved;  //These are just so that we can use the general function, and will be combined later.

		vector<Item> items = db->filteredMasterlist.Items();
		size_t imax = items.size();
		for (size_t i=0; i < imax; i++) {
			if (items[i].Messages().empty())
				continue;
			vector<Message> messages = items[i].Messages();
			size_t jmax = messages.size();
			for (size_t j=0; j < jmax; j++) {
				if (messages[j].Key() == TAG)
					GetBashTagsFromString(messages[j].Data(), tagsAdded, tagsRemoved);
			}
		}
		vector<Rule> rules = db->userlist.Rules();
		imax = rules.size();
		for (size_t i=0; i < imax; i++) {
			vector<RuleLine> lines = rules[i].Lines();
			size_t jmax = lines.size();
			for (size_t j=0; j < jmax; j++) {
				if (lines[j].ObjectMessageKey() == TAG) {
					GetBashTagsFromString(lines[j].Object(), tagsAdded, tagsRemoved);
				}
			}
		}
		//Now tagsAdded and tagsRemoved each contain a list of unique tag strings.
		//Time to combine. :D
		uint32_t UID = 0;
		boost::unordered_set<string>::iterator tagStringIter;
		for (tagStringIter = tagsAdded.begin(); tagStringIter != tagsAdded.end(); ++tagStringIter) {
			if (db->FindBashTag(*tagStringIter) == db->bashTagMap.end())	{						//Tag not found in bashTagMap. Add it!
				db->bashTagMap.insert(pair<uint32_t,string>(UID,*tagStringIter));
				UID++;  //Now increment UID to keep it U.
			}
		}
		for (tagStringIter = tagsRemoved.begin(); tagStringIter != tagsRemoved.end(); ++tagStringIter) {
			if (db->FindBashTag(*tagStringIter) == db->bashTagMap.end())	{						//Tag not found in bashTagMap. Add it!
				db->bashTagMap.insert(pair<uint32_t,string>(UID,*tagStringIter));
				UID++;  //Now increment UID to keep it U.
			}
		}

		//Now to convert for the outside world.
		size_t mapSize = db->bashTagMap.size();  //Set size.

		//Allocate memory.
		db->extTagMap = (BashTag*)calloc(mapSize, sizeof(BashTag));
		if (db->extTagMap == NULL)
			return ReturnCode(BOSS_API_ERROR_NO_MEM, "Memory allocation failed.");

		//Loop through internal bashTagMap and fill output elements.
		for (size_t i=0; i<mapSize; i++) {
			db->extTagMap[i].id = uint32_t(i);
			db->extTagMap[i].name = reinterpret_cast<const uint8_t *>(db->bashTagMap[i].c_str());
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
BOSS_API uint32_t GetModBashTags (boss_db db, const uint8_t * plugin, 
									uint32_t ** tagIds_added, 
									size_t * numTags_added, 
									uint32_t **tagIds_removed, 
									size_t *numTags_removed,
									bool * userlistModified) {
	//Check for valid args.
	if (db == NULL || plugin == NULL || userlistModified == NULL || numTags_added == NULL || numTags_removed == NULL || tagIds_removed == NULL || tagIds_added == NULL)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");
							
	//Convert modName.
	string mod(reinterpret_cast<const char *>(plugin));

	if (mod.empty())
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Plugin name is empty.");

	if (db->extTagMap == NULL)
		return ReturnCode(BOSS_API_ERROR_NO_TAG_MAP);

	//Set globals again in case they've been changed.
	data_path = db->db_data_path;
	gl_current_game = db->db_game;

	//Bash Tag temporary internal holders.
	boost::unordered_set<string> tagsAdded, tagsRemoved;

	//Initialise pointers to null and zero tag counts.
	*numTags_added = 0;
	*numTags_removed = 0;
	*tagIds_removed = NULL;
	*tagIds_added = NULL;
	*userlistModified = false;

	//Now search filtered masterlist for mod.
	size_t pos = db->filteredMasterlist.FindItem(mod);
	if (pos != db->filteredMasterlist.Items().size()) {
		vector<Message> messages = db->filteredMasterlist.Items()[pos].Messages();
		for (vector<Message>::iterator messageIter = messages.begin(); messageIter != messages.end(); ++messageIter) {
			if (messageIter->Key() == TAG)
				GetBashTagsFromString(messageIter->Data(), tagsAdded, tagsRemoved);
		}
	}

	//Now search userlist for mod.
	pos = db->userlist.FindRule(mod, true);
	if (pos != db->userlist.Rules().size()) {
		vector<RuleLine> lines = db->userlist.Rules()[pos].Lines();
		for (vector<RuleLine>::iterator lineIter = lines.begin(); lineIter != lines.end(); ++lineIter) {
			if (lineIter->Key() == REPLACE && (!tagsAdded.empty() || !tagsRemoved.empty())) {
				tagsAdded.clear();
				tagsRemoved.clear();
				*userlistModified = true;
			}
			if (lineIter->ObjectMessageKey() == TAG) {
				GetBashTagsFromString(lineIter->Object(), tagsAdded, tagsRemoved);
				*userlistModified = true;
			}
		}
	}

	//Now we convert strings to UIDs.
	vector<uint32_t> tagsAddedUIDs, tagsRemovedUIDs;
	boost::unordered_set<string>::iterator tagStringIter;
	for (tagStringIter = tagsAdded.begin(); tagStringIter != tagsAdded.end(); ++tagStringIter) {
		map<uint32_t,string>::iterator mapPos = db->FindBashTag(*tagStringIter);
		if (mapPos != db->bashTagMap.end())							//Tag found in bashTagMap. Get the UID.
			tagsAddedUIDs.push_back(mapPos->first);
	}
	for (tagStringIter = tagsRemoved.begin(); tagStringIter != tagsRemoved.end(); ++tagStringIter) {
		map<uint32_t,string>::iterator mapPos = db->FindBashTag(*tagStringIter);
		if (mapPos != db->bashTagMap.end())							//Tag found in bashTagMap. Get the UID.
			tagsRemovedUIDs.push_back(mapPos->first);
	}
	//Now set the sizes (we kept the two separate just in case some tags didn't have UIDs, which would change the size of the outputted array.
	size_t numAdded = tagsAddedUIDs.size();
	size_t numRemoved = tagsRemovedUIDs.size();

	//Free memory.
	free(db->extAddedTagIds);
	free(db->extRemovedTagIds);
	
	//Allocate memory.
	db->extAddedTagIds = (uint32_t*) malloc(numAdded * sizeof(uint32_t));
	if (db->extAddedTagIds == NULL)
		return ReturnCode(BOSS_API_ERROR_NO_MEM, "Memory allocation failed.");
	for (size_t i=0; i < numAdded; i++)
		db->extAddedTagIds[i] = tagsAddedUIDs[i];

	db->extRemovedTagIds = (uint32_t*) malloc(numRemoved * sizeof(uint32_t));
	if (db->extRemovedTagIds == NULL)
		return ReturnCode(BOSS_API_ERROR_NO_MEM, "Memory allocation failed.");
	for (size_t i=0; i < numRemoved; i++)
		db->extRemovedTagIds[i] = tagsRemovedUIDs[i];

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
BOSS_API uint32_t GetDirtyMessage (boss_db db, const uint8_t * plugin, 
									uint8_t ** message, uint32_t * needsCleaning) {
	//Check for valid args.
	if (db == NULL || plugin == NULL || message == NULL || needsCleaning == NULL)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");
									
	//Convert modName.
	string mod(reinterpret_cast<const char *>(plugin));

	if (mod.empty())
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Plugin name is empty.");

	//Set globals again in case they've been changed.
	data_path = db->db_data_path;
	gl_current_game = db->db_game;

	//Initialise pointers.
	*message = NULL;
	*needsCleaning = BOSS_API_CLEAN_UNKNOWN;

	//Now search filtered masterlist for mod.
	size_t pos = db->filteredMasterlist.FindItem(mod);
	if (pos != db->filteredMasterlist.Items().size()) {
		vector<Message> messages = db->filteredMasterlist.Items()[pos].Messages();
		for (vector<Message>::iterator messageIter = messages.begin(); messageIter != messages.end(); ++messageIter) {
			if (messageIter->Key() == DIRTY) {
				db->extString = StringToUint8_tString(messageIter->Data());
				if (db->extString == NULL)
					return ReturnCode(BOSS_API_ERROR_NO_MEM, "Memory allocation failed.");
				 *message = db->extString;

				if (messageIter->Data().find("Do not clean.") != string::npos)  //Mod should not be cleaned.
					*needsCleaning = BOSS_API_CLEAN_NO;
				else  //Mod should be cleaned.
					*needsCleaning = BOSS_API_CLEAN_YES;
				break;
			}
		}
	}

	return ReturnCode(BOSS_API_OK);
}

// Writes a minimal masterlist that only contains mods that have Bash Tag suggestions, 
// and/or dirty messages, plus the Tag suggestions and/or messages themselves, in order 
// to create the Wrye Bash taglist. outputFile is the path to use for output. If 
// outputFile already exists, it will only be overwritten if overwrite is true.
BOSS_API uint32_t DumpMinimal (boss_db db, const uint8_t * outputFile, const bool overwrite) {
	//Check for valid args.
	if (db == NULL || outputFile == NULL)
		return ReturnCode(BOSS_API_ERROR_INVALID_ARGS, "Null pointer passed.");

	//Set globals again in case they've been changed.
	data_path = db->db_data_path;
	gl_current_game = db->db_game;

	string path(reinterpret_cast<const char *>(outputFile));
	if (!fs::exists(path) || overwrite) {
		ofstream mlist(path.c_str());
		if (mlist.fail())
			return ReturnCode(BOSS_API_ERROR_FILE_WRITE_FAIL, path);
		else {
			//Iterate through items, printing out all relevant info.
			vector<Item> items = db->rawMasterlist.Items();  //Filtered works, but not raw.
			for (vector<Item>::iterator itemIter = items.begin(); itemIter != items.end(); ++itemIter) {
				if (itemIter->Type() == MOD || itemIter->Type() == REGEX) {
					bool namePrinted = false;
					vector<Message> messages = itemIter->Messages();
					for (vector<Message>::iterator messageIter = messages.begin(); messageIter != messages.end(); ++messageIter) {
						if (messageIter->Key() == TAG || messageIter->Key() == DIRTY) {
							if (!namePrinted) {
								if (itemIter->Type() == REGEX)
									mlist << "REGEX: ";
								mlist << itemIter->Name() << endl;  //Print the mod name.
							}
							mlist << " " << messageIter->KeyToString() << ": " << messageIter->Data() << endl;
						}
					}
				}
			}
			mlist.close();
		}
		return ReturnCode(BOSS_API_OK);
	} else
		return ReturnCode(BOSS_API_ERROR_FILE_WRITE_FAIL, path);
}