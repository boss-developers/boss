/*	Better Oblivion Sorting Software

	A "one-click" program for users that quickly optimises and avoids 
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge, 
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

    Copyright (C) 2011    BOSS Development Team.

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
	ItemList rawMasterlist, filteredMasterlist;
	RuleList userlist;
	map<uint32_t,string> bashTagMap;				//A hashmap containing all the Bash Tag strings found in the masterlist and userlist and their unique IDs.
													//Ordered to make ensuring UIDs easy (check the UID of the last element then increment). Strings are case-preserved.
	//Externally-visible data pointers storage.
	BashTag * extTagMap;				//Holds the pointer for the bashTagMap returned by GetBashTagMap().
	uint32_t * extAddedTagIds;
	uint32_t * extRemovedTagIds;
	uint8_t * extMessage;
	uint8_t ** extPluginList;

	//Pointer array sizes.
	size_t extPluginListSize;

	//Constructor
	_boss_db_int() {
		extTagMap = NULL;
		extAddedTagIds = NULL;
		extRemovedTagIds = NULL;
		extMessage = NULL;
		extPluginList = NULL;
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

// The following are the possible error codes that the API can return.
// Taken from BOSS-Common's Error.h and extended.
BOSS_API const uint32_t BOSS_API_ERROR_OK						=	BOSS_ERROR_OK;
BOSS_API const uint32_t BOSS_API_ERROR_FILE_WRITE_FAIL			=	BOSS_ERROR_FILE_WRITE_FAIL;
BOSS_API const uint32_t BOSS_API_ERROR_FILE_NOT_UTF8			=	BOSS_ERROR_FILE_NOT_UTF8;
BOSS_API const uint32_t BOSS_API_ERROR_FILE_NOT_FOUND			=	BOSS_ERROR_FILE_NOT_FOUND;
BOSS_API const uint32_t BOSS_API_ERROR_MASTER_TIME_READ_FAIL	=	BOSS_ERROR_FS_FILE_MOD_TIME_READ_FAIL;
BOSS_API const uint32_t BOSS_API_ERROR_FILE_MOD_TIME_WRITE_FAIL	=	BOSS_ERROR_FS_FILE_MOD_TIME_WRITE_FAIL;
BOSS_API const uint32_t BOSS_API_ERROR_CONDITION_EVAL_FAIL		=	BOSS_ERROR_CONDITION_EVAL_FAIL;
BOSS_API const uint32_t BOSS_API_ERROR_PARSE_FAIL				= 	BOSS_ERROR_MAX + 1;
BOSS_API const uint32_t BOSS_API_ERROR_NO_MEM					=	BOSS_ERROR_MAX + 2;
BOSS_API const uint32_t BOSS_API_ERROR_OVERWRITE_FAIL			=	BOSS_ERROR_MAX + 3;
BOSS_API const uint32_t BOSS_API_ERROR_INVALID_ARGS				=	BOSS_ERROR_MAX + 4;
BOSS_API const uint32_t BOSS_API_ERROR_NETWORK_FAIL				=	BOSS_ERROR_MAX + 5;
BOSS_API const uint32_t BOSS_API_ERROR_NO_INTERNET_CONNECTION	=	BOSS_ERROR_MAX + 6;
BOSS_API const uint32_t BOSS_API_ERROR_NO_UPDATE_NECESSARY		=	BOSS_ERROR_MAX + 7;
BOSS_API const uint32_t BOSS_API_ERROR_NO_TAG_MAP				=	BOSS_ERROR_MAX + 8;
BOSS_API const uint32_t BOSS_API_ERROR_REGEX_EVAL_FAIL			=	BOSS_ERROR_MAX + 9;
BOSS_API const uint32_t BOSS_API_ERROR_MAX						=	BOSS_API_ERROR_REGEX_EVAL_FAIL;

// The following are the mod cleanliness states that the API can return.
BOSS_API const uint32_t BOSS_API_CLEAN_NO		= 0;
BOSS_API const uint32_t BOSS_API_CLEAN_YES		= 1;
BOSS_API const uint32_t BOSS_API_CLEAN_UNKNOWN	= 2;

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
	free(db->extMessage);

	if (db->extPluginList != NULL) {
		for (size_t i=0; i<db->extPluginListSize; i++)
			free(db->extPluginList[i]);  //Clear all the uint8_t strings created.
		free(db->extPluginList);  //Clear the string array.
	}
}

//////////////////////////////
// Version Functions
//////////////////////////////

// Returns whether this version of BOSS supports the API from the given 
// BOSS version. Abstracts BOSS API stability policy away from clients.
BOSS_API bool IsCompatibleVersion (const uint32_t bossVersionMajor, const uint32_t bossVersionMinor, const uint32_t bossVersionPatch) {
	//The 1.9 API is backwards compatible with all 1.x (x<=9) versions of BOSS, and forward compatible with all 1.9.x versions.
	if (bossVersionMajor <= 1 && bossVersionMajor <= 9)
		return true;
	else
		return false;
}

// Returns the version string for this version of BOSS.
// The string exists for the lifetime of the library.
BOSS_API uint32_t GetVersionString (const uint8_t ** bossVersionStr) {
	//Check for valid args.
	if (bossVersionStr == NULL)
		return BOSS_API_ERROR_INVALID_ARGS;

	*bossVersionStr = reinterpret_cast<const uint8_t *>(boss_version.c_str());
	return BOSS_API_ERROR_OK;
}


////////////////////////////////////
// Lifecycle Management Functions
////////////////////////////////////

// Explicitly manage database lifetime. Allows clients to free memory when
// they want/need to.
BOSS_API uint32_t CreateBossDb  (boss_db * db) {
	if (db == NULL)  //Check for valid args.
		return BOSS_API_ERROR_INVALID_ARGS;

	boss_db retVal = new _boss_db_int;
	if (retVal == NULL)
		return BOSS_API_ERROR_NO_MEM;
	*db = retVal;
	return BOSS_API_ERROR_OK;
}

BOSS_API void     DestroyBossDb (boss_db db) {
	if (db == NULL)
		return;

	//Free memory at pointers stored in structure.
	DestroyPointers(db);
	
	//Now delete DB.
	delete db;
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
									const uint8_t * userlistPath,
									const uint8_t * dataPath,
									const uint32_t clientGame) {
	ItemList masterlist;
	RuleList userlist;
	
	//Check for valid args.
	if ((clientGame != OBLIVION && clientGame != FALLOUT3 && clientGame != FALLOUTNV && clientGame != NEHRIM && clientGame != SKYRIM) 
		|| db == NULL || masterlistPath == NULL || userlistPath == NULL || dataPath == NULL)
		return BOSS_API_ERROR_INVALID_ARGS;

	//Set the locale to get encoding conversions working correctly.
	setlocale(LC_CTYPE, "");
	locale global_loc = locale();
	locale loc(global_loc, new boost::filesystem::detail::utf8_codecvt_facet());
	boost::filesystem::path::imbue(loc);
	
	//Set game.
	gl_current_game = clientGame;

	//PATH SETTING
	data_path = fs::path(reinterpret_cast<const char *>(dataPath));
	fs::path masterlist_path = fs::path(reinterpret_cast<const char *>(masterlistPath));
	fs::path userlist_path = fs::path(reinterpret_cast<const char *>(userlistPath));	

	if (data_path.empty() || masterlist_path.empty())
		return BOSS_API_ERROR_INVALID_ARGS;

	//PARSING - Masterlist
	try {
		masterlist.Load(masterlist_path);
	} catch (boss_error e) {
		if (e.getCode() == BOSS_ERROR_FILE_NOT_FOUND)
			return BOSS_API_ERROR_FILE_NOT_FOUND;
		else if (e.getCode() == BOSS_ERROR_FILE_NOT_UTF8)
			return BOSS_API_ERROR_FILE_NOT_UTF8;
		else
			return BOSS_API_ERROR_PARSE_FAIL;
	}

	//PARSING - Userlist
	if (!userlist_path.empty()) {
		try {
			userlist.Load(userlist_path);
		} catch (boss_error e) {
			db->userlist.rules.clear();  //If userlist has parsing errors, empty it so no rules are applied.
			if (e.getCode() == BOSS_ERROR_FILE_NOT_FOUND)
				return BOSS_API_ERROR_FILE_NOT_FOUND;
			else if (e.getCode() == BOSS_ERROR_FILE_NOT_UTF8)
				return BOSS_API_ERROR_FILE_NOT_UTF8;
			else
				return BOSS_API_ERROR_PARSE_FAIL;
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
	return BOSS_API_ERROR_OK;
}

// Re-evaluates all conditional lines and regex mods in rawMasterlist, 
// putting the output into filteredMasterlist.
BOSS_API uint32_t EvalConditionals(boss_db db, const uint8_t * dataPath) {
	//Check for valid args.
	if (db == NULL || dataPath == NULL)
		return BOSS_API_ERROR_INVALID_ARGS;
		
	//PATH SETTING
	data_path = fs::path(reinterpret_cast<const char *>(dataPath));

	if (data_path.empty())
		return BOSS_API_ERROR_INVALID_ARGS;

	//First re-evaluate conditionals.
	ItemList masterlist = db->rawMasterlist;
	try {
		masterlist.EvalConditions();
	} catch (boss_error e) {
		cout << e.getString() << endl;
		return BOSS_API_ERROR_CONDITION_EVAL_FAIL;
	}

	vector<modEntry> matches;
	//Build modlist. Not using boss::ItemList::Load() as that builds to a vector<item> in load order, which isn't necessary.
	//Famous last words! Both are necessary in these dark times...
	ItemList modlist;
	modlist.Load(data_path);  //I seem to recall this might need exception handling. Oh well.
	boost::unordered_set<string> hashset;  //Holds installed mods for checking against masterlist
	boost::unordered_set<string>::iterator setPos;
	for (fs::directory_iterator itr(data_path); itr!=fs::directory_iterator(); ++itr) {
		const fs::path filename = itr->path().filename();
		const string ext = to_lower_copy(itr->path().extension().string());
		if (fs::is_regular_file(itr->status()) && (ext==".esp" || ext==".esm" || ext==".ghost"))	//Add file to hashset.
			hashset.insert(Tidy(filename.string()));
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
				reg = boost::regex(Tidy(regexItem.Name())+"(.ghost)?", boost::regex::extended);  //Ghost extension is added so ghosted mods will also be found.
			} catch (boost::regex_error e) {
				return BOSS_API_ERROR_REGEX_EVAL_FAIL;
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
	return BOSS_API_ERROR_OK;
}


//////////////////////////////////
// Masterlist Updating
//////////////////////////////////

// Checks if there is a masterlist at masterlistPath. If not,
// it downloads the latest masterlist for the given game to masterlistPath.
// If there is, it first compares online and local versions to see if an
// update is necessary.
BOSS_API uint32_t UpdateMasterlist(const uint32_t clientGame, const uint8_t * masterlistPath) {
	if ((clientGame != OBLIVION && clientGame != FALLOUT3 && clientGame != FALLOUTNV && clientGame != NEHRIM && clientGame != SKYRIM) || masterlistPath == NULL)
		return BOSS_API_ERROR_INVALID_ARGS;

	gl_current_game = clientGame;

	//PATH SETTING
	fs::path masterlist_path = fs::path(reinterpret_cast<const char *>(masterlistPath));

	if (masterlist_path.empty())
		return BOSS_API_ERROR_INVALID_ARGS;

	bool connection = false;
	try {
		connection = CheckConnection();
	} catch (boss_error e) {
		return BOSS_API_ERROR_NETWORK_FAIL;
	}
	if (!connection)
		return BOSS_API_ERROR_NO_INTERNET_CONNECTION;
	else {
		try {
			string localDate, remoteDate;
			uint32_t localRevision, remoteRevision;
			uiStruct ui;
			UpdateMasterlist(ui, localRevision, localDate, remoteRevision, remoteDate);
			if (localRevision == remoteRevision)
				return BOSS_API_ERROR_NO_UPDATE_NECESSARY;
			else
				return BOSS_API_ERROR_OK;
		} catch (boss_error e) {
			return BOSS_API_ERROR_NETWORK_FAIL;
		}
	}
}


////////////////////////////////
// Plugin Sorting Functions
////////////////////////////////

// Sorts the mods in dataPath according to their order in the masterlist at 
// masterlistPath for the given game. lastRecPos holds the load order position 
// of the last plugin recognised by BOSS. 
BOSS_API uint32_t SortMods(boss_db db, size_t * lastRecPos) {
	if (db == NULL)
		return BOSS_API_ERROR_INVALID_ARGS;

	ItemList modlist;
	time_t masterTime, modfiletime = 0;

	try {
		masterTime = GetMasterTime();
	} catch (boss_error e) {
		return BOSS_API_ERROR_MASTER_TIME_READ_FAIL;
	}

	//Build modlist and masterlist.
	try {
		modlist.Load(data_path);
	} catch (boss_error e) {
		if (e.getCode() == BOSS_ERROR_FILE_NOT_FOUND)
			return BOSS_API_ERROR_FILE_NOT_FOUND;
		else if (e.getCode() == BOSS_ERROR_FILE_NOT_UTF8)
			return BOSS_API_ERROR_FILE_NOT_UTF8;
		else
			return BOSS_API_ERROR_PARSE_FAIL;
	}

	//Set up working modlist.
	//The function below changes the input, so make copies.
	ItemList masterlist = db->filteredMasterlist;
	masterlist.EvalConditions();
	RuleList userlist = db->userlist;
	BuildWorkingModlist(modlist, masterlist, userlist);
	string dummy;
	ApplyUserRules(modlist, userlist, dummy);  //This needs to be done to get sensible ordering as the userlist has been taken into account in the working modlist.

	vector<Item> items = modlist.Items();
	size_t max = items.size();
	size_t mods = 0;
	for (size_t i=0; i < max; i++) {
		if (items[i].Type() == MOD && items[i].Exists()) {  //Only act on mods that exist.
			if (!items[i].IsMasterFile()) {
				//time_t is an integer number of seconds, so adding 60 on increases it by a minute.
				try {
					items[i].SetModTime(masterTime + mods*60);
				} catch(boss_error e) {
					return BOSS_API_ERROR_FILE_MOD_TIME_WRITE_FAIL;
				}
			}
			mods++;
		}
		if (i == modlist.LastRecognisedPos())
			*lastRecPos = mods-1;
	}

	return BOSS_API_ERROR_OK;
}

// Behaves as the above function does, but does not actually redate the plugins.
// It instead lists them in the order they would be sorted in using SortMods() in
// the sortedPlugins array outputted. The contents of the array are static and should
// not be freed by the client. lastRecPos holds the load order position of the last 
// plugin recognised by BOSS. 
BOSS_API uint32_t TrialSortMods(boss_db db, uint8_t *** sortedPlugins, size_t * listLength, size_t * lastRecPos) {
	if (sortedPlugins == NULL || db == NULL || listLength == NULL)
		return BOSS_API_ERROR_INVALID_ARGS;

	ItemList modlist;
	time_t masterTime, modfiletime = 0;

	try {
		masterTime = GetMasterTime();
	} catch (boss_error e) {
		return BOSS_API_ERROR_MASTER_TIME_READ_FAIL;
	}

	//Build modlist and masterlist.
	try {
		modlist.Load(data_path);
	} catch (boss_error e) {
		if (e.getCode() == BOSS_ERROR_FILE_NOT_FOUND)
			return BOSS_API_ERROR_FILE_NOT_FOUND;
		else if (e.getCode() == BOSS_ERROR_FILE_NOT_UTF8)
			return BOSS_API_ERROR_FILE_NOT_UTF8;
		else
			return BOSS_API_ERROR_PARSE_FAIL;
	}

	//Set up working modlist.
	//The function below changes the input, so make copies.
	ItemList masterlist = db->filteredMasterlist;
	masterlist.EvalConditions();
	RuleList userlist = db->userlist;
	BuildWorkingModlist(modlist, masterlist, userlist);
	string dummy;
	ApplyUserRules(modlist, userlist, dummy);  //This needs to be done to get sensible ordering as the userlist has been taken into account in the working modlist.

	//Initialise vars.
	*listLength = 0;
	*sortedPlugins = NULL;

	//Free memory if already used.
	if (db->extPluginList != NULL) {
		for (size_t i=0; i<db->extPluginListSize; i++)
			free(db->extPluginList[i]);  //Clear all the uint8_t strings created.
		free(db->extPluginList);  //Clear the string array.
	}

	//Build vector of relevant items (ie. only mods).
	vector<Item> items = modlist.Items();
	vector<uint8_t *> mods;
	size_t max = items.size();
	for (size_t i=0; i < max; i++) {
		if (items[i].Type() == MOD && items[i].Exists()) {  //Only act on mods that exist.
			uint8_t * p = StringToUint8_tString(items[i].Name());
			if (p == NULL)
				return BOSS_API_ERROR_NO_MEM;
			mods.push_back(p);
		}
		if (i == modlist.LastRecognisedPos())
			*lastRecPos = mods.size()-1;
	}

	//Now create external array.
	db->extPluginListSize = mods.size();
	db->extPluginList = (uint8_t**)malloc(db->extPluginListSize * sizeof(uint8_t*));
	if (db->extPluginList == NULL)
		return BOSS_API_ERROR_NO_MEM;
	for (size_t i=0; i < db->extPluginListSize; i++)
		db->extPluginList[i] = mods[i];
	
	*sortedPlugins = db->extPluginList;
	*listLength = db->extPluginListSize;

	return BOSS_API_ERROR_OK;
}


//////////////////////////
// DB Access Functions
//////////////////////////

// Returns an array of the Bash Tags encounterred when loading the masterlist
// and userlist, and the number of tags in the returned array. The array and
// its contents are static and should not be freed by the client.
BOSS_API uint32_t GetBashTagMap (boss_db db, BashTag ** tagMap, size_t * numTags) {
	if (db == NULL || tagMap == NULL || numTags == NULL)  //Check for valid args.
		return BOSS_API_ERROR_INVALID_ARGS;

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
			return BOSS_API_ERROR_NO_MEM;

		//Loop through internal bashTagMap and fill output elements.
		for (size_t i=0; i<mapSize; i++) {
			db->extTagMap[i].id = i;
			db->extTagMap[i].name = reinterpret_cast<const uint8_t *>(db->bashTagMap[i].c_str());
		}
		*tagMap = db->extTagMap;
		*numTags = mapSize;
	}
	return BOSS_API_ERROR_OK;
}

// Returns arrays of Bash Tag UIDs for Bash Tags suggested for addition and removal 
// by BOSS's masterlist and userlist, and the number of tags in each array.
// The returned arrays are valid until the db is destroyed or until the Load
// function is called.  The arrays should not be freed by the client. modName is 
// case-insensitive. If no Tags are found for an array, the array pointer (*tagIds)
// will be NULL. The userlistModified bool is true if the userlist contains Bash Tag 
// suggestion message additions.
BOSS_API uint32_t GetModBashTags (boss_db db, const uint8_t * modName, 
									uint32_t ** tagIds_added, 
									size_t * numTags_added, 
									uint32_t **tagIds_removed, 
									size_t *numTags_removed,
									bool * userlistModified) {
	//Check for valid args.
	if (db == NULL || modName == NULL || userlistModified == NULL || numTags_added == NULL || numTags_removed == NULL || tagIds_removed == NULL || tagIds_added == NULL)
		return BOSS_API_ERROR_INVALID_ARGS;
							
	//Convert modName.
	string mod(reinterpret_cast<const char *>(modName));

	if (mod.empty())
		return BOSS_API_ERROR_INVALID_ARGS;

	if (db->extTagMap == NULL)
		return BOSS_API_ERROR_NO_TAG_MAP;

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
		cout << "Yo ho ho!" << endl;
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
				cout << "Yo ho ho!" << endl;
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
		return BOSS_API_ERROR_NO_MEM;
	for (size_t i=0; i < numAdded; i++)
		db->extAddedTagIds[i] = tagsAddedUIDs[i];

	db->extRemovedTagIds = (uint32_t*) malloc(numRemoved * sizeof(uint32_t));
	if (db->extRemovedTagIds == NULL)
		return BOSS_API_ERROR_NO_MEM;
	for (size_t i=0; i < numRemoved; i++)
		db->extRemovedTagIds[i] = tagsRemovedUIDs[i];

	*tagIds_added = db->extAddedTagIds;
	*tagIds_removed = db->extRemovedTagIds;
	*numTags_added = numAdded;
	*numTags_removed = numRemoved;

	return BOSS_API_ERROR_OK;
}

// Returns the message associated with a dirty mod and whether the mod needs
// cleaning. If a mod has no dirty mmessage, *message will be NULL. modName is
// case-insensitive. The return values for needsCleaning are:
//   BOSS_API_CLEAN_NO
//   BOSS_API_CLEAN_YES
//   BOSS_API_CLEAN_UNKNOWN
// The message string is valid until the db is destroyed or until a Load
// function is called. The string should not be freed by the client.
BOSS_API uint32_t GetDirtyMessage (boss_db db, const uint8_t * modName, 
									uint8_t ** message, uint32_t * needsCleaning) {
	//Check for valid args.
	if (db == NULL || modName == NULL || message == NULL || needsCleaning == NULL)
		return BOSS_API_ERROR_INVALID_ARGS;
									
	//Convert modName.
	string mod(reinterpret_cast<const char *>(modName));

	if (mod.empty())
		return BOSS_API_ERROR_INVALID_ARGS;

	//Initialise pointers.
	*message = NULL;
	*needsCleaning = BOSS_API_CLEAN_UNKNOWN;

	//Now search filtered masterlist for mod.
	size_t pos = db->filteredMasterlist.FindItem(mod);
	if (pos != db->filteredMasterlist.Items().size()) {
		vector<Message> messages = db->filteredMasterlist.Items()[pos].Messages();
		for (vector<Message>::iterator messageIter = messages.begin(); messageIter != messages.end(); ++messageIter) {
			if (messageIter->Key() == DIRTY) {
				db->extMessage = StringToUint8_tString(messageIter->Data());
				 *message = db->extMessage;

				if (messageIter->Data().find("Do not clean.") != string::npos)  //Mod should not be cleaned.
					*needsCleaning = BOSS_API_CLEAN_NO;
				else  //Mod should be cleaned.
					*needsCleaning = BOSS_API_CLEAN_YES;
				break;
			}
		}
	}

	return BOSS_API_ERROR_OK;
}

// Writes a minimal masterlist that only contains mods that have Bash Tag suggestions, 
// and/or dirty messages, plus the Tag suggestions and/or messages themselves, in order 
// to create the Wrye Bash taglist. outputFile is the path to use for output. If 
// outputFile already exists, it will only be overwritten if overwrite is true.
BOSS_API uint32_t DumpMinimal (boss_db db, const uint8_t * outputFile, const bool overwrite) {
	//Check for valid args.
	if (db == NULL || outputFile == NULL)
		return BOSS_API_ERROR_INVALID_ARGS;

	string path(reinterpret_cast<const char *>(outputFile));
	if (!fs::exists(path) || overwrite) {
		ofstream mlist(path.c_str());
		if (mlist.fail())
			return BOSS_ERROR_FILE_WRITE_FAIL;
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
		return BOSS_API_ERROR_OK;
	} else
		return BOSS_API_ERROR_OVERWRITE_FAIL;
}