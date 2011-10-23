/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 3184 $, $Date: 2011-08-26 20:52:13 +0100 (Fri, 26 Aug 2011) $
*/

#include "Common/Execute.h"
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

	BOSS_COMMON_EXP summaryCounters::summaryCounters() {
		recognised = 0;
		unrecognised = 0;
		ghosted = 0;
		messages = 0;
		warnings = 0;
		errors = 0;
	}

	BOSS_COMMON_EXP bosslogContents::bosslogContents() {
		generalMessages.clear();
		summary.clear();
		userlistMessages.clear();
		seInfo.clear();
		recognisedPlugins.clear();
		unrecognisedPlugins.clear();

		oldRecognisedPlugins.clear();

		updaterErrors.clear();
		iniParsingError.clear();
		criticalError.clear();
		userlistParsingError.clear();
		userlistSyntaxErrors.clear();
		globalMessages.clear();
	}

	//Searches a hashset for the first matching string of a regex and returns its iterator position.
	BOSS_COMMON_EXP boost::unordered_set<string>::iterator FindRegexMatch(const boost::unordered_set<string> set, const boost::regex reg, boost::unordered_set<string>::iterator startPos) {
		while(startPos != set.end()) {
			if (boost::regex_match(*startPos,reg))
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
			/*In BOSSv2.0, this is where we will query the following registry entries:
			Oblivion x86: "HKLM\Software\Bethesda Softworks\Oblivion\Install Path"
			Oblivion x64: "HKLM\Software\Wow6432Node\Bethesda Softworks\Oblivion\Install Path"
			*/
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
	BOSS_COMMON_EXP void BuildWorkingModlist(ItemList& modlist, ItemList& masterlist, RuleList& userlist) {
		//Add all modlist and userlist mods to a hashset to optimise comparison against masterlist.
		boost::unordered_set<string> hashset;  //Holds mods for checking against masterlist
		boost::unordered_set<string>::iterator setPos;

		size_t size;
		size_t userlistSize = userlist.rules.size();
		size_t linesSize;
		/* Hashset must be a set of unique mods.
		Ghosted mods take priority over non-ghosted mods, as they are specifically what is installed. 
		*/

		LOG_INFO("Populating hashset with modlist.");
		size = modlist.items.size();
		for (size_t i=0; i<size; i++) {
			if (modlist.items[i].type == MOD)
				hashset.insert(Tidy(modlist.items[i].name.string()));
		}
		LOG_INFO("Populating hashset with userlist.");
		//Need to get ruleObject and sort line object if plugins.
		for (size_t i=0; i<userlistSize; i++) {
			if (Item(userlist.rules[i].ruleObject).IsPlugin()) {
				setPos = hashset.find(Tidy(userlist.rules[i].ruleObject));
				if (setPos == hashset.end()) {  //Mod not already in hashset.
					setPos = hashset.find(Tidy(userlist.rules[i].ruleObject + ".ghost"));
					if (setPos == hashset.end()) {  //Ghosted mod not already in hashset. 
						//Unique plugin, so add to hashset.
						hashset.insert(Tidy(userlist.rules[i].ruleObject));
					}
				}
			}
			if (userlist.rules[i].ruleKey != FOR) {  //First line is a sort line.
				if (Item(userlist.rules[i].lines[0].object).IsPlugin()) {
					setPos = hashset.find(Tidy(userlist.rules[i].lines[0].object));
					if (setPos == hashset.end()) {  //Mod not already in hashset.
						setPos = hashset.find(Tidy(userlist.rules[i].lines[0].object + ".ghost"));
						if (setPos == hashset.end()) {  //Ghosted mod not already in hashset. 
							//Unique plugin, so add to hashset.
							hashset.insert(Tidy(userlist.rules[i].lines[0].object));
						}
					}
				}
			}
		}

		//Now compare masterlist against hashset.
		vector<Item>::iterator iter = masterlist.items.begin();
		vector<Item>::iterator modlistPos;
		vector<Item> holdingVec;
		boost::unordered_set<string>::iterator addedPos;
		boost::unordered_set<string> addedMods;
		LOG_INFO("Comparing hashset against masterlist.");
		for (iter; iter != masterlist.items.end(); ++iter) {
			if (iter->type == MOD) {
				//Check to see if the mod is in the hashset. If it is, or its ghosted version is, also check if 
				//the mod is already in the holding vector. If not, add it.
				setPos = hashset.find(Tidy(iter->name.string()));
				if (setPos == hashset.end()) {
					iter->name = fs::path(iter->name.string() + ".ghost");		//Add ghost extension to mod name.
					setPos = hashset.find(Tidy(iter->name.string()));
				}
				if (setPos != hashset.end()) {										//Mod found in hashset. 
					addedPos = addedMods.find(Tidy(iter->name.string()));
					if (addedPos == addedMods.end()) {								//The mod is not already in the holding vector.
						holdingVec.push_back(*iter);									//Record it in the holding vector.
						modlistPos = modlist.FindItem(iter->name);
						if (modlistPos != modlist.items.end())
							modlist.items.erase(modlistPos);
						addedMods.insert(Tidy(iter->name.string()));
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
					modlistPos = modlist.FindItem(fs::path(mod));
					if (modlistPos != modlist.items.end())
						mod = modlistPos->name.string();
					else {
						for (size_t i=0; i<userlistSize; i++) {
							linesSize = userlist.rules[i].lines.size();
							for (size_t j=0; j<linesSize; j++) {
								if (Tidy(userlist.rules[i].lines[j].object) == mod)
									mod = userlist.rules[i].lines[j].object;
							}
						}
					}
					//Check that the mod hasn't already been added to the holding vector.
					addedPos = addedMods.find(Tidy(mod));
					if (addedPos == addedMods.end()) {							//The mod is not already in the holding vector.
						//Now do the adding/removing.
						//Create new temporary item to hold current found mod.
						fs::path modPath(mod);
						Item tempItem = Item(modPath, MOD, iter->messages);

						holdingVec.push_back(tempItem);							//Record it in the holding vector.
						modlistPos = modlist.FindItem(modPath);
						if (modlistPos != modlist.items.end())
							modlist.items.erase(modlistPos);
						addedMods.insert(Tidy(mod));
					}
					++setPos;
				} while (setPos != hashset.end());
			} else //Group lines must stay recorded.
				holdingVec.push_back(*iter);
		}
		masterlist.items = holdingVec;  //Masterlist now only contains the items needed to sort the user's mods.
		fs::path lastRec = masterlist.items.back().name;
		
		//Add modlist's mods to masterlist, then set the modlist to the masterlist as that's the output..
		masterlist.items.insert(masterlist.items.end(),modlist.items.begin(),modlist.items.end());
		modlist = masterlist;
		modlist.lastRecognisedPos = modlist.FindItem(lastRec);  //Record position of last sorted item.
	}

	//Applies the userlist rules to the working modlist.
	BOSS_COMMON_EXP void ApplyUserRules(ItemList& modlist, RuleList& userlist, string& outputBuffer) {
		if (userlist.rules.empty())
			return;
		//Because erase operations invalidate iterators after the position(s) erased, the last recognised mod needs to be recorded, then
		//set correctly again after all operations have completed.
		//Note that if a mod is sorted after the last recognised mod by the userlist, it becomes the last recognised mod, and the item will
		//need to be re-assigned to this item. This only occurs for BEFORE/AFTER plugin sorting rules.
		fs::path lastRecognisedModName(modlist.lastRecognisedPos->name);

		LOG_INFO("Starting userlist sort process... Total %" PRIuS " user rules statements to process.", userlist.rules.size());
		vector<Rule>::iterator ruleIter = userlist.rules.begin();
		vector<RuleLine>::iterator lineIter;
		vector<Item>::iterator modlistPos1, modlistPos2;
		unsigned int i = 0;
		for (ruleIter; ruleIter != userlist.rules.end(); ++ruleIter) {
			i++;
			LOG_DEBUG(" -- Processing rule #%" PRIuS ".", i);
			if (!ruleIter->enabled) {
				outputBuffer += "<li class='success'>The rule beginning \"" + ruleIter->KeyToString() + ": " + ruleIter->ruleObject + "\" is disabled. Rule skipped.";
				LOG_WARN("Rule beginning \"%s: %s\" is disabled. Rule skipped.", ruleIter->KeyToString().c_str(), ruleIter->ruleObject.c_str());
				continue;
			}	
			lineIter = ruleIter->lines.begin();
			Item ruleItem(fs::path(ruleIter->ruleObject));
			if (ruleItem.IsPlugin()) {  //Plugin: Can sort or add messages.
				if (ruleIter->ruleKey != FOR) { //First non-rule line is a sort line.
					if (lineIter->key == BEFORE || lineIter->key == AFTER) {
						Item mod;
						modlistPos1 = modlist.FindItem(ruleItem.name);
						//Do checks.
						if (ruleIter->ruleKey == ADD && modlistPos1 == modlist.items.end()) {
							outputBuffer += "<li class='warn'>\""+EscapeHTMLSpecial(ruleIter->ruleObject)+"\" is not installed or in the masterlist. Rule skipped.";
							LOG_WARN(" * \"%s\" is not installed.", ruleIter->ruleObject.c_str());
							continue;
						//If it adds a mod already sorted, skip the rule.
						} else if (ruleIter->ruleKey == ADD  && modlistPos1 <= modlist.lastRecognisedPos) {
							outputBuffer += "<li class='warn'>\""+EscapeHTMLSpecial(ruleIter->ruleObject)+"\" is already in the masterlist. Rule skipped.";
							LOG_WARN(" * \"%s\" is already in the masterlist.", ruleIter->ruleObject.c_str());
							continue;
						} else if (ruleIter->ruleKey == OVERRIDE && (modlistPos1 > modlist.lastRecognisedPos || modlistPos1 == modlist.items.end())) {
							outputBuffer += "<li class='error'>\""+EscapeHTMLSpecial(ruleIter->ruleObject)+"\" is not in the masterlist, cannot override. Rule skipped.";
							LOG_WARN(" * \"%s\" is not in the masterlist, cannot override.", ruleIter->ruleObject.c_str());
							continue;
						}
						modlistPos2 = modlist.FindItem(fs::path(lineIter->object));  //Find sort mod.
						//Do checks.
						if (modlistPos2 == modlist.items.end()) {  //Handle case of mods that don't exist at all.
							outputBuffer += "<li class='warn'>\""+EscapeHTMLSpecial(lineIter->object)+"\" is not installed, and is not in the masterlist. Rule skipped.";
							LOG_WARN(" * \"%s\" is not installed or in the masterlist.", lineIter->object.c_str());
							continue;
						} else if (modlistPos2 > modlist.lastRecognisedPos) {  //Handle the case of a rule sorting a mod into a position in unsorted mod territory.
							outputBuffer += "<li class='error'>\""+EscapeHTMLSpecial(lineIter->object)+"\" is not in the masterlist and has not been sorted by a rule. Rule skipped.";
							LOG_WARN(" * \"%s\" is not in the masterlist and has not been sorted by a rule.", lineIter->object.c_str());
							continue;
						} else if (lineIter->key == AFTER && modlistPos2 == modlist.lastRecognisedPos)
							lastRecognisedModName = fs::path(modlistPos1->name);
						mod = *modlistPos1;  //Record the rule mod in a new variable.
						modlist.items.erase(modlistPos1);  //Now remove the rule mod from its old position. This breaks all modlist iterators active.
						//Need to find sort mod pos again, to fix iterator.
						modlistPos2 = modlist.FindItem(fs::path(lineIter->object));  //Find sort mod.
						//Insert the mod into its new position.
						if (lineIter->key == AFTER)
							++modlistPos2;
						modlist.items.insert(modlistPos2, mod);
						outputBuffer += "<li class='success'>\""+EscapeHTMLSpecial(ruleIter->ruleObject)+"\" has been sorted "+lineIter->KeyToString()+" \""+EscapeHTMLSpecial(lineIter->object)+"\".";
					} else if (lineIter->key == TOP || lineIter->key == BOTTOM) {
						Item mod;
						modlistPos1 = modlist.FindItem(ruleItem.name);
						//Do checks.
						if (ruleIter->ruleKey == ADD && modlistPos1 == modlist.items.end()) {
							outputBuffer += "<li class='warn'>\""+EscapeHTMLSpecial(ruleIter->ruleObject)+"\" is not installed or in the masterlist. Rule skipped.";
							LOG_WARN(" * \"%s\" is not installed.", ruleIter->ruleObject.c_str());
							continue;
						//If it adds a mod already sorted, skip the rule.
						} else if (ruleIter->ruleKey == ADD  && modlistPos1 <= modlist.lastRecognisedPos) {
							outputBuffer += "<li class='warn'>\""+EscapeHTMLSpecial(ruleIter->ruleObject)+"\" is already in the masterlist. Rule skipped.";
							LOG_WARN(" * \"%s\" is already in the masterlist.", ruleIter->ruleObject.c_str());
							continue;
						} else if (ruleIter->ruleKey == OVERRIDE && (modlistPos1 > modlist.lastRecognisedPos || modlistPos1 == modlist.items.end())) {
							outputBuffer += "<li class='error'>\""+EscapeHTMLSpecial(ruleIter->ruleObject)+"\" is not in the masterlist, cannot override. Rule skipped.";
							LOG_WARN(" * \"%s\" is not in the masterlist, cannot override.", ruleIter->ruleObject.c_str());
							continue;
						}
						//Find the group to sort relative to.
						if (lineIter->key == TOP)
							modlistPos2 = modlist.FindItem(fs::path(lineIter->object)) + 1;  //Find the start, and increment by 1 so that mod is inserted after start.
						else
							modlistPos2 = modlist.FindGroupEnd(fs::path(lineIter->object));  //Find the end.
						//Check that the sort group actually exists.
						if (modlistPos2 == modlist.items.end()) {
							outputBuffer += "<li class='error'>The group \""+EscapeHTMLSpecial(lineIter->object)+"\" is not in the masterlist or is malformatted. Rule skipped.";
							LOG_WARN(" * \"%s\" is not in the masterlist, or is malformatted.", lineIter->object.c_str());
							continue;
						}
						mod = *modlistPos1;  //Record the rule mod in a new variable.
						modlist.items.erase(modlistPos1);  //Now remove the rule mod from its old position. This breaks all modlist iterators active.
						//Need to find group pos again, to fix iterators.
						if (lineIter->key == TOP)
							modlistPos2 = modlist.FindItem(fs::path(lineIter->object)) + 1;  //Find the start, and increment by 1 so that mod is inserted after start.
						else
							modlistPos2 = modlist.FindGroupEnd(fs::path(lineIter->object));  //Find the end.
						modlist.items.insert(modlistPos2, mod);  //Now insert the mod into the group. This breaks all modlist iterators active.
						//Print success message.
						outputBuffer += "<li class='success'>\""+EscapeHTMLSpecial(ruleIter->ruleObject)+"\" inserted at the "+ lineIter->KeyToString() + " of group \"" + EscapeHTMLSpecial(lineIter->object) + "\".";
					}
				}
				++lineIter;
				for (lineIter; lineIter != ruleIter->lines.end(); ++lineIter) {  //Message lines.
					if (!lineIter->IsObjectMessage()) {
						outputBuffer += "<li class='warn'>\""+EscapeHTMLSpecial(lineIter->object)+"\" is not a valid message. Rule skipped.";
						LOG_WARN(" * \"%s\" is not a valid message.", lineIter->object.c_str());
						break;
					}
					//Find the mod which will have its messages edited.
					modlistPos1 = modlist.FindItem(ruleItem.name);
					if (modlistPos1 == modlist.items.end()) {  //Rule mod isn't in the modlist (ie. not in masterlist or installed), so can neither add it nor override it.
						outputBuffer += "<li class='warn'>\""+EscapeHTMLSpecial(ruleIter->ruleObject)+"\" is not installed or in the masterlist. Rule skipped.";
						LOG_WARN(" * \"%s\" is not installed.", ruleIter->ruleObject.c_str());
						break;
					}
					Message newMessage = Message(lineIter->ObjectMessageKey(), lineIter->ObjectMessageData());
					if (lineIter->key == REPLACE)  //If the rule is to replace messages, clear existing messages.
						modlistPos1->messages.clear();
					modlistPos1->messages.push_back(newMessage);  //Append message to message list of mod.
					//Output confirmation.
					outputBuffer += "<li class='success'>\"<span class='message'>" + EscapeHTMLSpecial(lineIter->object) + "</span>\"";
					if (lineIter->key == APPEND)
						outputBuffer += " appended to ";
					else
						outputBuffer += " replaced ";
					outputBuffer += "messages attached to \"" + EscapeHTMLSpecial(ruleIter->ruleObject) + "\".\n";
				}
			} else if (lineIter->key == BEFORE || lineIter->key == AFTER) {  //Group: Can only sort.
				vector<Item> group;
				//Look for group to sort. Find start and end positions.
				modlistPos1 = modlist.FindItem(ruleItem.name);
				modlistPos2 = modlist.FindGroupEnd(ruleItem.name);
				//Check to see group actually exists.
				if (modlistPos1 != modlist.items.end() || modlistPos2 != modlist.items.end()) {
					outputBuffer += "<li class='error'>The group \""+EscapeHTMLSpecial(ruleIter->ruleObject)+"\" is not in the masterlist or is malformatted. Rule skipped.";
					LOG_WARN(" * \"%s\" is not in the masterlist, or is malformatted.", ruleIter->ruleObject.c_str());
					continue;
				}
				//Copy the start, end and everything in between to a new variable.
				group.assign(modlistPos1,modlistPos2+1);
				//Now erase group from modlist. This breaks the lastRecognisedPos iterator, so that will be reset after rule application.
				modlist.items.erase(modlistPos1,modlistPos2+1);
				//Find the group to sort relative to and insert it before or after it as appropriate.
				if (lineIter->key == BEFORE)
					modlistPos2 = modlist.FindItem(fs::path(lineIter->object));  //Find the start.
				else
					modlistPos2 = modlist.FindGroupEnd(fs::path(lineIter->object));  //Find the end, and add one, as inserting works before the given element.
				//Check that the sort group actually exists.
				if (modlistPos2 != modlist.items.end()) {
					modlist.items.insert(modlistPos1,group.begin(),group.end());  //Insert the group back in its old position.
					outputBuffer += "<li class='error'>The group \""+EscapeHTMLSpecial(lineIter->object)+"\" is not in the masterlist or is malformatted. Rule skipped.";
					LOG_WARN(" * \"%s\" is not in the masterlist, or is malformatted.", lineIter->object.c_str());
					continue;
				}
				if (lineIter->key == AFTER)
					modlistPos2++;
				//Now insert the group.
				modlist.items.insert(modlistPos2,group.begin(),group.end());
				//Print success message.
				outputBuffer += "<li class='success'>The group \""+EscapeHTMLSpecial(ruleIter->ruleObject)+"\" has been sorted "+lineIter->KeyToString()+" the group \""+EscapeHTMLSpecial(lineIter->object)+"\".";
			}
			//Now find that last recognised mod and set the iterator again.
			modlist.lastRecognisedPos = modlist.FindItem(lastRecognisedModName);
		}


		if (userlist.rules.empty()) 
			outputBuffer = "No valid rules were found in your userlist.txt.";
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
	BOSS_COMMON_EXP void SortRecognisedMods(ItemList& modlist, string& outputBuffer, const time_t esmtime, summaryCounters& counters) {
		time_t modfiletime = 0;
		LOG_INFO("Applying calculated ordering to user files...");
		vector<Item>::iterator iter = modlist.items.begin();
		vector<Message>::iterator messageIter;
		for (iter; iter != modlist.lastRecognisedPos+1; ++iter) {
			if (iter->type == MOD && iter->Exists()) {  //Only act on mods that exist.
				outputBuffer += "<li><span class='mod'>" + EscapeHTMLSpecial(iter->name.string()) + "</span>";
				if (!skip_version_parse) {
					string version = iter->GetHeader();
					if (!version.empty())
						outputBuffer += "<span class='version'>&nbsp;Version "+EscapeHTMLSpecial(version)+"</span>";
				}
				if (iter->IsGhosted()) {
					outputBuffer += "<span class='ghosted'>&nbsp;Ghosted</span>";
					counters.ghosted++;
				}
				if (show_CRCs)
					outputBuffer += "<span class='crc'>&nbsp;Checksum: " + IntToHexString(GetCrc32(data_path / iter->name)) + "</span>";
			
				if (!trial_run) {
					modfiletime = esmtime + counters.recognised*60;  //time_t is an integer number of seconds, so adding 60 on increases it by a minute. Using recModNo instead of i to avoid increases for group entries.
					//Re-date file. Provide exception handling in case their permissions are wrong.
					LOG_DEBUG(" -- Setting last modified time for file: \"%s\"", iter->name.string().c_str());
					try {
						fs::last_write_time(data_path / iter->name,modfiletime);
					} catch(fs::filesystem_error e) {
						outputBuffer += " - <span class='error'>Error: Could not change the date of \"" + EscapeHTMLSpecial(iter->name.string()) + "\", check the Troubleshooting section of the ReadMe for more information and possible solutions.</span>";
					}
				}
				//Finally, print the mod's messages.
				if (!iter->messages.empty()) {
					outputBuffer += "<ul>";
					messageIter = iter->messages.begin();
					for (messageIter; messageIter != iter->messages.end(); ++messageIter) {
						ShowMessage(outputBuffer, *messageIter);  //Print messages to buffer.
						counters.messages++;
						if (messageIter->key == WARN)
							counters.warnings++;
						else if (messageIter->key == ERR)
							counters.errors++;
					}
					outputBuffer += "</ul>";
				}
				counters.recognised++;
			}
		}
		LOG_INFO("User file ordering applied successfully.");
	}

	//List unrecognised mods.
	BOSS_COMMON_EXP void ListUnrecognisedMods(ItemList& modlist, string& outputBuffer, const time_t esmtime, summaryCounters& counters) {
		time_t modfiletime = 0;
		//Find and show found mods not recognised. These are the mods that are found at and after index x in the mods vector.
		//Order their dates to be i days after the master esm to ensure they load last.
		LOG_INFO("Reporting unrecognized mods...");
		vector<Item>::iterator iter = modlist.lastRecognisedPos+1;
		for (iter; iter != modlist.items.end(); ++iter) {
			if (iter->type == MOD && iter->Exists()) {  //Only act on mods that exist.
				outputBuffer += "<li><span class='mod'>" + EscapeHTMLSpecial(iter->name.string()) + "</span>";
				if (!skip_version_parse) {
					string version = iter->GetHeader();
					if (!version.empty())
						outputBuffer += "<span class='version'>&nbsp;Version "+EscapeHTMLSpecial(version)+"</span>";
				}
				if (iter->IsGhosted()) {
					outputBuffer += "<span class='ghosted'>&nbsp;Ghosted</span>";
					counters.ghosted++;
				}
				if (show_CRCs)
					outputBuffer += "<span class='crc'>&nbsp;Checksum: " + IntToHexString(GetCrc32(data_path / iter->name)) + "</span>";
			
				if (!trial_run) {
					modfiletime = esmtime + 86400 + (counters.recognised + counters.unrecognised)*60;  //time_t is an integer number of seconds, so adding 60 on increases it by a minute and adding 86,400 on increases it by a day. Using unrecModNo instead of i to avoid increases for group entries.
					//Re-date file. Provide exception handling in case their permissions are wrong.
					LOG_DEBUG(" -- Setting last modified time for file: \"%s\"", iter->name.string().c_str());
					try {
						fs::last_write_time(data_path / iter->name,modfiletime);
					} catch(fs::filesystem_error e) {
						outputBuffer += " - <span class='error'>Error: Could not change the date of \"" + EscapeHTMLSpecial(iter->name.string()) + "\", check the Troubleshooting section of the ReadMe for more information and possible solutions.</span>";
					}
				}
				counters.unrecognised++;
			}
		}
		if (modlist.lastRecognisedPos+1 == modlist.items.end())
			outputBuffer += "<i>No unrecognised plugins.</i>";
	
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
				Output("<li><input type='checkbox' checked='checked' id='b12' onclick='toggleRuleListWarnings(this)' /><label for='b12'>Hide Rule Warnings</label>");
			else
				Output("<li><input type='checkbox' id='b12' onclick='toggleRuleListWarnings(this)' /><label for='b12'>Hide Rule Warnings</label>");
		
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

		if (!contents.globalMessages.empty() || !contents.iniParsingError.empty() || !contents.criticalError.empty() || !contents.updaterErrors.empty()) {
			string buffer;
			Output("<h3 onclick='toggleSectionDisplay(this)'><span>&#x2212;</span>General Messages</h3><ul>");
			if (!contents.criticalError.empty())
				Output(contents.criticalError);
			if (!contents.iniParsingError.empty())  //First print parser/syntax error messages.
				Output(contents.iniParsingError);
			size_t size = contents.globalMessages.size();
			for (size_t i=0; i<size; i++)
				ShowMessage(buffer, contents.globalMessages[i]);  //Print messages.
			Output(contents.updaterErrors);
			Output(buffer);
			Output("</ul>");
			if (!contents.criticalError.empty()) {  //Exit early.
				OutputFooter();
				return;
			}
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
		// Display RuleList Messages
		/////////////////////////////

		if (!contents.userlistMessages.empty() || !contents.userlistParsingError.empty() || !contents.userlistSyntaxErrors.empty()) {

			Output("<h3 onclick='toggleSectionDisplay(this)'><span>&#x2212;</span>RuleList Messages</h3><ul id='userlistMessages'>");
			if (!contents.userlistParsingError.empty())  //First print parser/syntax error messages.
				Output(contents.userlistParsingError);
			size_t size = contents.userlistSyntaxErrors.size();
			for (size_t i=0;i<size;i++)
				Output(contents.userlistSyntaxErrors[i]);
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