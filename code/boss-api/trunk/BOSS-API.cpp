/*	Better Oblivion Sorting Software API

    Copyright (C) 2011  Random/Random007/jpearce, WrinklyNinja
	& the BOSS development team.
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/


#include "BOSS-API.h"
#include "BOSS-Common.h"
#include <boost/algorithm/string.hpp>
#include <boost/unordered_set.hpp>
#include <map>

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
	vector<modEntry> userlistData, masterlistData, regexData, regexMatchData;	//Internal data structures for userlist Bash Tag suggestions and masterlist Bash Tag suggestions and cleaning messages.
	map<uint32_t,string> bashTagMap;				//A hashmap containing all the Bash Tag strings found in the masterlist and userlist and their unique IDs.
													//Ordered to make ensuring UIDs easy (check the UID of the last element then increment). Strings are case-preserved.
	BashTag * extTagMap;				//Holds the pointer for the bashTagMap returned by GetBashTagMap().
	uint32_t * extAddedTagIds;
	uint32_t * extRemovedTagIds;
	const uint8_t * extMessage;

	//Get a Bash Tag's string name from its UID.
	string GetTagString(uint32_t uid) {
		map<uint32_t, string>::iterator mapPos = bashTagMap.find(uid);
		if (mapPos != bashTagMap.end())
			return mapPos->second;
		else
			return "";
	}
};

//Get a Bash Tag's position in the bashTagMap from its string name.
map<uint32_t, string>::iterator FindBashTag(map<uint32_t,string>& bashTagMap, string value) {
	map<uint32_t, string>::iterator mapPos = bashTagMap.begin();
	while (mapPos != bashTagMap.end()) {
		if (mapPos->second == value)
			break;
		++mapPos;
	}
	return mapPos;
}

// The following are the possible error codes that the API can return.
// Taken from BOSS-Common's Error.h and extended.
BOSS_API const uint32_t BOSS_API_ERROR_OK					=	BOSS_ERROR_OK;
BOSS_API const uint32_t BOSS_API_ERROR_FILE_WRITE_FAIL		=	BOSS_ERROR_FILE_WRITE_FAIL;
BOSS_API const uint32_t BOSS_API_ERROR_FILE_NOT_UTF8		=	BOSS_ERROR_FILE_NOT_UTF8;
BOSS_API const uint32_t BOSS_API_ERROR_FILE_NOT_FOUND		=	BOSS_ERROR_FILE_NOT_FOUND;
BOSS_API const uint32_t BOSS_API_ERROR_PARSE_FAIL			= 	BOSS_ERROR_MAX + 1;
BOSS_API const uint32_t BOSS_API_ERROR_NO_MEM				=	BOSS_ERROR_MAX + 2;
BOSS_API const uint32_t BOSS_API_ERROR_OVERWRITE_FAIL		=	BOSS_ERROR_MAX + 3;
BOSS_API const uint32_t BOSS_API_ERROR_INVALID_ARGS			=	BOSS_ERROR_MAX + 4;
BOSS_API const uint32_t BOSS_API_ERROR_MAX					=	BOSS_API_ERROR_INVALID_ARGS;

// The following are the mod cleanliness states that the API can return.
BOSS_API const uint32_t BOSS_API_CLEAN_NO = 0;
BOSS_API const uint32_t BOSS_API_CLEAN_YES = 1;
BOSS_API const uint32_t BOSS_API_CLEAN_UNKNOWN = 2;


//////////////////////////////
// Version Functions
//////////////////////////////

// Returns whether this version of BOSS supports the API from the given 
// BOSS version. Abstracts BOSS API stability policy away from clients.
BOSS_API bool IsCompatibleVersion (uint32_t bossVersionMajor, uint32_t bossVersionMinor, uint32_t bossVersionPatch) {
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

void DestroyPointers(boss_db db) {
	if (db->extTagMap != NULL)
		free(db->extTagMap);
	if (db->extAddedTagIds != NULL)
		free(db->extAddedTagIds);
	if (db->extRemovedTagIds != NULL)
		free(db->extRemovedTagIds);
	if (db->extMessage != NULL)
		free(const_cast<uint8_t*>(db->extMessage));
}

void InitPointers(boss_db db) {
	db->extTagMap = NULL;
	db->extAddedTagIds = NULL;
	db->extRemovedTagIds = NULL;
	db->extMessage = NULL;
}

// Explicitly manage database lifetime. Allows clients to free memory when
// they want/need to.
BOSS_API uint32_t CreateBossDb  (boss_db * db) {
	if (db == NULL)  //Check for valid args.
		return BOSS_API_ERROR_INVALID_ARGS;

	boss_db retVal = new _boss_db_int;
	if (retVal == NULL)
		return BOSS_API_ERROR_NO_MEM;
	InitPointers(retVal);
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
									const uint8_t * dataPath) {
	//Workflow for masterlist and userlist:
	// 1. Set paths.
	// 2. Parse into temporary structures.
	// 3. Iterate through temporary structures, scanning for Bash Tag suggestions.
	//    In masterlist, also scan for dirty mod messages. Add all mods with such
	//    info to temporary modEntry vector. Also add Bash Tags found to temporary
	//    BashTag vector.
	// 4. Set DB data structures equal to the data structures formed.
	//    If an error is encountered at any point, first set paths back to defaults,
	//    then exit immediately. DB is only changed at the end so it is unchanged 
	//    in case of error.
	map<uint32_t,string> bashTagMap;
	vector<item> masterlist;
	vector<modEntry> masterlistData, userlistData, regexData;
	vector<rule> userlist;
	uint32_t currentUID = 0;
	
	//Check for valid args.
	if (db == NULL || masterlistPath == NULL || userlistPath == NULL || dataPath == NULL)
		return BOSS_API_ERROR_INVALID_ARGS;
	
	//PATH SETTING
	data_path = fs::path(reinterpret_cast<const char *>(dataPath));
	masterlist_path = fs::path(reinterpret_cast<const char *>(masterlistPath));
	userlist_path = fs::path(reinterpret_cast<const char *>(userlistPath));	

	if (data_path.empty() || masterlist_path.empty())
		return BOSS_API_ERROR_INVALID_ARGS;

	//PARSING - Masterlist
	try {
		parseMasterlist(masterlist_path, masterlist);
	} catch (boss_error e) {
		if (e.getCode() == BOSS_ERROR_FILE_NOT_FOUND)
			return BOSS_API_ERROR_FILE_NOT_FOUND;
		else if (e.getCode() == BOSS_ERROR_FILE_NOT_UTF8)
			return BOSS_API_ERROR_FILE_NOT_UTF8;
		else
			return BOSS_API_ERROR_PARSE_FAIL;
	}
	if (!masterlistErrorBuffer.empty())
		return BOSS_API_ERROR_PARSE_FAIL;
	

	//PARSING - Userlist
	if (!userlist_path.empty()) {
		try {
			bool parsed = parseUserlist(userlist_path,userlist);
			if (!parsed) {
				userlist.clear();  //If userlist has parsing errors, empty it so no rules are applied.
				return BOSS_API_ERROR_PARSE_FAIL;
			}
		} catch (boss_error e) {
			if (e.getCode() == BOSS_ERROR_FILE_NOT_FOUND)
				return BOSS_API_ERROR_FILE_NOT_FOUND;
			else if (e.getCode() == BOSS_ERROR_FILE_NOT_UTF8)
				return BOSS_API_ERROR_FILE_NOT_UTF8;
			else
				return BOSS_API_ERROR_PARSE_FAIL;
		}
	}
	

	//CONVERSION - Masterlist
	map<uint32_t, string>::iterator mapPos;
	vector<item>::iterator iter = masterlist.begin();
	while (iter != masterlist.end()) {
		modEntry mod;
		if (iter->type == MOD && !iter->messages.empty()) {  //Might be worth remembering.
			bool hasMessages = false;

			//Find out whether mod already has a dataEntry or not.
			vector<modEntry>::iterator dataIter = masterlistData.begin();
			string plugin = iter->name.string();

			//Check if there is already an existing modEntry in the db userlistData structure with this plugin name.
			while (dataIter != masterlistData.end()) {
				if (Tidy(plugin) == Tidy(dataIter->name))
					break;
				++dataIter;
			}
			//Now if dataIter == masterlistData.end(), the mod does not have an entry, otherwise it does.

			//Check if the messages are of the right type.
			vector<message>::iterator messageIter = iter->messages.begin();
			while (messageIter != iter->messages.end()) {
				if (messageIter->key == TAG) {
					//Search for the Bash Tag listing syntaxes.
					size_t pos1,pos2 = string::npos;
					string addedList, removedList;
					pos1 = messageIter->data.find("{{BASH:");
					if (pos1 != string::npos)
						pos2 = messageIter->data.find("}}", pos1);
					if (pos2 != string::npos)
						addedList = messageIter->data.substr(pos1+7,pos2-pos1-7);

					pos1 = messageIter->data.find("[");
					pos2 = string::npos;
					if (pos1 != string::npos)
						pos2 = messageIter->data.find("]", pos1);
					if (pos2 != string::npos)
						removedList = messageIter->data.substr(pos1+1,pos2-pos1-1);

					if (!addedList.empty()) {
						string name;
						uint32_t uid;
						pos1 = 0;
						pos2 = addedList.find(",", pos1);
						while (pos2 != string::npos) {
							name = trim_copy(addedList.substr(pos1,pos2-pos1));

							//Search db's BashTagMap for the tag. If not found, add it. Either way, add the UID to the modEntry structure.
							mapPos = FindBashTag(bashTagMap, name);
							if (mapPos != bashTagMap.end())										//Tag found in bashTagMap. Get the UID.
								uid = mapPos->first;
							else {															//Tag not found in bashTagMap. Add it. 
								uid = currentUID;
								currentUID++;
								bashTagMap.insert(pair<uint32_t,string>(uid,name));
							}

							//If there is existing modEntry in the db masterlistData structure, don't add this one's info.
							//BOSS itself only works with the first found and the API should behave the same way.
							if (dataIter == masterlistData.end())  //Mod already exists in structure. Tags should be added to it.
								mod.bashTagsAdded.push_back(uid);

							pos1 = pos2+1;
							pos2 = addedList.find(",", pos1);
						}
						name = trim_copy(addedList.substr(pos1));
						mapPos = FindBashTag(bashTagMap, name);
						if (mapPos != bashTagMap.end())										//Tag found in bashTagMap. Get the UID.
							uid = mapPos->first;
						else {															//Tag not found in bashTagMap. Add it. 
							uid = currentUID;
							currentUID++;
							bashTagMap.insert(pair<uint32_t,string>(uid,name));
						}

						if (dataIter == masterlistData.end())  //Mod already exists in structure. Tags should be added to it.
							mod.bashTagsAdded.push_back(uid);
					}

					if (!removedList.empty()) {
						string name;
						uint32_t uid;
						pos1 = 0;
						pos2 = removedList.find(",", pos1);
						while (pos2 != string::npos) {
							name = trim_copy(removedList.substr(pos1,pos2-pos1));

							//Search db's bashTagMap for the tag. If not found, add it. Either way, add the UID to the modEntry structure.
							mapPos = FindBashTag(bashTagMap, name);
							if (mapPos != bashTagMap.end())										//Tag found in bashTagMap. Get the UID.
								uid = mapPos->first;
							else {															//Tag not found in bashTagMap. Add it. 
								uid = currentUID;
								currentUID++;
								bashTagMap.insert(pair<uint32_t,string>(uid,name));
							}

							//Now if there is already an entry, add Tag to that. Otherwise, add Tag to "mod".
							//If there is existing modEntry in the db masterlistData structure, don't add this one's info.
							//BOSS itself only works with the first found and the API should behave the same way.
							if (dataIter == masterlistData.end())
								mod.bashTagsRemoved.push_back(uid);

							pos1 = pos2+1;
							pos2 = removedList.find(",", pos1);
						}
						name = trim_copy(removedList.substr(pos1));
						mapPos = FindBashTag(bashTagMap, name);
						if (mapPos != bashTagMap.end())										//Tag found in bashTagMap. Get the UID.
							uid = mapPos->first;
						else {															//Tag not found in bashTagMap. Add it. 
							uid = currentUID;
							currentUID++;
							bashTagMap.insert(pair<uint32_t,string>(uid,name));
						}

						if (dataIter == masterlistData.end())  //Mod already exists in structure. Tags should be added to it.
							mod.bashTagsRemoved.push_back(uid);
					}	
				} else if (messageIter->key == DIRTY) {
					mod.cleaningMessage = messageIter->data;
				}
				++messageIter;
			}

			//If "mod"'s vectors are not empty, then that means there is not already an entry, so "mod" should be added.
			if (!mod.bashTagsAdded.empty() || !mod.bashTagsRemoved.empty() || !mod.cleaningMessage.empty()) {  //Tags exist to be added, but there is no entry for this plugin currently. Add an entry.
				mod.name = plugin;
				masterlistData.push_back(mod);
			}
		} else if (iter->type == REGEX && !iter->messages.empty()) {  //Might be worth remembering.
			//Check if the messages are of the right type.
			vector<message>::iterator messageIter = iter->messages.begin();
			while (messageIter != iter->messages.end()) {
				if (messageIter->key == TAG) {
					//Search for the Bash Tag listing syntaxes.
					size_t pos1,pos2 = string::npos;
					string addedList, removedList;
					pos1 = messageIter->data.find("{{BASH:");
					if (pos1 != string::npos)
						pos2 = messageIter->data.find("}}", pos1);
					if (pos2 != string::npos)
						addedList = messageIter->data.substr(pos1+7,pos2-pos1-7);

					pos1 = messageIter->data.find("[");
					pos2 = string::npos;
					if (pos1 != string::npos)
						pos2 = messageIter->data.find("]", pos1);
					if (pos2 != string::npos)
						removedList = messageIter->data.substr(pos1+1,pos2-pos1-1);

					if (!addedList.empty()) {
						string name;
						uint32_t uid;
						pos1 = 0;
						pos2 = addedList.find(",", pos1);
						while (pos2 != string::npos) {
							name = trim_copy(addedList.substr(pos1,pos2-pos1));

							//Search db's bashTagMap for the tag. If not found, add it. Either way, add the UID to the modEntry structure.
							mapPos = FindBashTag(bashTagMap, name);
							if (mapPos != bashTagMap.end())										//Tag found in bashTagMap. Get the UID.
								uid = mapPos->first;
							else {															//Tag not found in bashTagMap. Add it. 
								uid = currentUID;
								currentUID++;
								bashTagMap.insert(pair<uint32_t,string>(uid,name));
							}

							//We don't know if any regex-matching mods are installed yet, so just add it to a temporary modEntry.
							mod.bashTagsAdded.push_back(uid);

							pos1 = pos2+1;
							pos2 = addedList.find(",", pos1);
						}
						name = trim_copy(addedList.substr(pos1));
						mapPos = FindBashTag(bashTagMap, name);
						if (mapPos != bashTagMap.end())										//Tag found in bashTagMap. Get the UID.
							uid = mapPos->first;
						else {															//Tag not found in bashTagMap. Add it. 
							uid = currentUID;
							currentUID++;
							bashTagMap.insert(pair<uint32_t,string>(uid,name));
						}

						mod.bashTagsAdded.push_back(uid);
					}

					if (!removedList.empty()) {
						string name;
						uint32_t uid;
						pos1 = 0;
						pos2 = removedList.find(",", pos1);
						while (pos2 != string::npos) {
							name = trim_copy(removedList.substr(pos1,pos2-pos1));

							//Search db's bashTagMap for the tag. If not found, add it. Either way, add the UID to the modEntry structure.
							mapPos = FindBashTag(bashTagMap, name);
							if (mapPos != bashTagMap.end())										//Tag found in bashTagMap. Get the UID.
								uid = mapPos->first;
							else {															//Tag not found in bashTagMap. Add it. 
								uid = currentUID;
								currentUID++;
								bashTagMap.insert(pair<uint32_t,string>(uid,name));
							}

							//We don't know if any regex-matching mods are installed yet, so just add it to a temporary modEntry.
							mod.bashTagsRemoved.push_back(uid);

							pos1 = pos2+1;
							pos2 = removedList.find(",", pos1);
						}
						name = trim_copy(removedList.substr(pos1));
						mapPos = FindBashTag(bashTagMap, name);
						if (mapPos != bashTagMap.end())										//Tag found in bashTagMap. Get the UID.
							uid = mapPos->first;
						else {															//Tag not found in bashTagMap. Add it. 
							uid = currentUID;
							currentUID++;
							bashTagMap.insert(pair<uint32_t,string>(uid,name));
						}

						mod.bashTagsRemoved.push_back(uid);
					}	
				} else if (messageIter->key == DIRTY) {
					mod.cleaningMessage = messageIter->data;
				}
				++messageIter;
			}

			if (mod.cleaningMessage.empty() && mod.bashTagsAdded.empty() || mod.bashTagsRemoved.empty())
				continue;  //Nothing of interest.

			//To save clients having to Load() every time their load order changes, 
			//move the regex matching to a separate function (called by Load() and client).
			//To tell regexes and normal mod entries apart, add regexes to a different vector.

			//To ensure ghosted mods are matched, add .ghost regex.
			mod.name = iter->name.string()+"(.ghost)?";

			//Now check that regex is not already in vector.
			vector<modEntry>::iterator dataIter = regexData.begin();
			while (dataIter != regexData.end()) {
				if (Tidy(mod.name) == Tidy(dataIter->name))
					break;
				++dataIter;
			}

			if (dataIter == regexData.end()) {  //Interesting info, but no existing modEntry.
				regexData.push_back(mod);
			}
		}
		++iter;
	}
	
	//The masterlistData structure now only contains mods that are installed and have Bash Tag suggestions or dirty mod messages attached.
	//The Tag suggestions are listed by their UIDs in vectors according to whether they are being added or removed.
	//The valid dirty mod message is also stored.
	
	//CONVERSION - Userlist
	if (!userlist_path.empty()) {
		vector<rule>::iterator userlistIter = userlist.begin();
		while (userlistIter != userlist.end()) {
			vector<line>::iterator lineIter = userlistIter->lines.begin();
			vector<modEntry>::iterator dataIter = userlistData.begin();
			string plugin = userlistIter->ruleObject;
			modEntry mod;

			//Check if there is already an existing modEntry in the db userlistData structure with this plugin name.
			while (dataIter != userlistData.end()) {
				if (Tidy(plugin) == Tidy(dataIter->name))
					break;
				++dataIter;
			}
			//Now if dataIter == userlistData.end(), the mod does not have an entry, otherwise it does.

			while (lineIter != userlistIter->lines.end()) {
				if (lineIter->key == APPEND || lineIter->key == REPLACE) {
					//Need to check if the message added is a Bash Tag suggestion.
					char sym = lineIter->object[0];  //Look for message symbol. (MF1)
					size_t pos = lineIter->object.find(":"); //Look for separator colon. (MF2)
					keyType key;
					if (pos != string::npos)
						key = StringToKey(Tidy(lineIter->object.substr(0,pos)));  //MF2 keyword.

					if (sym == '%' || key == TAG) {  //It's a Bash Tag suggestion.
				
						//If REPLACE, it needs to replace what's in the masterlist, so search masterlistData for the mod and clear its Bash Tag vectors if found.
						vector<modEntry>::iterator mIter = masterlistData.begin();
						while (mIter != masterlistData.end()) {
							if (Tidy(plugin) == Tidy(mIter->name)) {
								mIter->bashTagsAdded.clear();
								mIter->bashTagsRemoved.clear();
								break;
							}
							++dataIter;
						}
				
						//Search for the Bash Tag listing syntaxes.
						size_t pos1,pos2 = string::npos;
						string addedList, removedList;
						pos1 = lineIter->object.find("{{BASH:");
						if (pos1 != string::npos)
							pos2 = lineIter->object.find("}}", pos1);
						if (pos2 != string::npos)
							addedList = lineIter->object.substr(pos1+7,pos2-pos1-7);

						pos1 = lineIter->object.find("[");
						pos2 = string::npos;
						if (pos1 != string::npos)
							pos2 = lineIter->object.find("]", pos1);
						if (pos2 != string::npos)
							removedList = lineIter->object.substr(pos1+1,pos2-pos1-1);

						if (!addedList.empty()) {
							string name;
							uint32_t uid;
							pos1 = 0;
							pos2 = addedList.find(",", pos1);
							while (pos2 != string::npos) {
								name = trim_copy(addedList.substr(pos1,pos2-pos1));

								//Search db's bashTagMap for the tag. If not found, add it. Either way, add the UID to the modEntry structure.
								mapPos = FindBashTag(bashTagMap, name);
								if (mapPos != bashTagMap.end()) {										//Tag found in bashTagMap. Get the UID.
									uid = mapPos->first;
								} else {															//Tag not found in bashTagMap. Add it. 
									uid = currentUID;
									currentUID++;
									bashTagMap.insert(pair<uint32_t,string>(uid,name));
								}

								//Now if there is already an entry, add Tag to that. Otherwise, add Tag to "mod".
								if (dataIter != userlistData.end())  //Mod already exists in structure. Tags should be added to it.
									dataIter->bashTagsAdded.push_back(uid);
								else
									mod.bashTagsAdded.push_back(uid);

								pos1 = pos2+1;
								pos2 = addedList.find(",", pos1);
							}
							name = trim_copy(addedList.substr(pos1));
							mapPos = FindBashTag(bashTagMap, name);
							if (mapPos != bashTagMap.end())										//Tag found in bashTagMap. Get the UID.
								uid = mapPos->first;
							else {															//Tag not found in bashTagMap. Add it. 
								uid = currentUID;
								currentUID++;
								bashTagMap.insert(pair<uint32_t,string>(uid,name));
							}

							//Now if there is already an entry, add Tag to that. Otherwise, add Tag to "mod".
							if (dataIter != userlistData.end())  //Mod already exists in structure. Tags should be added to it.
								dataIter->bashTagsAdded.push_back(uid);
							else
								mod.bashTagsAdded.push_back(uid);
						}

						if (!removedList.empty()) {
							string name;
							uint32_t uid;
							pos1 = 0;
							pos2 = removedList.find(",", pos1);
							while (pos2 != string::npos) {
								name = trim_copy(removedList.substr(pos1,pos2-pos1));

								//Search db's bashTagMap for the tag. If not found, add it. Either way, add the UID to the modEntry structure.
								mapPos = FindBashTag(bashTagMap, name);
								if (mapPos != bashTagMap.end()) {										//Tag found in bashTagMap. Get the UID.
									uid = mapPos->first;
								} else {															//Tag not found in bashTagMap. Add it. 
									uid = currentUID;
									currentUID++;
									bashTagMap.insert(pair<uint32_t,string>(uid,name));
								}

								//Now if there is already an entry, add Tag to that. Otherwise, add Tag to "mod".
								if (dataIter != userlistData.end())  //Mod already exists in structure. Tags should be added to it.
									dataIter->bashTagsRemoved.push_back(uid);
								else
									mod.bashTagsRemoved.push_back(uid);

								pos1 = pos2+1;
								pos2 = removedList.find(",", pos1);
							}
							name = trim_copy(removedList.substr(pos1));
							mapPos = FindBashTag(bashTagMap, name);
							if (mapPos != bashTagMap.end())										//Tag found in bashTagMap. Get the UID.
								uid = mapPos->first;
							else {															//Tag not found in bashTagMap. Add it. 
								uid = currentUID;
								currentUID++;
								bashTagMap.insert(pair<uint32_t,string>(uid,name));
							}

							//Now if there is already an entry, add Tag to that. Otherwise, add Tag to "mod".
							if (dataIter != userlistData.end())  //Mod already exists in structure. Tags should be added to it.
								dataIter->bashTagsRemoved.push_back(uid);
							else
								mod.bashTagsRemoved.push_back(uid);
						}
					}
				}
				++lineIter;
			}

			//If "mod"'s vectors are not empty, then that means there is not already an entry, so "mod" should be added.
			if (!mod.bashTagsAdded.empty() || !mod.bashTagsRemoved.empty()) {  //Tags exist to be added, but there is no entry for this plugin currently. Add an entry.
				mod.name = plugin;
				userlistData.push_back(mod);
			}
			++userlistIter;
		}
	}
	
	//The userlist has now been adapted so that only the following information is stored:
	// - Mods with Bash Tags added and/or removed.
	// - The UIDs of the Bash Tags added and/or removed for those mods.

	//FREE CURRENT POINTERS
	//Free memory at pointers stored in structure.
	DestroyPointers(db);
	InitPointers(db);
	
	//DB SET
	db->masterlistData = masterlistData;
	db->regexData = regexData;
	db->userlistData = userlistData;
	db->bashTagMap = bashTagMap;
	ReEvalRegex(db, dataPath);
	return BOSS_API_ERROR_OK;
}

BOSS_API uint32_t ReEvalRegex(boss_db db, const uint8_t * dataPath) {
	//Check for valid args.
	if (db == NULL || dataPath == NULL)
		return BOSS_API_ERROR_INVALID_ARGS;
		
	//PATH SETTING
	data_path = fs::path(reinterpret_cast<const char *>(dataPath));

	if (data_path.empty())
		return BOSS_API_ERROR_INVALID_ARGS;

	vector<modEntry> matches;
	//Build modlist. Not using boss::BuildModlist() as that builds to a vector<item> in load order, which isn't necessary.
	boost::unordered_set<string> hashset;  //Holds installed mods for checking against masterlist
	boost::unordered_set<string>::iterator setPos;
	for (fs::directory_iterator itr(data_path); itr!=fs::directory_iterator(); ++itr) {
		const fs::path filename = itr->path().filename();
		const string ext = to_lower_copy(itr->path().extension().string());
		if (fs::is_regular_file(itr->status()) && (ext==".esp" || ext==".esm" || ext==".ghost"))	//Add file to hashset.
			hashset.insert(Tidy(filename.string()));
	}

	//Now search through hashset for each regexData element and add matches o a regexMatchData vector.
	vector<modEntry>::iterator iter = db->regexData.begin();
	while (iter != db->regexData.end()) {
		//Now check to see if it matches any installed mods.
		//Form a regex.
		boost::regex reg(Tidy(iter->name),boost::regex::extended);  //Ghost extension is added so ghosted mods will also be found.
		//Now start looking.
		setPos = hashset.begin();
		do {
			setPos = FindRegexMatch(hashset, reg, setPos);
			if (setPos != hashset.end()) {  //Mod is installed.
				//Check if there is already an existing modEntry in the db masterlistData structure with this plugin name.
				//If there is, don't add this one's info, as BOSS itself only works with the first found and the API should behave the same way.
				vector<modEntry>::iterator dataIter = matches.begin();
				while (dataIter != matches.end()) {
					if (Tidy(iter->name) == Tidy(dataIter->name))
						break;
					++dataIter;
				}
				//Now if dataIter == matches.end(), the mod does not have an entry, otherwise it does.

				if (dataIter == matches.end()) {  //Interesting info, but no existing modEntry.
					matches.push_back(*iter);
				}
			}
			++setPos;
		} while (setPos != hashset.end());
		++iter;
	}

	//Now set DB vector to function's vector.
	db->regexMatchData = matches;
	return BOSS_API_ERROR_OK;
}


//////////////////////////
// DB Access Functions
//////////////////////////

// Returns an array of the Bash Tags encounterred when loading the masterlist
// and userlist, and the number of tags in the returned array. The array and
// its contents are static and should not be freed by the client.
BOSS_API uint32_t GetBashTagMap (boss_db db, BashTag ** tagMap, uint32_t * numTags) {
	if (db == NULL || tagMap == NULL || numTags == NULL)  //Check for valid args.
		return BOSS_API_ERROR_INVALID_ARGS;

	if (db->extTagMap != NULL) {  //Check to see if bashTagMap is already populated.
		*numTags = uint32_t(db->bashTagMap.size());  //Set size.
		*tagMap = db->extTagMap;
	} else {
		*numTags = uint32_t(db->bashTagMap.size());  //Set size.

		//Allocate memory.
		db->extTagMap = (BashTag*)calloc(size_t(*numTags), sizeof(BashTag));
		if (db->extTagMap == NULL)
			return BOSS_API_ERROR_NO_MEM;

		//Loop through internal bashTagMap and fill output elements.
		for (uint32_t i=0;i<*numTags;i++) {
			db->extTagMap[i].id = i;
			db->extTagMap[i].name = reinterpret_cast<const uint8_t *>(db->bashTagMap[i].c_str());
		}
		*tagMap = db->extTagMap;
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
									uint32_t * numTags_added, 
									uint32_t **tagIds_removed, 
									uint32_t *numTags_removed,
									bool * userlistModified) {
	//Check for valid args.
	if (db == NULL || modName == NULL || userlistModified == NULL || numTags_added == NULL || numTags_removed == NULL || tagIds_removed == NULL || tagIds_added == NULL)
		return BOSS_API_ERROR_INVALID_ARGS;
							
	//Convert modName.
	string mod(reinterpret_cast<const char *>(modName));

	if (mod.empty())
		return BOSS_API_ERROR_INVALID_ARGS;

	//Allocate memory for userlistModified.
	//userlistModified = (bool*)malloc(sizeof(bool));
	//db->extTagIds.push_back(userlistModified);  //Record address.

	//Initialise pointers to null and zero tag counts.
	*numTags_added = 0;
	*numTags_removed = 0;
	*tagIds_removed = NULL;
	*tagIds_added = NULL;
	*userlistModified = false;
	
	//Need to search masterlist, regexMatch then userlist separately, check each mod case-insensitively for a match.
	size_t size1, size2;
	vector<modEntry>::iterator iter;
	vector<uint32_t> masterlist_tagsAdded, masterlist_tagsRemoved, userlist_tagsAdded, userlist_tagsRemoved, regex_tagsAdded, regex_tagsRemoved;

	//Masterlist
	iter = db->masterlistData.begin();
	while (iter != db->masterlistData.end()) {
		if (Tidy(iter->name) == Tidy(mod)) {  //Mod found.
			//Add UIDs of tags added and removed to outputs.
			masterlist_tagsAdded = iter->bashTagsAdded;
			masterlist_tagsRemoved = iter->bashTagsRemoved;
			break;
		}
		++iter;
	}

	//regexMatch
	iter = db->regexMatchData.begin();
	while (iter != db->regexMatchData.end()) {
		if (Tidy(iter->name) == Tidy(mod)) {  //Mod found.
			//Add UIDs of tags added and removed to outputs.
			regex_tagsAdded = iter->bashTagsAdded;
			regex_tagsRemoved = iter->bashTagsRemoved;
			break;
		}
		++iter;
	}

	//Userlist
	iter = db->userlistData.begin();
	while (iter != db->userlistData.end()) {
		if (Tidy(iter->name) == Tidy(mod)) {  //Mod found.
			//Add UIDs of tags added and removed to outputs.
			userlist_tagsAdded = iter->bashTagsAdded;
			userlist_tagsRemoved = iter->bashTagsRemoved;
			break;
		}
		++iter;
	}

	if (!userlist_tagsAdded.empty() || !userlist_tagsRemoved.empty())
		*userlistModified = true;

	//Now combine the masterlist and userlist Tags.
	*numTags_added = masterlist_tagsAdded.size() + userlist_tagsAdded.size() + regex_tagsAdded.size();
	*numTags_removed = masterlist_tagsRemoved.size() + userlist_tagsRemoved.size() + regex_tagsRemoved.size();
	
	//Allocate memory.
	uint32_t * temp;
	temp = (uint32_t*)realloc(db->extAddedTagIds,*numTags_added * sizeof(uint32_t));
	if (temp == NULL) {  //The realloc() fails sometimes for some reason. Try doing the same thing with free() and malloc().
		free(db->extAddedTagIds);
		db->extAddedTagIds = (uint32_t*)malloc(*numTags_added * sizeof(uint32_t));
		if (db->extAddedTagIds == NULL)
			return BOSS_API_ERROR_NO_MEM;
	} else 
		db->extAddedTagIds = temp;
	temp = (uint32_t*)realloc(db->extRemovedTagIds,*numTags_removed * sizeof(uint32_t));
	if (temp == NULL) {  //The realloc() fails sometimes for some reason. Try doing the same thing with free() and malloc().
		free(db->extRemovedTagIds);
		db->extRemovedTagIds = (uint32_t*)malloc(*numTags_removed * sizeof(uint32_t));
		if (db->extRemovedTagIds == NULL)
			return BOSS_API_ERROR_NO_MEM;
	} else 
		db->extRemovedTagIds = temp;
	

	//Loop through vectors and fill output elements.
	size1 = masterlist_tagsAdded.size();
	size2 = regex_tagsAdded.size();
	for (size_t i=0;i<size1;i++)
		db->extAddedTagIds[i] = masterlist_tagsAdded[i];
	for (size_t i=size1;i<size1+size2;i++)
		db->extAddedTagIds[i] = regex_tagsAdded[i-size1];
	for (size_t i=size1+size2;i<*numTags_added;i++)
		db->extAddedTagIds[i] = userlist_tagsAdded[i-size1-size2];

	size1 = masterlist_tagsRemoved.size();
	size2 = regex_tagsRemoved.size();
	for (size_t i=0;i<size1;i++)
		db->extRemovedTagIds[i] = masterlist_tagsRemoved[i];
	for (size_t i=size1;i<size1+size2;i++)
		db->extRemovedTagIds[i] = regex_tagsRemoved[i-size1];
	for (size_t i=size1+size2;i<*numTags_removed;i++)
		db->extRemovedTagIds[i] = userlist_tagsRemoved[i-size1-size2];

	*tagIds_added = db->extAddedTagIds;
	*tagIds_removed = db->extRemovedTagIds;

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
									const uint8_t ** message, uint32_t * needsCleaning) {
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

	//Search masterlist.
	vector<modEntry>::iterator iter = db->masterlistData.begin();
	while (iter != db->masterlistData.end()) {
		if (Tidy(iter->name) == Tidy(mod)) {
			if (!iter->cleaningMessage.empty()) {  //Mod found and it has a message.
				db->extMessage = reinterpret_cast<const uint8_t *>(iter->cleaningMessage.c_str());
				*message = db->extMessage;

				if (iter->cleaningMessage.find("Do not clean.") != string::npos)  //Mod should not be cleaned.
					*needsCleaning = BOSS_API_CLEAN_NO;
				else  //Mod should be cleaned.
					*needsCleaning = BOSS_API_CLEAN_YES;
				break;
			}
		}
		++iter;
	}

	return BOSS_API_ERROR_OK;
}

// Writes a minimal masterlist that only contains mods that have Bash Tag suggestions, 
// plus the Tag suggestions themselves, in order to create the Wrye Bash taglist.
// outputFile is the path to use for output. If outputFile already exists, it will
// only be overwritten if overwrite is true.
BOSS_API uint32_t DumpMinimal (boss_db db, const uint8_t * outputFile, const bool overwrite) {
	//Check for valid args.
	if (db == NULL || outputFile == NULL)
		return BOSS_API_ERROR_INVALID_ARGS;

	string path(reinterpret_cast<const char *>(outputFile));
	if (!fs::exists(path) || overwrite) {
		//Convert masterlistData back to MF2 masterlist syntax and output.
		ofstream mlist(path.c_str());
		if (mlist.fail())
			return BOSS_ERROR_FILE_WRITE_FAIL;
		else {
			//Iterate through masterlistData and then regexData.
			//masterlistData
			vector<modEntry>::iterator iter = db->masterlistData.begin();
			while (iter != db->masterlistData.end()) {
				
				//Output mod line.
				mlist << iter->name << endl;
				
				//Now output Bash Tag suggestions.
				if (!iter->bashTagsAdded.empty() || !iter->bashTagsRemoved.empty()) {
					mlist << "TAG:";
					if (!iter->bashTagsAdded.empty()) {
						mlist << " {{BASH: ";
						vector<uint32_t>::iterator tagIter = iter->bashTagsAdded.begin();
						vector<uint32_t>::iterator last = iter->bashTagsAdded.end();
						last--;
						while (tagIter != iter->bashTagsAdded.end()) {
							mlist << db->GetTagString(*tagIter);
							if (tagIter != last)
								mlist << ", ";
							++tagIter;
						}
						mlist << "}}";
					}
					if (!iter->bashTagsRemoved.empty()) {
						mlist << " [";
						vector<uint32_t>::iterator tagIter = iter->bashTagsRemoved.begin();
						vector<uint32_t>::iterator last = iter->bashTagsRemoved.end();
						last--;
						while (tagIter != iter->bashTagsRemoved.end()) {
							mlist << db->GetTagString(*tagIter);
							if (tagIter != last)
								mlist << ", ";
							++tagIter;
						}
						mlist << "]";
					}
					mlist << endl;
				}
				++iter;
			}
			//regexData
			iter = db->regexData.begin();
			while (iter != db->regexData.end()) {
				//Output mod line.
				mlist << "REGEX: " << iter->name << endl;
				
				//Now output Bash Tag suggestions.
				if (!iter->bashTagsAdded.empty() || !iter->bashTagsRemoved.empty()) {
					mlist << "TAG:";
					if (!iter->bashTagsAdded.empty()) {
						mlist << " {{BASH: ";
						vector<uint32_t>::iterator tagIter = iter->bashTagsAdded.begin();
						vector<uint32_t>::iterator last = iter->bashTagsAdded.end();
						last--;
						while (tagIter != iter->bashTagsAdded.end()) {
							mlist << db->GetTagString(*tagIter);
							if (tagIter != last)
								mlist << ", ";
							++tagIter;
						}
						mlist << "}}";
					}
					if (!iter->bashTagsRemoved.empty()) {
						mlist << " [";
						vector<uint32_t>::iterator tagIter = iter->bashTagsRemoved.begin();
						vector<uint32_t>::iterator last = iter->bashTagsRemoved.end();
						last--;
						while (tagIter != iter->bashTagsRemoved.end()) {
							mlist << db->GetTagString(*tagIter);
							if (tagIter != last)
								mlist << ", ";
							++tagIter;
						}
						mlist << "]";
					}
					mlist << endl;
				}
				++iter;
			}
			mlist.close();
		}
		return BOSS_API_ERROR_OK;
	} else
		return BOSS_API_ERROR_OVERWRITE_FAIL;
}