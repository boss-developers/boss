//
// BOSSAPI.cpp
//
// <copyright header>


#include "BOSS-API.h"
#include "BOSS-Common.h"
#include <boost/algorithm/string.hpp>
#include <boost/unordered_map.hpp>

using namespace std;
using namespace boss;

using boost::algorithm::trim_copy;

#ifdef __cplusplus
extern "C"
{
#endif

struct modEntry {
	string name;						//Plugin filename
	vector<uint32_t> bashTagsAdded;		//Vector of Bash Tags added unique IDs.
	vector<uint32_t> bashTagsRemoved;	//Vector of Bash Tags removed unique IDs.
	string cleaningMessage;				//Doesn't need to be a vector because there will only be one valid cleaning message outputted by the parser.
};

/*Stuff that needs to be stored in memory while db is live:
	masterlist data
	userlist data
	Bash Tag map (internal)

*/

// internal definition of state
struct _boss_db_int {
	vector<modEntry> userlistData, masterlistData;		//A structure containing all the mods with Bash Tags added or removed by the userlist, referenced by unique IDs.
	map<uint32_t,string> BashTagMap;					//A hashmap containing all the Bash Tags found in the masterlist and userlist and their unique IDs.
														//Ordered to make ensuring UIDs easy (check the UID of the last element then increment). Strings are case-sensitive.

	map<uint32_t, string>::iterator FindBashTag(string value) {
		map<uint32_t, string>::iterator mapPos = BashTagMap.begin();
		while (mapPos != BashTagMap.end()) {
			if (mapPos->second == value)
				break;
		}
		return mapPos;
	}

	string GetTagString(uint32_t uid) {
		map<uint32_t, string>::iterator mapPos = BashTagMap.find(uid);
		if (mapPos != BashTagMap.end())
			return mapPos->second;
		else
			return "";
	}
};

// Variable conversions.
// Need:
//  std::string -> uint8_t*
//  uint8_t* -> std::string

uint32_t StdStringToUint8_t(const string str, uint8_t ** ostr) {
	uint32_t length = str.length();
	//Allocate memory.
	*ostr = (uint8_t*)malloc(sizeof(uint8_t) * length);  //Returns null if malloc failed.
	if (*ostr == 0)
		return BOSS_ERROR_NO_MEM;
	//Fill memory allocated.
	for (uint32_t i=0;i<length;i++) {
		*ostr[i] = str[i];
	}
	return BOSS_ERROR_SUCCESS;
}

uint32_t Uint8_tToStdString(const uint8_t * str, string * ostr) {
	uint32_t length = sizeof(str) / sizeof(uint8_t);
	for (uint32_t i=0;i<length;i++) {
		ostr[i] = str[i];
	}
	return BOSS_ERROR_SUCCESS;
}


BOSS_API const uint32_t BOSS_ERROR_SUCCESS = 0;
BOSS_API const uint32_t BOSS_ERROR_BAD_ARGUMENT = 1;
BOSS_API const uint32_t BOSS_ERROR_FILE_NOT_FOUND = 2;
BOSS_API const uint32_t BOSS_ERROR_FILE_ACCESS_DENIED = 3;
BOSS_API const uint32_t BOSS_ERROR_FILE_INVALID_SYNTAX = 4;
BOSS_API const uint32_t BOSS_ERROR_NO_MEM = 5;
BOSS_API const uint32_t BOSS_ERROR_INTERNAL = 6;
// ...
BOSS_API const uint32_t BOSS_ERROR_MAX = 6;


//////////////////////////////
// version functions

// returns whether this version of the BOSS library supports the API from the
// given BOSS library version.  this abstracts the BOSS API stability policy
// away from clients that use this library.
BOSS_API bool IsCompatibleVersion (uint32_t bossVersionMajor, uint32_t bossVersionMinor) {
	//The 1.9 API is backwards compatible with all 1.x versions of BOSS.
	if (bossVersionMajor < 2 && bossVersionMajor < 10)
		return true;
	else
		return false;
}

// returns the version string for the library in utf-8, used for display
// the string exists for the lifetime of the library
BOSS_API uint32_t GetVersionString (uint8_t ** bossVersionStr) {
	StdStringToUint8_t(g_version, bossVersionStr);
}


//////////////////////////////
// data definition functions

// returns an array of BashTags and the number of tags in the returned array
// the array and its contents are static and should not be freed by the client
BOSS_API void GetBashTagMap (boss_db db, BashTag ** tagMap, uint32_t * numTags) {
	//Set size.
	*numTags = db->BashTagMap.size();
	//Allocate memory.
	*tagMap = (BashTag*)malloc(sizeof(BashTag) * (*numTags));

	//Loop through internal BashTagMap and fill output elements.
	for (uint32_t i=0;i<*numTags;i++) {
		tagMap[i]->id = i;
		StdStringToUint8_t(db->BashTagMap[i], &tagMap[i]->name);
	}
}


////////////////////////////////////
// lifecycle management functions

// explicitly manage the lifetime of the database.  this way the client can
// free up the memory when it wants/needs to, for example when the process is
// low on memory.
BOSS_API uint32_t CreateBossDb  (boss_db * db) {
	*db = new _boss_db_int;
}
BOSS_API void     DestroyBossDb (boss_db db) {
	delete db;
}

/////////////////////////////////////
// path setting/getting functions.

// These functions are required to interface with the BOSS internal code correctly and
// avoid any issues that may be caused by not globally setting the correct paths.

BOSS_API uint32_t GetDataPath(uint8_t * path) {
	StdStringToUint8_t(data_path.string(),&path);
}

BOSS_API uint32_t GetBOSSPath(uint8_t * path) {
	StdStringToUint8_t(boss_path.string(),&path);
}

BOSS_API uint32_t GetMasterlistFilename(uint8_t * filename) {
	StdStringToUint8_t(masterlist_path.filename().string(),&filename);
}

BOSS_API uint32_t SetDataPath(const uint8_t * path) {
	string str;
	Uint8_tToStdString(path, &str);
	data_path = fs::path(str);
}

BOSS_API uint32_t SetBOSSPath(const uint8_t * path) {
	string str;
	Uint8_tToStdString(path, &str);
	boss_path = fs::path(str);
}

BOSS_API uint32_t SetMasterlistFilename(const uint8_t * filename) {
	string str;
	Uint8_tToStdString(filename, &str);
	masterlist_path = boss_path / fs::path(str);
}


/////////////////////////////////////
// database loading functions.

// can be called multiple times.  if masterlist is loaded, then userlist is
// loaded, then masterlist is loaded again, the previously-loaded userlist
// should still be applied over the new masterlist.  path strings are in
// UTF-8.  on error, database is expected to be unchanged.  path is case
// sensitive if the underlying file system is case sensitive


// All that matters for the API is what mods have what messages. Load order doesn't matter.
// Currently requested functions are only interested in Bash Tag suggestions and dirty mod messages, but 
// incompatibilities and requirements also have a specific syntax option so are candidates for providing
// useful info. It may be useful to keep them.

// In order to keep the size of structures down, the following items should be stripped from the parsed masterlist:
// - group begin/end lines
// - mods with no BTS or dirty mod messages.

// We can't use existing functions to trim down the masterlist further, as they check what is referenced in the userlist,
// which may not be loaded at masterlist loading time. However, the userlist doesn't need to be referenced as we are not
// interested in mods the masterlist doesn't know about, as we only want masterlist mod message info in the masterlist. 
// If a user has added a mod in their userlist and attached tags to it in the userlist, then the tags will show up in
// the userlist data structure.

// Furthermore, we need to be aware of what mods the user has in order to interpret masterlist regex. The user's modlist
// is unused other than for this purpose though, so does not need to be stored in the db structure.

// Along the same lines, sorting lines don't matter in user rules, so should be stripped from the parsed userlist.

BOSS_API uint32_t LoadMasterlist (boss_db db) {
	vector<item> masterlist;

	//Parse masterlist.
	try {
		parseMasterlist(masterlist_path, masterlist);
	} catch (boss_error e) {
		/*e.getString()*/
		/*e.getCode()*/
	}

	if (!masterlistErrorBuffer.empty()) {
		//Errors
	}

	//Build modlist.
	std::vector<boss::item> modlist;
	BuildModlist(modlist);

	//Now trim masterlist

	//Add all installed mods to hashset.
	boost::unordered_set<string> hashset;  //Holds mods for checking against masterlist
	boost::unordered_set<string>::iterator setPos;
	size_t size = modlist.size();
	for (size_t i=0; i<size; i++) {
		if (modlist[i].type == MOD)
			hashset.insert(Tidy(modlist[i].name.string()));
	}

	//Now compare masterlist against hashset. Anything worth keeping should go in the holdingVec.
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
			vector<modEntry>::iterator dataIter = db->masterlistData.begin();
			string plugin = Tidy(iter->name.string());

			//Check if there is already an existing modEntry in the db userlistData structure with this plugin name.
			while (dataIter != db->masterlistData.end()) {
				if (plugin == dataIter->name)
					break;
				++dataIter;
			}
			//Now if dataIter == db->masterlistData.end(), the mod does not have an entry, otherwise it does.

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
							mapPos = db->FindBashTag(name);
							if (mapPos != db->BashTagMap.end()) {										//Tag found in BashTagMap. Get the UID.
								uid = mapPos->first;
							} else {															//Tag not found in BashTagMap. Add it. 
								uint32_t uid = db->BashTagMap.rbegin()->first + 1;
								db->BashTagMap.insert(pair<uint32_t,string>(uid,name));
							}

							//Now if there is already an entry, add Tag to that. Otherwise, add Tag to "mod".
							//If there is existing modEntry in the db masterlistData structure, don't add this one's info.
							//BOSS itself only works with the first found and the API should behave the same way.
							if (dataIter == db->masterlistData.end())  //Mod already exists in structure. Tags should be added to it.
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

							//Search db's BashTagMap for the tag. If not found, add it. Either way, add the UID to the modEntry structure.
							mapPos = db->FindBashTag(name);
							if (mapPos != db->BashTagMap.end()) {										//Tag found in BashTagMap. Get the UID.
								uid = mapPos->first;
							} else {															//Tag not found in BashTagMap. Add it. 
								uint32_t uid = db->BashTagMap.rbegin()->first + 1;
								db->BashTagMap.insert(pair<uint32_t,string>(uid,name));
							}

							//Now if there is already an entry, add Tag to that. Otherwise, add Tag to "mod".
							//If there is existing modEntry in the db masterlistData structure, don't add this one's info.
							//BOSS itself only works with the first found and the API should behave the same way.
							if (dataIter == db->masterlistData.end())
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
				db->masterlistData.push_back(mod);
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

							//Search db's BashTagMap for the tag. If not found, add it. Either way, add the UID to the modEntry structure.
							mapPos = db->FindBashTag(name);
							if (mapPos != db->BashTagMap.end()) {										//Tag found in BashTagMap. Get the UID.
								uid = mapPos->first;
							} else {															//Tag not found in BashTagMap. Add it. 
								uint32_t uid = db->BashTagMap.rbegin()->first + 1;
								db->BashTagMap.insert(pair<uint32_t,string>(uid,name));
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

							//Search db's BashTagMap for the tag. If not found, add it. Either way, add the UID to the modEntry structure.
							mapPos = db->FindBashTag(name);
							if (mapPos != db->BashTagMap.end()) {										//Tag found in BashTagMap. Get the UID.
								uid = mapPos->first;
							} else {															//Tag not found in BashTagMap. Add it. 
								uint32_t uid = db->BashTagMap.rbegin()->first + 1;
								db->BashTagMap.insert(pair<uint32_t,string>(uid,name));
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
				if (setPos == hashset.end())  //Exit if the mod hasn't been found.
					break;
				//Look for mod in modlist. Replace with case-preserved mod name.
				size_t pos = GetModPos(modlist,*setPos);
				if (pos != (size_t)-1) {
					//Now we need to do all the adding.
					
					//Check if there is already an existing modEntry in the db masterlistData structure with this plugin name.
					//If there is, don't add this one's info, as BOSS itself only works with the first found and the API should behave the same way.
					vector<modEntry>::iterator dataIter = db->masterlistData.begin();
					string plugin = Tidy(iter->name.string());
					while (dataIter != db->masterlistData.end()) {
						if (plugin == dataIter->name)
							break;
						++dataIter;
					}
					//Now if dataIter == db->masterlistData.end(), the mod does not have an entry, otherwise it does.

					if (dataIter == db->masterlistData.end()) {  //Interesting info, but no existing modEntry.
						mod.name = plugin;
						db->masterlistData.push_back(mod);
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
}

BOSS_API uint32_t LoadUserlist   (boss_db db) {
	vector<rule> userlist;
	map<uint32_t, string>::iterator mapPos;

	//Parse userlist.
	try {
		bool parsed = parseUserlist(userlist_path,userlist);
		if (!parsed)
			userlist.clear();  //If userlist has parsing errors, empty it so no rules are applied.
	} catch (boss_error e) {
		/*e.getString()*/
		/*e.getCode()*/
	}
	
	//Now we need to trim down the userlist.
	vector<rule>::iterator iter = userlist.begin();
	while (iter != userlist.end()) {
		vector<line>::iterator lineIter = iter->lines.begin();
		vector<modEntry>::iterator dataIter = db->userlistData.begin();
		string plugin = Tidy(iter->ruleObject);
		modEntry mod;

		//Check if there is already an existing modEntry in the db userlistData structure with this plugin name.
		while (dataIter != db->userlistData.end()) {
			if (plugin == dataIter->name)
				break;
			++dataIter;
		}
		//Now if dataIter == db->userlistData.end(), the mod does not have an entry, otherwise it does.

		while (lineIter != iter->lines.end()) {
			if (lineIter->key == APPEND || lineIter->key == REPLACE) {
				//Need to check if the message added is a Bash Tag suggestion.
				char sym = lineIter->object[0];  //Look for message symbol. (MF1)
				size_t pos = lineIter->object.find(":"); //Look for separator colon. (MF2)
				keyType key;
				if (pos != string::npos)
					key = StringToKey(Tidy(lineIter->object.substr(0,pos)));  //MF2 keyword.

				if (sym == '%' || key == TAG) {  //It's a Bash Tag suggestion.
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

							//Search db's BashTagMap for the tag. If not found, add it. Either way, add the UID to the modEntry structure.
							mapPos = db->FindBashTag(name);
							if (mapPos != db->BashTagMap.end()) {										//Tag found in BashTagMap. Get the UID.
								uid = mapPos->first;
							} else {															//Tag not found in BashTagMap. Add it. 
								uint32_t uid = db->BashTagMap.rbegin()->first + 1;
								db->BashTagMap.insert(pair<uint32_t,string>(uid,name));
							}

							//Now if there is already an entry, add Tag to that. Otherwise, add Tag to "mod".
							if (dataIter != db->userlistData.end())  //Mod already exists in structure. Tags should be added to it.
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

							//Search db's BashTagMap for the tag. If not found, add it. Either way, add the UID to the modEntry structure.
							mapPos = db->FindBashTag(name);
							if (mapPos != db->BashTagMap.end()) {										//Tag found in BashTagMap. Get the UID.
								uid = mapPos->first;
							} else {															//Tag not found in BashTagMap. Add it. 
								uint32_t uid = db->BashTagMap.rbegin()->first + 1;
								db->BashTagMap.insert(pair<uint32_t,string>(uid,name));
							}

							//Now if there is already an entry, add Tag to that. Otherwise, add Tag to "mod".
							if (dataIter != db->userlistData.end())  //Mod already exists in structure. Tags should be added to it.
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
			db->userlistData.push_back(mod);
		}
		++iter;
	}
	//The userlist has now been adapted so that only the following information is stored:
	// - Mods with Bash Tags added and/or removed.
	// - The UIDs of the Bash Tags added and/or removed for those mods.
}


/////////////////////////////////////
// db access functions

// returns an array of tagIds and the number of tags.  if there are no tags,
// *tagIds can be NULL.  modName is encoded in utf-8 and is not case sensitive
// the returned arrays are valid until the db is destroyed or until a Load
// function is called.  The arrays should not be freed by the client.
BOSS_API uint32_t GetModBashTags (boss_db db, const uint8_t * modName, uint32_t ** tagIds_added, uint32_t * numTags_added, uint32_t **tagIds_removed, uint32_t *numTags_removed) {
	//Convert modName.
	string mod;
	Uint8_tToStdString(modName, &mod);
	mod = Tidy(mod);

	//Initialise pointers to null and zero tag counts.
	*numTags_added = 0;
	*numTags_removed = 0;
	*tagIds_removed = 0;
	*tagIds_added = 0;

	//Need to search masterlist then userlist separately, check each mod case-insensitively for a match.
	size_t size;
	vector<modEntry>::iterator iter;
	vector<uint32_t> masterlist_tagsAdded, masterlist_tagsRemoved, userlist_tagsAdded, userlist_tagsRemoved;

	//Masterlist
	iter = db->masterlistData.begin();
	while (iter != db->masterlistData.end()) {
		if (iter->name == mod) {  //Mod found.
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
		if (iter->name == mod) {  //Mod found.
			//Add UIDs of tags added and removed to outputs.
			userlist_tagsAdded = iter->bashTagsAdded;
			userlist_tagsRemoved = iter->bashTagsRemoved;
			break;
		}
		++iter;
	}

	//Now combine the masterlist and userlist Tags.
	*numTags_added = masterlist_tagsAdded.size() + userlist_tagsAdded.size();
	*numTags_removed = masterlist_tagsRemoved.size() + userlist_tagsRemoved.size();

	//Allocate memory.
	*tagIds_added = (uint32_t*)malloc(sizeof(uint32_t) * (*numTags_added));
	*tagIds_removed = (uint32_t*)malloc(sizeof(uint32_t) * (*numTags_removed));

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
}

// returns the message associated with a dirty mod and whether the mod needs
// cleaning.  if a mod has no dirty message, *message should be NULL.  modName
// is encoded in utf-8 and is not case sensitive.  message, if not NULL, must
// be encoded in utf-8
// needsCleaning:
//   0 == no
//   1 == yes
//   2 == unknown
// the message string is valid until the db is destroyed or until a Load
// function is called.  the string should not be freed by the client.
BOSS_API uint32_t GetDirtyMessage (boss_db db, const uint8_t * modName, uint8_t ** message, uint32_t * needsCleaning) {
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
		if (iter->name == mod && !iter->cleaningMessage.empty()) {  //Mod found and it has a message.
			StdStringToUint8_t(iter->cleaningMessage, message);
			if (iter->cleaningMessage.find("Do not clean.") != string::npos)  //Mod should not be cleaned.
				*needsCleaning = 0;
			else  //Mod should be cleaned.
				*needsCleaning = 1;
			break;
		}
		++iter;
	}

	//To work to myk002's specs, the pointers to the output info need to be stored internally so that the memory can later be freed.
}

// writes out a minimal masterlist that only contains mods that have tags (plus
// the tags themselves) in order to create the Wrye Bash taglist.  outputFile
// is a UTF-8 string specifying the path to use for output.  if it already
// exists, outputFile will be overwritten iff overwrite is true
BOSS_API uint32_t DumpTags (boss_db db, const uint8_t * outputFile, bool overwrite) {
	string path;
	Uint8_tToStdString(outputFile, &path);
	if (!fs::exists(path) || overwrite) {
		//Convert masterlistData back to MF2 masterlist syntax and output.
		ofstream mlist(path.c_str());
		if (mlist.fail())
			return 1;  //Replace with "file write failed" error code.
		else {
			vector<modEntry>::iterator iter = db->userlistData.begin();
			while (iter != db->masterlistData.end()) {
				
				//Output mod line.
				mlist << iter->name << endl;

				//Now output dirty message line, if it exists.
		//		if (!iter->cleaningMessage.empty())
		//			mlist << "DIRTY: " << iter->cleaningMessage << endl;

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
	} else
		return 1;  //Replace with "file already exists" error code.
}


#ifdef __cplusplus
}
#endif