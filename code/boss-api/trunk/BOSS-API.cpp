/*	Better Oblivion Sorting Software API

    Copyright (C) 2011  Random/Random007/jpearce, WrinklyNinja
	& the BOSS development team.
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/


#include "BOSS-API.h"
#include "BOSS-Common.h"
#include <boost/algorithm/string.hpp>
#include <boost/unordered_map.hpp>

using namespace std;
using namespace boss;

using boost::algorithm::trim_copy;
using boost::algorithm::to_lower_copy;

#ifdef __cplusplus
extern "C"
{
#endif

////////////////////////
// Types (Internal)
////////////////////////

// Structure for a single plugin's data.
struct modEntry {
	string name;						//Plugin filename, case NOT preserved.
	vector<uint32_t> bashTagsAdded;		//Vector of Bash Tags added unique IDs.
	vector<uint32_t> bashTagsRemoved;	//Vector of Bash Tags removed unique IDs.
	string cleaningMessage;				//Doesn't need to be a vector because there will only be one valid cleaning message outputted by the parser.
};

// Database structure.
struct _boss_db_int {
	vector<modEntry> userlistData, masterlistData;	//Internal data structures for userlist Bash Tag suggestions and masterlist Bash Tag suggestions and cleaning messages.
	map<uint32_t,string> bashTagMap;				//A hashmap containing all the Bash Tag strings found in the masterlist and userlist and their unique IDs.
													//Ordered to make ensuring UIDs easy (check the UID of the last element then increment). Strings are case-preserved.
	BashTag * extTagMap;				//Holds the pointer for the bashTagMap returned by GetBashTagMap().
	vector<uint32_t*> extTagIds;		//Holds the pointers to the arrays of Bash Tag UIDs returned by GetModBashTags() for each mod.
	vector<uint8_t*> extMessages;		//Holds the pointers to the dirty message string returned by GetDirtyMessage() for each mod.

	//Get a Bash Tag's string name from its UID.
	string GetTagString(uint32_t uid) {
		map<uint32_t, string>::iterator mapPos = bashTagMap.find(uid);
		if (mapPos != bashTagMap.end())
			return mapPos->second;
		else
			return "";
	}
};

#ifdef __cplusplus
}
#endif

//Get a Bash Tag's position in the bashTagMap from its string name.
map<uint32_t, string>::iterator FindBashTag(map<uint32_t,string> bashTagMap, string value) {
	map<uint32_t, string>::iterator mapPos = bashTagMap.begin();
	while (mapPos != bashTagMap.end()) {
		if (mapPos->second == value)
			break;
	}
	return mapPos;
}


#ifdef __cplusplus
extern "C"
{
#endif



// String conversion from std::string to uint8_t*.
// Assumes both have memory allocated.
uint32_t StdStringToUint8_t(const string str, uint8_t ** ostr) {
	uint32_t length = str.length();
	if (length != (sizeof(ostr) / sizeof(uint8_t)))
		return BOSS_API_ERROR_NO_MEM;
	//Fill memory allocated.
	for (uint32_t i=0;i<length;i++) {
		*ostr[i] = str[i];
	}
	return BOSS_API_ERROR_OK;
}

//String conversion from uint8_t* to std::string.
uint32_t Uint8_tToStdString(const uint8_t * str, string * ostr) {
	//Check for valid args.
	if (str == 0)
		return BOSS_API_ERROR_INVALID_ARGS;

	uint32_t length = sizeof(str) / sizeof(uint8_t);
	for (uint32_t i=0;i<length;i++) {
		ostr[i] = str[i];
	}
	return BOSS_API_ERROR_OK;
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
BOSS_API const uint32_t BOSS_API_ERROR_TAGMAP_EXISTS		=	BOSS_ERROR_MAX + 5;

//////////////////////////////
// Version Functions
//////////////////////////////

// Returns whether this version of BOSS supports the API from the given 
// BOSS version. Abstracts BOSS API stability policy away from clients.
BOSS_API bool IsCompatibleVersion (uint32_t bossVersionMajor, uint32_t bossVersionMinor) {
	//The 1.9 API is backwards compatible with all 1.x versions of BOSS.
	if (bossVersionMajor <= 1 && bossVersionMajor <= 9)
		return true;
	else
		return false;
}

// Returns the version string for this version of BOSS.
// The string exists for the lifetime of the library.
BOSS_API uint32_t GetVersionString (const uint8_t ** bossVersionStr) {
	//Check for valid args.
	if (bossVersionStr == 0)
		return BOSS_API_ERROR_INVALID_ARGS;
	
	uint8_t * ver;
	//Allocate memory.
	ver = (uint8_t*)malloc(sizeof(uint8_t) * g_version.length());  //Returns null if malloc failed.
	if (ver == 0)
		return BOSS_API_ERROR_NO_MEM;
	
	uint32_t ret = StdStringToUint8_t(g_version, &ver);
	if (ret == BOSS_API_ERROR_OK)
		*bossVersionStr = ver;
	return ret;
}


////////////////////////////////////
// Lifecycle Management Functions
////////////////////////////////////

// Explicitly manage database lifetime. Allows clients to free memory when
// they want/need to.
BOSS_API uint32_t CreateBossDb  (boss_db * db) {
	//Check for valid args.
	if (db == 0)
		return BOSS_API_ERROR_INVALID_ARGS;
	*db = new _boss_db_int;
	return BOSS_API_ERROR_OK;
}
BOSS_API void     DestroyBossDb (boss_db db) {
	//Check for valid args.
	if (db != 0) {
		//Free memory at pointers stored in structure.
		free(db->extTagMap);
		vector<uint32_t*>::iterator bashTagIter = db->extTagIds.begin();
		while (bashTagIter != db->extTagIds.end()) {
			free(*bashTagIter);
			++bashTagIter;
		}
		vector<uint8_t*>::iterator messageIter = db->extMessages.begin();
		while (messageIter != db->extMessages.end()) {
			free(*messageIter);
			++messageIter;
		}
	
		//Now delete DB.
		delete db;
	}
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
	uint32_t ret;
	map<uint32_t,string> bashTagMap;
	vector<item> masterlist;
	vector<modEntry> masterlistData, userlistData;
	vector<rule> userlist;
	
	//Check for valid args.
	if (db == 0 || masterlistPath == 0 || userlistPath == 0 || dataPath == 0)
		return BOSS_API_ERROR_INVALID_ARGS;
	
	//PATH SETTING
	string uPath, mPath, dPath;
	ret = Uint8_tToStdString(masterlistPath, &mPath);
	if (ret != BOSS_API_ERROR_OK)
		return ret;
	ret = Uint8_tToStdString(userlistPath, &uPath);
	if (ret != BOSS_API_ERROR_OK)
		return ret;
	ret = Uint8_tToStdString(dataPath, &dPath);
	if (ret != BOSS_API_ERROR_OK)
		return ret;
	data_path = fs::path(dPath);
	masterlist_path = fs::path(mPath);
	userlist_path = fs::path(uPath);
	
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
	
	//Build modlist. Not using boss::BuildModlist() as that builds to a vector<item> in load order, which isn't necessary.
	boost::unordered_set<string> hashset;  //Holds installed mods for checking against masterlist
	for (fs::directory_iterator itr(data_path); itr!=fs::directory_iterator(); ++itr) {
		const fs::path filename = itr->path().filename();
		const string ext = to_lower_copy(itr->path().extension().string());
		if (fs::is_regular_file(itr->status()) && (ext==".esp" || ext==".esm" || ext==".ghost"))	//Add file to hashset.
			hashset.insert(Tidy(filename.string()));
	}
	
	//PARSING - Userlist
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
	
	//CONVERSION - Masterlist
	boost::unordered_set<string>::iterator setPos;
	map<uint32_t, string>::iterator mapPos;
	vector<item>::iterator iter = masterlist.begin();
	while (iter != masterlist.end()) {
		modEntry mod;
		if (iter->type == MOD && !iter->messages.empty()) {  //Might be worth remembering.
			bool hasMessages = false, isInstalled = false;

			//Check if the mod is installed.
			setPos = hashset.find(Tidy(iter->name.string()));
			if (setPos != hashset.end())									//Mod found in hashset. 
				isInstalled  = true;
			else {
				//Mod not found. Look for ghosted mod.
				iter->name = fs::path(iter->name.string() + ".ghost");		//Add ghost extension to mod name.
				setPos = hashset.find(Tidy(iter->name.string()));
				if (setPos != hashset.end())									//Mod found in hashset.
						isInstalled  = true;
			}
			if (!isInstalled)
				continue;

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
					if (pos1 != string::npos)
						pos2 = messageIter->data.find("]", pos1);
					if (pos2 != string::npos)
						removedList = messageIter->data.substr(pos1+1,pos2-pos1-1);

					if (!addedList.empty()) {
						pos1 = 0;
						pos2 = addedList.find(",", pos1);
						while (pos2 != string::npos) {
							string name = trim_copy(addedList.substr(pos1,pos2-pos1));
							uint32_t uid;

							//Search db's BashTagMap for the tag. If not found, add it. Either way, add the UID to the modEntry structure.
							mapPos = FindBashTag(bashTagMap, name);
							if (mapPos != bashTagMap.end()) {										//Tag found in bashTagMap. Get the UID.
								uid = mapPos->first;
							} else {															//Tag not found in bashTagMap. Add it. 
								uint32_t uid = bashTagMap.rbegin()->first + 1;
								bashTagMap.insert(pair<uint32_t,string>(uid,name));
							}

							//Now if there is already an entry, add Tag to that. Otherwise, add Tag to "mod".
							//If there is existing modEntry in the db masterlistData structure, don't add this one's info.
							//BOSS itself only works with the first found and the API should behave the same way.
							if (dataIter == masterlistData.end())  //Mod already exists in structure. Tags should be added to it.
								mod.bashTagsAdded.push_back(uid);

							pos1 = pos2+1;
							pos2 = addedList.find(",", pos1);
						}
					}

					if (!removedList.empty()) {
						pos1 = 0;
						pos2 = removedList.find(",", pos1);
						while (pos2 != string::npos) {
							string name = trim_copy(removedList.substr(pos1,pos2-pos1));
							uint32_t uid;

							//Search db's bashTagMap for the tag. If not found, add it. Either way, add the UID to the modEntry structure.
							mapPos = FindBashTag(bashTagMap, name);
							if (mapPos != bashTagMap.end()) {										//Tag found in bashTagMap. Get the UID.
								uid = mapPos->first;
							} else {															//Tag not found in bashTagMap. Add it. 
								uint32_t uid = bashTagMap.rbegin()->first + 1;
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
					if (pos1 != string::npos)
						pos2 = messageIter->data.find("]", pos1);
					if (pos2 != string::npos)
						removedList = messageIter->data.substr(pos1+1,pos2-pos1-1);

					if (!addedList.empty()) {
						pos1 = 0;
						pos2 = addedList.find(",", pos1);
						while (pos2 != string::npos) {
							string name = trim_copy(addedList.substr(pos1,pos2-pos1));
							uint32_t uid;

							//Search db's bashTagMap for the tag. If not found, add it. Either way, add the UID to the modEntry structure.
							mapPos = FindBashTag(bashTagMap, name);
							if (mapPos != bashTagMap.end()) {										//Tag found in bashTagMap. Get the UID.
								uid = mapPos->first;
							} else {															//Tag not found in bashTagMap. Add it. 
								uint32_t uid = bashTagMap.rbegin()->first + 1;
								bashTagMap.insert(pair<uint32_t,string>(uid,name));
							}

							//We don't know if any regex-matching mods are installed yet, so just add it to a temporary modEntry.
							mod.bashTagsAdded.push_back(uid);

							pos1 = pos2+1;
							pos2 = addedList.find(",", pos1);
						}
					}

					if (!removedList.empty()) {
						pos1 = 0;
						pos2 = removedList.find(",", pos1);
						while (pos2 != string::npos) {
							string name = trim_copy(removedList.substr(pos1,pos2-pos1));
							uint32_t uid;

							//Search db's bashTagMap for the tag. If not found, add it. Either way, add the UID to the modEntry structure.
							mapPos = FindBashTag(bashTagMap, name);
							if (mapPos != bashTagMap.end()) {										//Tag found in bashTagMap. Get the UID.
								uid = mapPos->first;
							} else {															//Tag not found in bashTagMap. Add it. 
								uint32_t uid = bashTagMap.rbegin()->first + 1;
								bashTagMap.insert(pair<uint32_t,string>(uid,name));
							}

							//We don't know if any regex-matching mods are installed yet, so just add it to a temporary modEntry.
							mod.bashTagsRemoved.push_back(uid);

							pos1 = pos2+1;
							pos2 = removedList.find(",", pos1);
						}
					}	
				} else if (messageIter->key == DIRTY) {
					mod.cleaningMessage = messageIter->data;
				}
				++messageIter;
			}

			if (mod.cleaningMessage.empty() && mod.bashTagsAdded.empty() || mod.bashTagsRemoved.empty())
				continue;  //Nothing of interest.

			//Now check to see if it matches any installed mods.
			//Form a regex.
			boost::regex reg(Tidy(iter->name.string())+"(.ghost)?",boost::regex::extended);  //Ghost extension is added so ghosted mods will also be found.
			//Now start looking.
			setPos = hashset.begin();
			do {
				setPos = FindRegexMatch(hashset, reg, setPos);
				if (setPos != hashset.end()) {  //Mod is installed.
					//Check if there is already an existing modEntry in the db masterlistData structure with this plugin name.
					//If there is, don't add this one's info, as BOSS itself only works with the first found and the API should behave the same way.
					vector<modEntry>::iterator dataIter = masterlistData.begin();
					string plugin = iter->name.string();
					while (dataIter != masterlistData.end()) {
						if (Tidy(plugin) == Tidy(dataIter->name))
							break;
						++dataIter;
					}
					//Now if dataIter == masterlistData.end(), the mod does not have an entry, otherwise it does.

					if (dataIter == masterlistData.end()) {  //Interesting info, but no existing modEntry.
						mod.name = plugin;
						masterlistData.push_back(mod);
					}
				}
				++setPos;
			} while (setPos != hashset.end());
		}
		++iter;
	}
	
	//The masterlistData structure now only contains mods that are installed and have Bash Tag suggestions or dirty mod messages attached.
	//The Tag suggestions are listed by their UIDs in vectors according to whether they are being added or removed.
	//The valid dirty mod message is also stored.
	
	//CONVERSION - Userlist
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
					if (pos1 != string::npos)
						pos2 = lineIter->object.find("]", pos1);
					if (pos2 != string::npos)
						removedList = lineIter->object.substr(pos1+1,pos2-pos1-1);

					if (!addedList.empty()) {
						pos1 = 0;
						pos2 = addedList.find(",", pos1);
						while (pos2 != string::npos) {
							string name = trim_copy(addedList.substr(pos1,pos2-pos1));
							uint32_t uid;

							//Search db's bashTagMap for the tag. If not found, add it. Either way, add the UID to the modEntry structure.
							mapPos = FindBashTag(bashTagMap, name);
							if (mapPos != bashTagMap.end()) {										//Tag found in bashTagMap. Get the UID.
								uid = mapPos->first;
							} else {															//Tag not found in bashTagMap. Add it. 
								uint32_t uid = bashTagMap.rbegin()->first + 1;
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
					}

					if (!removedList.empty()) {
						pos1 = 0;
						pos2 = removedList.find(",", pos1);
						while (pos2 != string::npos) {
							string name = trim_copy(removedList.substr(pos1,pos2-pos1));
							uint32_t uid;

							//Search db's bashTagMap for the tag. If not found, add it. Either way, add the UID to the modEntry structure.
							mapPos = FindBashTag(bashTagMap, name);
							if (mapPos != bashTagMap.end()) {										//Tag found in bashTagMap. Get the UID.
								uid = mapPos->first;
							} else {															//Tag not found in bashTagMap. Add it. 
								uint32_t uid = bashTagMap.rbegin()->first + 1;
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
	
	//The userlist has now been adapted so that only the following information is stored:
	// - Mods with Bash Tags added and/or removed.
	// - The UIDs of the Bash Tags added and/or removed for those mods.

	//FREE CURRENT POINTERS
	//Free memory at pointers stored in structure.
	free(db->extTagMap);
	vector<uint32_t*>::iterator bashTagIter = db->extTagIds.begin();
	while (bashTagIter != db->extTagIds.end()) {
		free(*bashTagIter);
		++bashTagIter;
	}
	vector<uint8_t*>::iterator messageIter = db->extMessages.begin();
	while (messageIter != db->extMessages.end()) {
		free(*messageIter);
		++messageIter;
	}
	
	//DB SET
	db->masterlistData = masterlistData;
	db->userlistData = userlistData;
	db->bashTagMap = bashTagMap;	
	return BOSS_API_ERROR_OK;
}


//////////////////////////
// DB Access Functions
//////////////////////////

// Returns an array of the Bash Tags encounterred when loading the masterlist
// and userlist, and the number of tags in the returned array. The array and
// its contents are static and should not be freed by the client.
BOSS_API uint32_t GetBashTagMap (boss_db db, BashTag ** tagMap, uint32_t * numTags) {
	//Check for valid args.
	if (db == 0)
		return BOSS_API_ERROR_INVALID_ARGS;
		
	//Check to see if bashTagMap is already populated.
	if (*tagMap != 0)
		return BOSS_API_ERROR_TAGMAP_EXISTS;

	//Set size.
	*numTags = db->bashTagMap.size();
	//Allocate memory.
	*tagMap = (BashTag*)malloc(sizeof(BashTag) * (*numTags));
	if (*tagMap == 0)
		return BOSS_API_ERROR_NO_MEM;
	db->extTagMap = *tagMap;  //Record address.
	//Loop through internal bashTagMap and fill output elements.
	for (uint32_t i=0;i<*numTags;i++) {
		tagMap[i]->id = i;
		//Allocate memory.
		tagMap[i]->name = (uint8_t*)malloc(sizeof(uint8_t) * db->bashTagMap[i].length());  //Returns null if malloc failed.
		if (tagMap[i]->name == 0)
			return BOSS_API_ERROR_NO_MEM;
		
		uint32_t ret = StdStringToUint8_t(db->bashTagMap[i], &tagMap[i]->name);
		if (ret != BOSS_API_ERROR_OK)
			return ret;
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
	if (db == 0 || modName == 0)
		return BOSS_API_ERROR_INVALID_ARGS;
									
	//Convert modName.
	string mod;
	Uint8_tToStdString(modName, &mod);

	//Initialise pointers to null and zero tag counts.
	*numTags_added = 0;
	*numTags_removed = 0;
	*tagIds_removed = 0;
	*tagIds_added = 0;
	*userlistModified = false;

	//Need to search masterlist then userlist separately, check each mod case-insensitively for a match.
	size_t size;
	vector<modEntry>::iterator iter;
	vector<uint32_t> masterlist_tagsAdded, masterlist_tagsRemoved, userlist_tagsAdded, userlist_tagsRemoved;

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
	*numTags_added = masterlist_tagsAdded.size() + userlist_tagsAdded.size();
	*numTags_removed = masterlist_tagsRemoved.size() + userlist_tagsRemoved.size();

	//Allocate memory.
	*tagIds_added = (uint32_t*)malloc(sizeof(uint32_t) * (*numTags_added));
	if (*tagIds_added == 0)
		return BOSS_API_ERROR_NO_MEM;
	db->extTagIds.push_back(*tagIds_added);  //Record address.
	*tagIds_removed = (uint32_t*)malloc(sizeof(uint32_t) * (*numTags_removed));
	if (*tagIds_removed == 0)
		return BOSS_API_ERROR_NO_MEM;
	db->extTagIds.push_back(*tagIds_removed);  //Record address.
	

	//Loop through vectors and fill output elements.
	size = masterlist_tagsAdded.size();
	for (size_t i=0;i<size;i++)
		*tagIds_added[i] = masterlist_tagsAdded[i];
	for (size_t i=0;i<*numTags_added;i++)
		*tagIds_added[i] = userlist_tagsAdded[i-size];

	size = masterlist_tagsRemoved.size();
	for (size_t i=0;i<size;i++)
		*tagIds_removed[i] = masterlist_tagsRemoved[i];
	for (size_t i=size;i<*numTags_removed;i++)
		*tagIds_removed[i] = userlist_tagsRemoved[i-size];

	//To work to myk002's specs, the pointers to the output info need to be stored internally so that the memory can later be freed.
	return BOSS_API_ERROR_OK;
}

// Returns the message associated with a dirty mod and whether the mod needs
// cleaning. If a mod has no dirty mmessage, *message will be NULL. modName is
// case-insensitive. The return values for needsCleaning are:
//   0 == no
//   1 == yes
//   2 == unknown
// The message string is valid until the db is destroyed or until a Load
// function is called. The string should not be freed by the client.
BOSS_API uint32_t GetDirtyMessage (boss_db db, const uint8_t * modName, 
									uint8_t ** message, uint32_t * needsCleaning) {
	//Check for valid args.
	if (db == 0 || modName == 0)
		return BOSS_API_ERROR_INVALID_ARGS;
									
	//Convert modName.
	string mod;
	Uint8_tToStdString(modName, &mod);
	mod = Tidy(mod);

	//Initialise pointers.
	*message = 0;
	*needsCleaning = 2;

	//Search masterlist.
	vector<modEntry>::iterator iter = db->masterlistData.begin();
	while (iter != db->masterlistData.end()) {
		if (Tidy(iter->name) == Tidy(mod) && !iter->cleaningMessage.empty()) {  //Mod found and it has a message.
			//Allocate memory.
			*message = (uint8_t*)malloc(sizeof(uint8_t) * iter->cleaningMessage.length());  //Returns null if malloc failed.
			if (*message == 0)
				return BOSS_API_ERROR_NO_MEM;
			uint32_t ret = StdStringToUint8_t(iter->cleaningMessage, message);
			if (ret != BOSS_API_ERROR_OK)
				return ret;
			if (iter->cleaningMessage.find("Do not clean.") != string::npos)  //Mod should not be cleaned.
				*needsCleaning = 0;
			else  //Mod should be cleaned.
				*needsCleaning = 1;
			break;
		}
		++iter;
	}

	//To work to myk002's specs, the pointers to the output info need to be stored internally so that the memory can later be freed.
	return BOSS_API_ERROR_OK;
}

// Writes a minimal masterlist that only contains mods that have Bash Tag suggestions, 
// plus the Tag suggestions themselves, in order to create the Wrye Bash taglist.
// outputFile is the path to use for output. If outputFile already exists, it will
// only be overwritten if overwrite is true.
BOSS_API uint32_t DumpMinimal (boss_db db, const uint8_t * outputFile, bool overwrite) {
	//Check for valid args.
	if (db == 0 || outputFile == 0)
		return BOSS_API_ERROR_INVALID_ARGS;

	string path;
	Uint8_tToStdString(outputFile, &path);
	if (!fs::exists(path) || overwrite) {
		//Convert masterlistData back to MF2 masterlist syntax and output.
		ofstream mlist(path.c_str());
		if (mlist.fail())
			return BOSS_ERROR_FILE_WRITE_FAIL;
		else {
			vector<modEntry>::iterator iter = db->userlistData.begin();
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
						}
						mlist << "]";
					}
				}
				++iter;
			}
			mlist.close();
		}
		return BOSS_API_ERROR_OK;
	} else
		return BOSS_API_ERROR_OVERWRITE_FAIL;
}


#ifdef __cplusplus
}
#endif