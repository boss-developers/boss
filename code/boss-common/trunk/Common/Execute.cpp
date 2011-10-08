/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 3184 $, $Date: 2011-08-26 20:52:13 +0100 (Fri, 26 Aug 2011) $
*/

#include "Common/Execute.h"
#include "Parsing/Parser.h"
#include "Common/Error.h"
#include "Common/Globals.h"
#include "Support/Helpers.h"
#include "Support/Logger.h"
#include "Output/Output.h"

#include <boost/algorithm/string.hpp>

namespace boss {
	using namespace std;

	using boost::algorithm::trim_copy;
	using boost::algorithm::to_lower_copy;

	summaryCounters::summaryCounters() {
		recognised = 0;
		unrecognised = 0;
		ghosted = 0;
		messages = 0;
		warnings = 0;
		errors = 0;
	}

	bosslogContents::bosslogContents() {
		generalMessages = "";
		summary = "";
		userlistMessages = "";
		seInfo = "";
		recognisedPlugins = "";
		unrecognisedPlugins = "";
	}

	//Removes the ".ghost" extension from ghosted filenames.
	string TrimDotGhost(string plugin) {
		fs::path pluginPath(plugin);
		const string ext = to_lower_copy(pluginPath.extension().string());
		if (ext == ".ghost")
			return plugin.substr(0,plugin.length()-6);
		else
			return plugin;
	}

	//Searches a hashset for the first matching string of a regex and returns its iterator position.
	BOSS_COMMON_EXP boost::unordered_set<string>::iterator FindRegexMatch(const boost::unordered_set<string> set, const boost::regex reg, boost::unordered_set<string>::iterator startPos) {
		while(startPos != set.end()) {
			string mod = *startPos;
			if (boost::regex_match(mod,reg))
				return startPos;
			++startPos;
		}
		return set.end();
	}

	//Record recognised mod list from last HTML BOSSlog generated.
	BOSS_COMMON_EXP string GetOldRecognisedList(const fs::path log) {
		size_t pos1, pos2;
		string result;
		fileToBuffer(log,result);
		pos1 = result.find("<ul id='recognised'>");
		if (pos1 != string::npos)
			pos2 = result.find("<h3", pos1+20);
		if (pos2 != string::npos)
			pos2 = result.rfind("</ul>",pos2);
		if (pos2 != string::npos)
			result = result.substr(pos1+20, pos2-pos1-20);
		return result;
	}

	//Detect the game BOSS is installed for.
	//1 = Oblivion, 2 = Fallout 3, 3 = Nehrim, 4 = Fallout: New Vegas, 5 = Skyrim. Throws exception if error.
	BOSS_COMMON_EXP void GetGame() {
		if (fs::exists(data_path / "Oblivion.esm")) {
			if (fs::exists(data_path / "Nehrim.esm"))
				throw boss_error(BOSS_ERROR_OBLIVION_AND_NEHRIM);
			game = 1;
		} else if (fs::exists(data_path / "Nehrim.esm")) 
			game = 3;
		else if (fs::exists(data_path / "FalloutNV.esm")) 
			game = 4;
		else if (fs::exists(data_path / "Fallout3.esm")) 
			game = 2;
		else if (fs::exists(data_path / "Skyrim.esm")) 
			game = 5;
		else
			throw boss_error(BOSS_ERROR_NO_MASTER_FILE);
	}

	//Gets the string representation of the detected game.
	BOSS_COMMON_EXP string GetGameString() {
		if (game == 1)
			return "TES IV: Oblivion";
		else if (game == 2)
			return "Fallout 3";
		else if (game == 3)
			return "Nehrim - At Fate's Edge";
		else if (game == 4)
			return "Fallout: New Vegas";
		else if (game == 5)
			return "TES V: Skyrim";
		else
			return "Game Not Detected";
	}

	//Gets the timestamp of the game's master file. Throws exception if error.
	BOSS_COMMON_EXP time_t GetMasterTime() {
		try {
			if (game == 1) 
				return fs::last_write_time(data_path / "Oblivion.esm");
			else if (game == 2) 
				return fs::last_write_time(data_path / "Fallout3.esm");
			else if (game == 3) 
				return fs::last_write_time(data_path / "Nehrim.esm");
			else if (game == 4) 
				return fs::last_write_time(data_path / "FalloutNV.esm");
			else if (game == 5) 
				return fs::last_write_time(data_path / "Skyrim.esm");
			else
				throw boss_error(BOSS_ERROR_NO_MASTER_FILE);
		} catch(fs::filesystem_error e) {
			throw boss_error(BOSS_ERROR_FS_FILE_MOD_TIME_READ_FAIL, GameMasterFile(), e.what());
		}
	}

	//Returns the expeccted master file.
	BOSS_COMMON_EXP string GameMasterFile() {
		if (game == 1) 
			return "Oblivion.esm";
		else if (game == 2) 
			return "Fallout3.esm";
		else if (game == 3) 
			return "Nehrim.esm";
		else if (game == 4) 
			return "FalloutNV.esm";
		else if (game == 5) 
			return "Skyrim.esm";
		else
			return "Game Not Detected";
	}

	//Create a modlist containing all the mods that are installed or referenced in the userlist with their masterlist messages.
	//Returns the vector position of the last recognised mod in modlist.
	BOSS_COMMON_EXP size_t BuildWorkingModlist(vector<item>& modlist, vector<item> masterlist, const vector<rule>& userlist) {
		//Add all modlist and userlist mods to a hashset to optimise comparison against masterlist.
		boost::unordered_set<string> hashset;  //Holds mods for checking against masterlist
		boost::unordered_set<string>::iterator setPos;
		size_t lastRecognisedPos;

		size_t size;
		size_t userlistSize = userlist.size();
		size_t linesSize;
		/* Hashset must be a set of unique mods.
		Ghosted mods take priority over non-ghosted mods, as they are specifically what is installed. 
		*/

		LOG_INFO("Populating hashset with modlist.");
		size = modlist.size();
		for (size_t i=0; i<size; i++) {
			if (modlist[i].type == MOD)
				hashset.insert(Tidy(modlist[i].name.string()));
		}
		LOG_INFO("Populating hashset with userlist.");
		for (size_t i=0; i<userlistSize; i++) {
			if (IsPlugin(userlist[i].ruleObject)) {
				setPos = hashset.find(Tidy(userlist[i].ruleObject));
				if (setPos == hashset.end()) {  //Mod not already in hashset.
					setPos = hashset.find(Tidy(userlist[i].ruleObject + ".ghost"));
					if (setPos == hashset.end()) {  //Ghosted mod not already in hashset. 
						//Unique plugin, so add to hashset.
						hashset.insert(Tidy(userlist[i].ruleObject));
					}
				}
			}
			linesSize = userlist[i].lines.size();
			for (size_t j=0; j<linesSize; j++) {
				if (IsPlugin(userlist[i].lines[j].object)) {
					setPos = hashset.find(Tidy(userlist[i].lines[j].object));
					if (setPos == hashset.end()) {  //Mod not already in hashset.
						setPos = hashset.find(Tidy(userlist[i].lines[j].object + ".ghost"));
						if (setPos == hashset.end()) {  //Ghosted mod not already in hashset. 
							//Unique plugin, so add to hashset.
							hashset.insert(Tidy(userlist[i].lines[j].object));
						}
					}
				}
			}
		}

		//Now compare masterlist against hashset.
		vector<item>::iterator iter = masterlist.begin();
		vector<item> holdingVec;
		boost::unordered_set<string>::iterator addedPos;
		boost::unordered_set<string> addedMods;
		size_t pos;
		LOG_INFO("Comparing hashset against masterlist.");
		while (iter != masterlist.end()) {
			if (iter->type == MOD) {
				//Check to see if the mod is in the hashset. If it is, or its ghosted version is, also check if 
				//the mod is already in the holding vector. If not, add it.
				setPos = hashset.find(Tidy(iter->name.string()));
				if (setPos != hashset.end()) {										//Mod found in hashset. 
					addedPos = addedMods.find(Tidy(iter->name.string()));
					if (addedPos == addedMods.end()) {								//The mod is not already in the holding vector.
						holdingVec.push_back(*iter);									//Record it in the holding vector.
						pos = GetModPos(modlist,iter->name.string());				//Also remove it from the modlist.
						if (pos != (size_t)-1)
							modlist.erase(modlist.begin()+pos);
						addedMods.insert(Tidy(iter->name.string()));
					}
				} else {
					//Mod not found. Look for ghosted mod.
					iter->name = fs::path(iter->name.string() + ".ghost");		//Add ghost extension to mod name.
					setPos = hashset.find(Tidy(iter->name.string()));
					if (setPos != hashset.end()) {									//Mod found in hashset.
						addedPos = addedMods.find(Tidy(iter->name.string()));
						if (addedPos == addedMods.end()) {							//The mod is not already in the holding vector.
							holdingVec.push_back(*iter);								//Record it in the holding vector.
							pos = GetModPos(modlist,iter->name.string());			//Also remove it from the modlist.
							if (pos != (size_t)-1)
								modlist.erase(modlist.begin()+pos);
							addedMods.insert(Tidy(iter->name.string()));
						}
					}
				}
			} else if (iter->type == REGEX) {
				//Form a regex.
				boost::regex reg(Tidy(iter->name.string())+"(.ghost)?",boost::regex::extended);  //Ghost extension is added so ghosted mods will also be found.
				//Now start looking.
				setPos = hashset.begin();
				do {
					setPos = FindRegexMatch(hashset, reg, setPos);
					if (setPos == hashset.end())  //Exit if the mod hasn't been found.
						break;
					string mod = *setPos;
					//Look for mod in modlist, and userlist. Replace with case-preserved mod name.
					pos = GetModPos(modlist,mod);
					if (pos != (size_t)-1)
						mod = modlist[pos].name.string();
					else {
						for (size_t i=0; i<userlistSize; i++) {
							linesSize = userlist[i].lines.size();
							for (size_t j=0; j<linesSize; j++) {
								if (Tidy(userlist[i].lines[j].object) == mod)
									mod = userlist[i].lines[j].object;
							}
						}
					}
					//Check that the mod hasn't already been added to the holding vector.
					addedPos = addedMods.find(Tidy(mod));
					if (addedPos == addedMods.end()) {							//The mod is not already in the holding vector.
						//Now do the adding/removing.
						//Create new temporary item to hold current found mod.
						item tempItem = *iter;
						tempItem.type = MOD;
						tempItem.name = fs::path(mod);
						holdingVec.push_back(tempItem);							//Record it in the holding vector.
						pos = GetModPos(modlist,mod);							//Also remove it from the modlist.
						if (pos != (size_t)-1)
							modlist.erase(modlist.begin()+pos);
						addedMods.insert(Tidy(mod));
					}
					++setPos;
				} while (setPos != hashset.end());
			} else //Group lines must stay recorded.
				holdingVec.push_back(*iter);
			++iter;
		}
		masterlist = holdingVec;  //Masterlist now only contains the items needed to sort the user's mods.
		lastRecognisedPos = masterlist.size()-1;  //Record position of last sorted mod.

		//Add modlist's mods to masterlist, then set the modlist to the masterlist as that's the output..
		masterlist.insert(masterlist.end(),modlist.begin(),modlist.end());
		modlist = masterlist;
		return lastRecognisedPos;
	}

	//Applies the userlist rules to the working modlist.
	BOSS_COMMON_EXP void ApplyUserRules(vector<item>& modlist, const vector<rule>& userlist, string& ouputBuffer, size_t& lastRecognisedPos) {
		size_t userlistSize = userlist.size();
		//Apply rules, one rule at a time, one line at a time.
		LOG_INFO("Starting userlist sort process... Total %" PRIuS " user rules statements to process.", userlistSize);
		for (size_t i=0; i<userlistSize; i++) {
			LOG_DEBUG(" -- Processing rule #%" PRIuS ".", i+1);
			if (!userlist[i].enabled)
				continue;
			size_t linesSize = userlist[i].lines.size();
			for (size_t j=0; j<linesSize; j++) {
				//A mod sorting rule.
				if ((userlist[i].lines[j].key == BEFORE || userlist[i].lines[j].key == AFTER) && IsPlugin(userlist[i].lines[j].object)) {
					size_t index1,index2;
					item mod;
					index1 = GetModPos(modlist,userlist[i].ruleObject);  //Find the rule mod in the modlist.
					//Do checks/increments.
					if (userlist[i].ruleKey == ADD && index1 > lastRecognisedPos) 
						lastRecognisedPos++;
					else if (userlist[i].ruleKey == ADD && index1 == (size_t)-1) {
						ouputBuffer += "<li class='warn'>\""+EscapeHTMLSpecial(userlist[i].ruleObject)+"\" is not installed or in the masterlist. Rule skipped.";
						LOG_WARN(" * \"%s\" is not installed.", userlist[i].ruleObject.c_str());
						break;
					//If it adds a mod already sorted, skip the rule.
					} else if (userlist[i].ruleKey == ADD && index1 <= lastRecognisedPos) {
						ouputBuffer += "<li class='warn'>\""+EscapeHTMLSpecial(userlist[i].ruleObject)+"\" is already in the masterlist. Rule skipped.";
						LOG_WARN(" * \"%s\" is already in the masterlist.", userlist[i].ruleObject.c_str());
						break;
					} else if (userlist[i].ruleKey == OVERRIDE && (index1 > lastRecognisedPos || index1 == (size_t)-1)) {
						ouputBuffer += "<li class='error'>\""+EscapeHTMLSpecial(userlist[i].ruleObject)+"\" is not in the masterlist, cannot override. Rule skipped.";
						LOG_WARN(" * \"%s\" is not in the masterlist, cannot override.", userlist[i].ruleObject.c_str());
						break;
					}
					mod = modlist[index1];  //Record the rule mod in a new variable.
					//Remove the rule mod from its current position.
					modlist.erase(modlist.begin()+index1);
					//Find the sort mod in the modlist.
					index2 = GetModPos(modlist,userlist[i].lines[j].object);
					//Handle case of mods that don't exist at all.
					if (index2 == (size_t)-1) {
						if (userlist[i].ruleKey == ADD)
							lastRecognisedPos--;
						modlist.insert(modlist.begin()+index1,mod);
						ouputBuffer += "<li class='warn'>\""+EscapeHTMLSpecial(userlist[i].lines[j].object)+"\" is not installed, and is not in the masterlist. Rule skipped.";
						LOG_WARN(" * \"%s\" is not installed or in the masterlist.", userlist[i].lines[j].object.c_str());
						break;
					}
					//Handle the case of a rule sorting a mod into a position in unsorted mod territory.
					if (index2 > lastRecognisedPos) {
						if (userlist[i].ruleKey == ADD)
							lastRecognisedPos--;
						modlist.insert(modlist.begin()+index1,mod);
						ouputBuffer += "<li class='error'>\""+EscapeHTMLSpecial(userlist[i].lines[j].object)+"\" is not in the masterlist and has not been sorted by a rule. Rule skipped.";
						LOG_WARN(" * \"%s\" is not in the masterlist and has not been sorted by a rule.", userlist[i].lines[j].object.c_str());
						break;
					}
					//Insert the mod into its new position.
					if (userlist[i].lines[j].key == AFTER) 
						index2 += 1;
					modlist.insert(modlist.begin()+index2,mod);
					ouputBuffer += "<li class='success'>\""+EscapeHTMLSpecial(userlist[i].ruleObject)+"\" has been sorted "+EscapeHTMLSpecial(KeyToString(userlist[i].lines[j].key))+" \""+EscapeHTMLSpecial(userlist[i].lines[j].object)+"\".";
				//A group sorting line.
				} else if ((userlist[i].lines[j].key == BEFORE || userlist[i].lines[j].key == AFTER) && !IsPlugin(userlist[i].lines[j].object)) {
					vector<item> group;
					size_t index1, index2;
					//Look for group to sort. Find start and end positions.
					index1 = GetModPos(modlist, userlist[i].ruleObject);
					index2 = GetGroupEndPos(modlist, userlist[i].ruleObject);
					//Check to see group actually exists.
					if (index1 == (size_t)-1 || index2 == (size_t)-1) {
						ouputBuffer += "<li class='error'>The group \""+EscapeHTMLSpecial(userlist[i].ruleObject)+"\" is not in the masterlist or is malformatted. Rule skipped.";
						LOG_WARN(" * \"%s\" is not in the masterlist, or is malformatted.", userlist[i].ruleObject.c_str());
						break;
					}
					//Copy the start, end and everything in between to a new variable.
					group.assign(modlist.begin()+index1,modlist.begin()+index2+1);
					//Now erase group from modlist.
					modlist.erase(modlist.begin()+index1,modlist.begin()+index2+1);
					//Find the group to sort relative to and insert it before or after it as appropriate.
					if (userlist[i].lines[j].key == BEFORE)
						index2 = GetModPos(modlist, userlist[i].lines[j].object);  //Find the start.
					else
						index2 = GetGroupEndPos(modlist, userlist[i].lines[j].object);  //Find the end, and add one, as inserting works before the given element.
					//Check that the sort group actually exists.
					if (index2 == (size_t)-1) {
						modlist.insert(modlist.begin()+index1,group.begin(),group.end());  //Insert the group back in its old position.
						ouputBuffer += "<li class='error'>The group \""+EscapeHTMLSpecial(userlist[i].lines[j].object)+"\" is not in the masterlist or is malformatted. Rule skipped.";
						LOG_WARN(" * \"%s\" is not in the masterlist, or is malformatted.", userlist[i].lines[j].object.c_str());
						break;
					}
					if (userlist[i].lines[j].key == AFTER)
						index2++;
					//Now insert the group.
					modlist.insert(modlist.begin()+index2,group.begin(),group.end());
					//Print success message.
					ouputBuffer += "<li class='success'>The group \""+EscapeHTMLSpecial(userlist[i].ruleObject)+"\" has been sorted "+KeyToString(userlist[i].lines[j].key)+" the group \""+EscapeHTMLSpecial(userlist[i].lines[j].object)+"\".";
				//An insertion line.
				} else if (userlist[i].lines[j].key == TOP || userlist[i].lines[j].key == BOTTOM) {
					size_t index1,index2;
					item mod;
					index1 = GetModPos(modlist,userlist[i].ruleObject);  //Find the rule mod in the modlist.
					//Do checks/increments.
					if (userlist[i].ruleKey == ADD && index1 > lastRecognisedPos) 
						lastRecognisedPos++;
					else if (userlist[i].ruleKey == ADD && index1 == (size_t)-1) {
						ouputBuffer += "<li class='warn'>\""+EscapeHTMLSpecial(userlist[i].ruleObject)+"\" is not installed or in the masterlist. Rule skipped.";
						LOG_WARN(" * \"%s\" is not installed.", userlist[i].ruleObject.c_str());
						break;
					//If it adds a mod already sorted, skip the rule.
					} else if (userlist[i].ruleKey == ADD  && index1 <= lastRecognisedPos) {
						ouputBuffer += "<li class='warn'>\""+EscapeHTMLSpecial(userlist[i].ruleObject)+"\" is already in the masterlist. Rule skipped.";
						LOG_WARN(" * \"%s\" is already in the masterlist.", userlist[i].ruleObject.c_str());
						break;
					} else if (userlist[i].ruleKey == OVERRIDE && (index1 > lastRecognisedPos || index1 == (size_t)-1)) {
						ouputBuffer += "<li class='error'>\""+EscapeHTMLSpecial(userlist[i].ruleObject)+"\" is not in the masterlist, cannot override. Rule skipped.";
						LOG_WARN(" * \"%s\" is not in the masterlist, cannot override.", userlist[i].ruleObject.c_str());
						break;
					}
					mod = modlist[index1];  //Record the rule mod in a new variable.
					//Remove the rule mod from its current position.
					modlist.erase(modlist.begin()+index1);
					//Find the position of the group to sort it to.
					//Find the group to sort relative to and insert it before or after it as appropriate.
					if (userlist[i].lines[j].key == TOP)
						index2 = GetModPos(modlist, userlist[i].lines[j].object);  //Find the start.
					else
						index2 = GetGroupEndPos(modlist, userlist[i].lines[j].object);  //Find the end.
					//Check that the sort group actually exists.
					if (index2 == (size_t)-1) {
						if (userlist[i].ruleKey == ADD)
							lastRecognisedPos--;
						modlist.insert(modlist.begin()+index1,mod);  //Insert the mod back in its old position.
						ouputBuffer += "<li class='error'>The group \""+EscapeHTMLSpecial(userlist[i].lines[j].object)+"\" is not in the masterlist or is malformatted. Rule skipped.";
						LOG_WARN(" * \"%s\" is not in the masterlist, or is malformatted.", userlist[i].lines[j].object.c_str());
						break;
					}
					//Now insert the mod into the group.
					modlist.insert(modlist.begin()+index2,mod);
					//Print success message.
					ouputBuffer += "<li class='success'>\""+EscapeHTMLSpecial(userlist[i].ruleObject)+"\" inserted at the "+ KeyToString(userlist[i].lines[j].key) + " of group \"" + EscapeHTMLSpecial(userlist[i].lines[j].object) + "\".";
				//A message line.
				} else if (userlist[i].lines[j].key == APPEND || userlist[i].lines[j].key == REPLACE) {
					size_t index, pos;
					string key,data;
					message newMessage;
					//Find the mod which will have its messages edited.
					index = GetModPos(modlist,userlist[i].ruleObject);
					//Do checks/increments.
					if (index == (size_t)-1) {  //Rule mod isn't in the modlist (ie. not in masterlist or installed), so can neither add it nor override it.
						ouputBuffer += "<li class='warn'>\""+EscapeHTMLSpecial(userlist[i].ruleObject)+"\" is not installed or in the masterlist. Rule skipped.";
						LOG_WARN(" * \"%s\" is not installed.", userlist[i].ruleObject.c_str());
						break;
					}
					//The provided message string could be using either the old format or the new format. Need to support both.
					//The easiest way is to check the first character. If it's a symbol character, it's the old format, otherwise the new.
					char sym = userlist[i].lines[j].object[0];
					if (sym == '?' || sym == '%' || sym == ':' || sym == '"') {  //Old format.
						//First character is the keyword, the rest is the data.
						newMessage.key = StringToKey(userlist[i].lines[j].object.substr(0,1));
						newMessage.data = trim_copy(userlist[i].lines[j].object.substr(1));
					} else {  //New format.
						pos = userlist[i].lines[j].object.find(":"); //Look for separator colon.
						newMessage.key = StringToKey(Tidy(userlist[i].lines[j].object.substr(0,pos)));
						newMessage.data = trim_copy(userlist[i].lines[j].object.substr(pos+1));
					}
					//If the rule is to replace messages, clear existing messages.
					if (userlist[i].lines[j].key == REPLACE)
						modlist[index].messages.clear();
					//Append message to message list of mod.
					modlist[index].messages.push_back(newMessage);

					//Output confirmation.
					ouputBuffer += "<li class='success'>\"<span class='message'>" + EscapeHTMLSpecial(userlist[i].lines[j].object) + "</span>\"";
					if (userlist[i].lines[j].key == APPEND)
						ouputBuffer += " appended to ";
					else
						ouputBuffer += " replaced ";
					ouputBuffer += "messages attached to \"" + EscapeHTMLSpecial(userlist[i].ruleObject) + "\".\n";
				}
			}
		}
		if (userlist.empty()) 
			ouputBuffer = "No valid rules were found in your userlist.txt.";
	}

	//Lists Script Extender plugin info in the output buffer.
	BOSS_COMMON_EXP string GetSEPluginInfo(string& outputBuffer) {
		string SE, SELoc, SEPluginLoc;
		if (game == 1 || game == 3) {  //Oblivion/Nehrim
			SE = "OBSE";
			SELoc = "../obse_1_2_416.dll";
			SEPluginLoc = "OBSE/Plugins";
		} else if (game == 2) {  //Fallout 3
			SE = "FOSE";
			SELoc = "../fose_loader.exe";
			SEPluginLoc = "FOSE/Plugins";
		} else if (game == 4) {  //Fallout: New Vegas
			SE = "NVSE";
			SELoc = "../nvse_loader.exe";
			SEPluginLoc = "NVSE/Plugins";
		} else if (game == 5) {  //Skyrim
			SE = "SKSE";
			SELoc = "../skse_loader.exe";
			SEPluginLoc = "SKSE/Plugins";
		}

		if (!fs::exists(SELoc) || SELoc.empty()) {
			LOG_DEBUG("Script Extender not detected");
			return "";
		} else {
			string CRC = IntToHexString(GetCrc32(SELoc));
			string ver = GetExeDllVersion(SELoc);

			outputBuffer += "<li><span class='mod'>" + SE + "</span>";
			if (ver.length() != 0)
				outputBuffer += "<span class='version'>&nbsp;Version: " + ver + "</span>";
			outputBuffer += "<span class='crc'>&nbsp;Checksum: " + CRC + "</span>";

			if (!fs::is_directory(data_path / SEPluginLoc)) {
				LOG_DEBUG("Script extender plugins directory not detected");
			} else {
				for (fs::directory_iterator itr(data_path / SEPluginLoc); itr!=fs::directory_iterator(); ++itr) {
					const fs::path filename = itr->path().filename();
					const string ext = Tidy(itr->path().extension().string());
					if (fs::is_regular_file(itr->status()) && ext==".dll") {
						string CRC = IntToHexString(GetCrc32(itr->path()));
						string ver = GetExeDllVersion(itr->path());

						outputBuffer += "<li><span class='mod'>" + EscapeHTMLSpecial(filename.string()) + "</span>";
						if (ver.length() != 0)
							outputBuffer += "<span class='version'>&nbsp;Version: " + ver + "</span>";
						outputBuffer += "<span class='crc'>&nbsp;Checksum: " + CRC + "</span>";
					}
				}
			}
			return SE;
		}
	}

	//Sort recognised mods.
	BOSS_COMMON_EXP void SortRecognisedMods(const vector<item>& modlist, const size_t lastRecognisedPos, string& ouputBuffer, const time_t esmtime, summaryCounters& counters) {
		time_t modfiletime = 0;
		LOG_INFO("Applying calculated ordering to user files...");
		for (size_t i=0; i<=lastRecognisedPos; i++) {
			//Only act on mods that exist.
			if (modlist[i].type == MOD && (Exists(data_path / modlist[i].name))) {
				ouputBuffer += "<li><span class='mod'>" + EscapeHTMLSpecial(TrimDotGhost(modlist[i].name.string())) + "</span>";
				if (!skip_version_parse) {
					string version = GetModHeader(modlist[i].name);
					if (!version.empty())
						ouputBuffer += "<span class='version'>&nbsp;Version "+EscapeHTMLSpecial(version)+"</span>";
				}
				if (IsGhosted(data_path / modlist[i].name)) {
					ouputBuffer += "<span class='ghosted'>&nbsp;Ghosted</span>";
					counters.ghosted++;
				}
				if (show_CRCs)
					ouputBuffer += "<span class='crc'>&nbsp;Checksum: " + IntToHexString(GetCrc32(data_path / modlist[i].name)) + "</span>";
				//Now change the file's date, if it is not the game's master file.
				if (!IsMasterFile(modlist[i].name.string()) && !trial_run) {
					//Calculate the new file time.
					modfiletime = esmtime + counters.recognised*60;  //time_t is an integer number of seconds, so adding 60 on increases it by a minute. Using recModNo instead of i to avoid increases for group entries.
					//Re-date file. Provide exception handling in case their permissions are wrong.
					LOG_DEBUG(" -- Setting last modified time for file: \"%s\"", modlist[i].name.string().c_str());
					try { 
						fs::last_write_time(data_path / modlist[i].name,modfiletime);
					} catch(fs::filesystem_error e) {
						ouputBuffer += " - <span class='error'>Error: Could not change the date of \"" + EscapeHTMLSpecial(modlist[i].name.string()) + "\", check the Troubleshooting section of the ReadMe for more information and possible solutions.</span>";
					}
				}
				//Finally, print the mod's messages.
				if (!modlist[i].messages.empty()) {
					ouputBuffer += "<ul>";
					size_t size = modlist[i].messages.size();
					for (size_t j=0; j<size; j++) {
						ShowMessage(ouputBuffer, modlist[i].messages[j]);  //Print messages to buffer.
						counters.messages++;
						if (modlist[i].messages[j].key == WARN)
							counters.warnings++;
						else if (modlist[i].messages[j].key == ERR)
							counters.errors++;
					}
					ouputBuffer += "</ul>";
				} else
					ouputBuffer += "";
				counters.recognised++;
			}
		}
		LOG_INFO("User file ordering applied successfully.");
	}

	//List unrecognised mods.
	BOSS_COMMON_EXP void ListUnrecognisedMods(const vector<item>& modlist, const size_t lastRecognisedPos, string& ouputBuffer, const time_t esmtime, summaryCounters& counters) {
		time_t modfiletime = 0;
		size_t modlistSize = modlist.size();
		//Find and show found mods not recognised. These are the mods that are found at and after index x in the mods vector.
		//Order their dates to be i days after the master esm to ensure they load last.
		LOG_INFO("Reporting unrecognized mods...");
		for (size_t i=lastRecognisedPos+1; i<modlistSize; i++) {
			//Only act on mods that exist.
			if (modlist[i].type == MOD && (Exists(data_path / modlist[i].name))) {
				ouputBuffer += "<li><span class='mod'>" + EscapeHTMLSpecial(TrimDotGhost(modlist[i].name.string())) + "</span>";
				if (!skip_version_parse) {
					string version = GetModHeader(modlist[i].name);
					if (!version.empty())
						ouputBuffer += "<span class='version'>&nbsp;Version "+EscapeHTMLSpecial(version)+"</span>";
				}
				if (IsGhosted(data_path / modlist[i].name)) {
					ouputBuffer += "<span class='ghosted'>&nbsp;Ghosted</span>";
					counters.ghosted++;
				}
				if (show_CRCs)
					ouputBuffer += "<span class='crc'>&nbsp;Checksum: " + IntToHexString(GetCrc32(data_path / modlist[i].name)) + "</span>";
			
				if (!trial_run) {
					modfiletime = esmtime + 86400 + (counters.recognised + counters.unrecognised)*60;  //time_t is an integer number of seconds, so adding 60 on increases it by a minute and adding 86,400 on increases it by a day. Using unrecModNo instead of i to avoid increases for group entries.
					//Re-date file. Provide exception handling in case their permissions are wrong.
					LOG_DEBUG(" -- Setting last modified time for file: \"%s\"", modlist[i].name.string().c_str());
					try {
						fs::last_write_time(data_path / modlist[i].name,modfiletime);
					} catch(fs::filesystem_error e) {
						ouputBuffer += " - <span class='error'>Error: Could not change the date of \"" + EscapeHTMLSpecial(modlist[i].name.string()) + "\", check the Troubleshooting section of the ReadMe for more information and possible solutions.</span>";
					}
				}
				counters.unrecognised++;
			}
		}
		if (lastRecognisedPos+1 == modlistSize)
			ouputBuffer += "<i>No unrecognised plugins.</i>";
	
		LOG_INFO("Unrecognized mods reported.");
	}

	//Prints the full BOSSlog.
	BOSS_COMMON_EXP void PrintBOSSlog(bosslogContents contents, const summaryCounters counters, const string scriptExtender) {
		/////////////////////////////
		// Print Output to BOSSlog
		/////////////////////////////

		OutputHeader();  //Output BOSSlog header.

		/////////////////////////////
		// Print BOSSLog Filters
		/////////////////////////////
	
		if (log_format == "html") {
			Output("<ul id='filters'>");
			if (UseDarkColourScheme)
				Output("<li><input type='checkbox' checked='checked' id='b1' onclick='swapColorScheme(this)' /><label for='b1'>Use Dark Colour Scheme</label>");
			else
				Output("<li><input type='checkbox' id='b1' onclick='swapColorScheme(this)' /><label for='b1'>Use Dark Colour Scheme</label>");

			if (HideRuleWarnings)
				Output("<li><input type='checkbox' checked='checked' id='b12' onclick='toggleUserlistWarnings(this)' /><label for='b12'>Hide Rule Warnings</label>");
			else
				Output("<li><input type='checkbox' id='b12' onclick='toggleUserlistWarnings(this)' /><label for='b12'>Hide Rule Warnings</label>");
		
			if (HideVersionNumbers)
				Output("<li><input type='checkbox' checked='checked' id='b2' onclick='toggleDisplayCSS(this,\".version\",\"inline\")' /><label for='b2'>Hide Version Numbers</label>");
			else
				Output("<li><input type='checkbox' id='b2' onclick='toggleDisplayCSS(this,\".version\",\"inline\")' /><label for='b2'>Hide Version Numbers</label>");

			if (HideGhostedLabel)
				Output("<li><input type='checkbox' checked='checked' id='b3' onclick='toggleDisplayCSS(this,\".ghosted\",\"inline\")' /><label for='b3'>Hide 'Ghosted' Label</label>");
			else
				Output("<li><input type='checkbox' id='b3' onclick='toggleDisplayCSS(this,\".ghosted\",\"inline\")' /><label for='b3'>Hide 'Ghosted' Label</label>");

			if (HideChecksums)
				Output("<li><input type='checkbox' checked='checked' id='b4' onclick='toggleDisplayCSS(this,\".crc\",\"inline\")' /><label for='b4'>Hide Checksums</label>");
			else
				Output("<li><input type='checkbox' id='b4' onclick='toggleDisplayCSS(this,\".crc\",\"inline\")' /><label for='b4'>Hide Checksums</label>");

			if (HideMessagelessMods)
				Output("<li><input type='checkbox' checked='checked' id='noMessageModFilter' onclick='toggleMods()' /><label for='noMessageModFilter'>Hide Messageless Mods</label>");
			else
				Output("<li><input type='checkbox' id='noMessageModFilter' onclick='toggleMods()' /><label for='noMessageModFilter'>Hide Messageless Mods</label>");

			if (HideGhostedMods)
				Output("<li><input type='checkbox' checked='checked' id='ghostModFilter' onclick='toggleMods()' /><label for='ghostModFilter'>Hide Ghosted Mods</label>");
			else
				Output("<li><input type='checkbox' id='ghostModFilter' onclick='toggleMods()' /><label for='ghostModFilter'>Hide Ghosted Mods</label>");

			if (HideCleanMods)
				Output("<li><input type='checkbox' checked='checked' id='cleanModFilter' onclick='toggleMods()' /><label for='cleanModFilter'>Hide Clean Mods</label>");
			else
				Output("<li><input type='checkbox' id='cleanModFilter' onclick='toggleMods()' /><label for='cleanModFilter'>Hide Clean Mods</label>");

			if (HideAllModMessages)
				Output("<li><input type='checkbox' checked='checked' id='b7' onclick='toggleDisplayCSS(this,\"li ul\",\"block\")' /><label for='b7'>Hide All Mod Messages</label>");
			else
				Output("<li><input type='checkbox' id='b7' onclick='toggleDisplayCSS(this,\"li ul\",\"block\")' /><label for='b7'>Hide All Mod Messages</label>");

			if (HideNotes)
				Output("<li><input type='checkbox' checked='checked' id='b8' onclick='toggleDisplayCSS(this,\".note\",\"table\")' /><label for='b8'>Hide Notes</label>");
			else
				Output("<li><input type='checkbox' id='b8' onclick='toggleDisplayCSS(this,\".note\",\"table\")' /><label for='b8'>Hide Notes</label>");

			if (HideBashTagSuggestions)
				Output("<li><input type='checkbox' checked='checked' id='b9' onclick='toggleDisplayCSS(this,\".tag\",\"table\")' /><label for='b9'>Hide Bash Tag Suggestions</label>");
			else
				Output("<li><input type='checkbox' id='b9' onclick='toggleDisplayCSS(this,\".tag\",\"table\")' /><label for='b9'>Hide Bash Tag Suggestions</label>");

			if (HideRequirements)
				Output("<li><input type='checkbox' checked='checked' id='b10' onclick='toggleDisplayCSS(this,\".req\",\"table\")' /><label for='b10'>Hide Requirements</label>");
			else
				Output("<li><input type='checkbox' id='b10' onclick='toggleDisplayCSS(this,\".req\",\"table\")' /><label for='b10'>Hide Requirements</label>");

			if (HideIncompatibilities)
				Output("<li><input type='checkbox' checked='checked' id='b11' onclick='toggleDisplayCSS(this,\".inc\",\"table\")' /><label for='b11'>Hide Incompatibilities</label>");
			else
				Output("<li><input type='checkbox' id='b11' onclick='toggleDisplayCSS(this,\".inc\",\"table\")' /><label for='b11'>Hide Incompatibilities</label>");

			if (HideDoNotCleanMessages)
				Output("<li><input type='checkbox' checked='checked' id='b12' onclick='toggleDoNotClean(this,\"table\")' /><label for='b11'>Hide 'Do Not Clean' Messages</label>");
			else
				Output("<li><input type='checkbox' id='b12' onclick='toggleDoNotClean(this,\"table\")' /><label for='b11'>Hide 'Do Not Clean' Messages</label>");

			Output("</ul>");
		}


		/////////////////////////////
		// Display Global Messages
		/////////////////////////////

		if (!globalMessageBuffer.empty() || !iniErrorBuffer.empty() || !contents.updaterErrors.empty()) {
			string buffer;
			size_t size = iniErrorBuffer.size();
			Output("<h3 onclick='toggleSectionDisplay(this)'><span>&#x2212;</span>General Messages</h3><ul>");
			for (size_t i=0; i<size; i++)  //First print parser/syntax error messages.
				Output(iniErrorBuffer[i]);
			size = globalMessageBuffer.size();
			for (size_t i=0; i<size; i++)
				ShowMessage(buffer, globalMessageBuffer[i]);  //Print messages.
			Output(contents.updaterErrors);
			Output(buffer);
			Output("</ul>");
		}

		/////////////////////////////
		// Print Summary
		/////////////////////////////

		Output("<h3 onclick='toggleSectionDisplay(this)'><span>&#x2212;</span>Summary</h3><div>");

		if (contents.oldRecognisedPlugins == contents.recognisedPlugins)
			Output("<p>No change in recognised plugin list since last run.");

		if (!contents.summary.empty()) {
			Output(contents.summary);
		}
	
		Output("<table><tbody>");
		Output("<tr><td>Recognised plugins:<td>" + IntToString(counters.recognised) + "<td>Warning messages:<td>" + IntToString(counters.warnings));
		Output("<tr><td>Unrecognised plugins:<td>" + IntToString(counters.unrecognised) + "<td>Error messages:<td>" + IntToString(counters.errors));
		Output("<tr><td>Ghosted plugins:<td>" + IntToString(counters.ghosted) + "<td>Total number of messages:<td>" + IntToString(counters.messages));
		Output("<tr><td>Total number of plugins:<td>" + IntToString(counters.recognised+counters.unrecognised)+"<td><td>");
		Output("</table>");

		Output("<p>Mods sorted by your userlist are counted as recognised, not unrecognised, plugins.");
		Output("</div>");

		
		/////////////////////////////
		// Display Userlist Messages
		/////////////////////////////

		if (!contents.userlistMessages.empty() || !userlistErrorBuffer.empty()) {
			size_t size = userlistErrorBuffer.size();
			Output("<h3 onclick='toggleSectionDisplay(this)'><span>&#x2212;</span>Userlist Messages</h3><ul id='userlistMessages'>");
			for (size_t i=0; i<size; i++)  //First print parser/syntax error messages.
				Output(userlistErrorBuffer[i]);
			Output(contents.userlistMessages);  //Now print the rest of the userlist messages.
			Output("</ul>");
		}


		/////////////////////////////////
		// Display Script Extender Info
		/////////////////////////////////

		if (!contents.seInfo.empty()) {
			Output("<h3 onclick='toggleSectionDisplay(this)'><span>&#x2212;</span>" + scriptExtender + " And " + scriptExtender + " Plugin Checksums</h3><ul>");
			Output(contents.seInfo);
			Output("</ul>");
		}


		/////////////////////////////////
		// Display Recognised Mods
		/////////////////////////////////

		if (revert<1) Output("<h3 onclick='toggleSectionDisplay(this)'><span>&#x2212;</span>Recognised And Re-ordered Plugins</h3><ul id='recognised'>");
		else if (revert==1) Output("<h3 onclick='toggleSectionDisplay(this)'><span>&#x2212;</span>Restored Load Order (Using modlist.txt)</h3><ul id='recognised'>");
		else if (revert==2) Output("<h3 onclick='toggleSectionDisplay(this)'><span>&#x2212;</span>Restored Load Order (Using modlist.old)</h3><ul id='recognised'>");
		Output(contents.recognisedPlugins);
		Output("</ul>");


		/////////////////////////////////
		// Display Unrecognised Mods
		/////////////////////////////////

		Output("<h3 onclick='toggleSectionDisplay(this)'><span>&#x2212;</span>Unrecognised Plugins</h3><div><p>Reorder these by hand using your favourite mod ordering utility.<ul>");
		Output(contents.unrecognisedPlugins);
		Output("</ul></div>");


		////////////////
		// Finish
		////////////////

		Output("<h3 id='end'>Execution Complete</h3>");
		OutputFooter();
	}
}