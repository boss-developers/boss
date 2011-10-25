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

	summaryCounters::summaryCounters()
		: recognised(0), unrecognised(0), ghosted(0), messages(0), warnings(0), errors(0) {}

	//Searches a hashset for the first matching string of a regex and returns its iterator position. Usage internal to BOSS-Common.
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

	//Detect the game BOSS is installed for. Returns an enum as defined in Globals.h. Throws exception if error.
	BOSS_COMMON_EXP void GetGame() {
		if (fs::exists(data_path / "Oblivion.esm")) {
			if (fs::exists(data_path / "Nehrim.esm"))
				throw boss_error(BOSS_ERROR_OBLIVION_AND_NEHRIM);
			game = OBLIVION;
		} else if (fs::exists(data_path / "Nehrim.esm")) 
			game = NEHRIM;
		else if (fs::exists(data_path / "FalloutNV.esm")) 
			game = FALLOUTNV;
		else if (fs::exists(data_path / "Fallout3.esm")) 
			game = FALLOUT3;
		else if (fs::exists(data_path / "Skyrim.esm")) 
			game = SKYRIM;
		else
			throw boss_error(BOSS_ERROR_NO_MASTER_FILE);
			/*In BOSSv2.0, this is where we will query the following registry entries:
			Oblivion x86: "HKLM\Software\Bethesda Softworks\Oblivion\Install Path"
			Oblivion x64: "HKLM\Software\Wow6432Node\Bethesda Softworks\Oblivion\Install Path"
			*/
	}

	//Gets the string representation of the detected game.
	BOSS_COMMON_EXP string GetGameString() {
		if (game == OBLIVION)
			return "TES IV: Oblivion";
		else if (game == FALLOUT3)
			return "Fallout 3";
		else if (game == NEHRIM)
			return "Nehrim - At Fate's Edge";
		else if (game == FALLOUTNV)
			return "Fallout: New Vegas";
		else if (game == SKYRIM)
			return "TES V: Skyrim";
		else
			return "Game Not Detected";
	}

	//Returns the expeccted master file. Usage internal to BOSS-Common.
	string GameMasterFile() {
		if (game == OBLIVION) 
			return "Oblivion.esm";
		else if (game == FALLOUT3) 
			return "Fallout3.esm";
		else if (game == NEHRIM) 
			return "Nehrim.esm";
		else if (game == FALLOUTNV) 
			return "FalloutNV.esm";
		else if (game == SKYRIM) 
			return "Skyrim.esm";
		else
			return "Game Not Detected";
	}

	//Gets the timestamp of the game's master file. Throws exception if error.
	BOSS_COMMON_EXP time_t GetMasterTime() {
		try {
			if (game == OBLIVION) 
				return fs::last_write_time(data_path / "Oblivion.esm");
			else if (game == FALLOUT3) 
				return fs::last_write_time(data_path / "Fallout3.esm");
			else if (game == NEHRIM) 
				return fs::last_write_time(data_path / "Nehrim.esm");
			else if (game == FALLOUTNV) 
				return fs::last_write_time(data_path / "FalloutNV.esm");
			else if (game == SKYRIM) 
				return fs::last_write_time(data_path / "Skyrim.esm");
			else
				throw boss_error(BOSS_ERROR_NO_MASTER_FILE);
		} catch(fs::filesystem_error e) {
			throw boss_error(BOSS_ERROR_FS_FILE_MOD_TIME_READ_FAIL, GameMasterFile(), e.what());
		}
	}

	BOSS_COMMON_EXP void PerformSortingFunctionality(fs::path file,
												ItemList& modlist,
												ItemList& masterlist,
												RuleList& userlist,
												const time_t esmtime,
												bosslogContents contents) {
		string SE;
		summaryCounters counters;

		BuildWorkingModlist(modlist, masterlist, userlist);
		LOG_INFO("modlist now filled with ordered mods and unknowns.");

		ApplyUserRules(modlist, userlist, contents.userlistMessages);
		LOG_INFO("userlist sorting process finished.");

		if (show_CRCs)
			SE = GetSEPluginInfo(contents.seInfo);

		SortRecognisedMods(modlist, contents.recognisedPlugins, esmtime, counters);

		ListUnrecognisedMods(modlist, contents.unrecognisedPlugins, esmtime, counters);

		PrintBOSSlog(file, contents, counters, SE);
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

		Outputter buffer(log_format);

		LOG_INFO("Starting userlist sort process... Total %" PRIuS " user rules statements to process.", userlist.rules.size());
		vector<Rule>::iterator ruleIter = userlist.rules.begin();
		vector<RuleLine>::iterator lineIter;
		vector<Item>::iterator modlistPos1, modlistPos2;
		unsigned int i = 0;
		for (ruleIter; ruleIter != userlist.rules.end(); ++ruleIter) {
			i++;
			LOG_DEBUG(" -- Processing rule #%" PRIuS ".", i);
			if (!ruleIter->enabled) {
				buffer << LIST_ITEM_CLASS_SUCCESS << "The rule beginning \"" << ruleIter->KeyToString() << ": " << ruleIter->ruleObject << "\" is disabled. Rule skipped.";
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
							buffer << LIST_ITEM_CLASS_WARN << "\"" << ruleIter->ruleObject << "\" is not installed or in the masterlist. Rule skipped.";
							LOG_WARN(" * \"%s\" is not installed.", ruleIter->ruleObject.c_str());
							continue;
						//If it adds a mod already sorted, skip the rule.
						} else if (ruleIter->ruleKey == ADD  && modlistPos1 <= modlist.lastRecognisedPos) {
							buffer << LIST_ITEM_CLASS_WARN << "\"" << ruleIter->ruleObject << "\" is already in the masterlist. Rule skipped.";
							LOG_WARN(" * \"%s\" is already in the masterlist.", ruleIter->ruleObject.c_str());
							continue;
						} else if (ruleIter->ruleKey == OVERRIDE && (modlistPos1 > modlist.lastRecognisedPos || modlistPos1 == modlist.items.end())) {
							buffer << LIST_ITEM_CLASS_ERROR << "\"" << ruleIter->ruleObject << "\" is not in the masterlist, cannot override. Rule skipped.";
							LOG_WARN(" * \"%s\" is not in the masterlist, cannot override.", ruleIter->ruleObject.c_str());
							continue;
						}
						modlistPos2 = modlist.FindItem(fs::path(lineIter->object));  //Find sort mod.
						//Do checks.
						if (modlistPos2 == modlist.items.end()) {  //Handle case of mods that don't exist at all.
							buffer << LIST_ITEM_CLASS_WARN << "\"" << lineIter->object << "\" is not installed, and is not in the masterlist. Rule skipped.";
							LOG_WARN(" * \"%s\" is not installed or in the masterlist.", lineIter->object.c_str());
							continue;
						} else if (modlistPos2 > modlist.lastRecognisedPos) {  //Handle the case of a rule sorting a mod into a position in unsorted mod territory.
							buffer << LIST_ITEM_CLASS_ERROR << "\"" << lineIter->object << "\" is not in the masterlist and has not been sorted by a rule. Rule skipped.";
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
						buffer << LIST_ITEM_CLASS_SUCCESS << "\"" << ruleIter->ruleObject << "\" has been sorted " << lineIter->KeyToString() << " \"" << lineIter->object << "\".";
					} else if (lineIter->key == TOP || lineIter->key == BOTTOM) {
						Item mod;
						modlistPos1 = modlist.FindItem(ruleItem.name);
						//Do checks.
						if (ruleIter->ruleKey == ADD && modlistPos1 == modlist.items.end()) {
							buffer << LIST_ITEM_CLASS_WARN << "\"" << ruleIter->ruleObject << "\" is not installed or in the masterlist. Rule skipped.";
							LOG_WARN(" * \"%s\" is not installed.", ruleIter->ruleObject.c_str());
							continue;
						//If it adds a mod already sorted, skip the rule.
						} else if (ruleIter->ruleKey == ADD  && modlistPos1 <= modlist.lastRecognisedPos) {
							buffer << LIST_ITEM_CLASS_WARN << "\"" << ruleIter->ruleObject << "\" is already in the masterlist. Rule skipped.";
							LOG_WARN(" * \"%s\" is already in the masterlist.", ruleIter->ruleObject.c_str());
							continue;
						} else if (ruleIter->ruleKey == OVERRIDE && (modlistPos1 > modlist.lastRecognisedPos || modlistPos1 == modlist.items.end())) {
							buffer << LIST_ITEM_CLASS_ERROR << "\"" << ruleIter->ruleObject << "\" is not in the masterlist, cannot override. Rule skipped.";
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
							buffer << LIST_ITEM_CLASS_ERROR << "The group \"" << lineIter->object << "\" is not in the masterlist or is malformatted. Rule skipped.";
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
						buffer << LIST_ITEM_CLASS_SUCCESS << "\"" << ruleIter->ruleObject << "\" inserted at the " << lineIter->KeyToString() << " of group \"" << lineIter->object << "\".";
					}
				}
				++lineIter;
				for (lineIter; lineIter != ruleIter->lines.end(); ++lineIter) {  //Message lines.
					if (!lineIter->IsObjectMessage()) {
						buffer << LIST_ITEM_CLASS_WARN << "\"" << lineIter->object << "\" is not a valid message. Rule skipped.";
						LOG_WARN(" * \"%s\" is not a valid message.", lineIter->object.c_str());
						break;
					}
					//Find the mod which will have its messages edited.
					modlistPos1 = modlist.FindItem(ruleItem.name);
					if (modlistPos1 == modlist.items.end()) {  //Rule mod isn't in the modlist (ie. not in masterlist or installed), so can neither add it nor override it.
						buffer << LIST_ITEM_CLASS_WARN << "\"" << ruleIter->ruleObject << "\" is not installed or in the masterlist. Rule skipped.";
						LOG_WARN(" * \"%s\" is not installed.", ruleIter->ruleObject.c_str());
						break;
					}
					Message newMessage = Message(lineIter->ObjectMessageKey(), lineIter->ObjectMessageData());
					if (lineIter->key == REPLACE)  //If the rule is to replace messages, clear existing messages.
						modlistPos1->messages.clear();
					modlistPos1->messages.push_back(newMessage);  //Append message to message list of mod.
					//Output confirmation.
					buffer << LIST_ITEM_CLASS_SUCCESS << "\"" << SPAN_CLASS_MESSAGE_OPEN << lineIter->object << SPAN_CLOSE <<"\"";
					if (lineIter->key == APPEND)
						buffer << " appended to ";
					else
						buffer << " replaced ";
					buffer << "messages attached to \"" << ruleIter->ruleObject << "\".";
				}
			} else if (lineIter->key == BEFORE || lineIter->key == AFTER) {  //Group: Can only sort.
				vector<Item> group;
				//Look for group to sort. Find start and end positions.
				modlistPos1 = modlist.FindItem(ruleItem.name);
				modlistPos2 = modlist.FindGroupEnd(ruleItem.name);
				//Check to see group actually exists.
				if (modlistPos1 != modlist.items.end() || modlistPos2 != modlist.items.end()) {
					buffer << LIST_ITEM_CLASS_ERROR << "The group \"" << ruleIter->ruleObject << "\" is not in the masterlist or is malformatted. Rule skipped.";
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
					buffer << LIST_ITEM_CLASS_ERROR << "The group \"" << lineIter->object << "\" is not in the masterlist or is malformatted. Rule skipped.";
					LOG_WARN(" * \"%s\" is not in the masterlist, or is malformatted.", lineIter->object.c_str());
					continue;
				}
				if (lineIter->key == AFTER)
					modlistPos2++;
				//Now insert the group.
				modlist.items.insert(modlistPos2,group.begin(),group.end());
				//Print success message.
				buffer << LIST_ITEM_CLASS_SUCCESS << "The group \"" << ruleIter->ruleObject << "\" has been sorted " << lineIter->KeyToString() << " the group \"" << lineIter->object << "\".";
			}
			//Now find that last recognised mod and set the iterator again.
			modlist.lastRecognisedPos = modlist.FindItem(lastRecognisedModName);
		}


		if (userlist.rules.empty()) 
			buffer << ITALIC_OPEN << "No valid rules were found in your userlist.txt." << ITALIC_CLOSE;
		outputBuffer = buffer.AsString();
	}

	//Lists Script Extender plugin info in the output buffer. Usage internal to BOSS-Common.
	string GetSEPluginInfo(string& outputBuffer) {
		Outputter buffer(log_format);
		string SE, SELoc, SEPluginLoc;
		if (game == OBLIVION || game == NEHRIM) {
			SE = "OBSE";
			SELoc = "../obse_1_2_416.dll";
			SEPluginLoc = "OBSE/Plugins";
		} else if (game == FALLOUT3) {  //Fallout 3
			SE = "FOSE";
			SELoc = "../fose_loader.exe";
			SEPluginLoc = "FOSE/Plugins";
		} else if (game == FALLOUTNV) {  //Fallout: New Vegas
			SE = "NVSE";
			SELoc = "../nvse_loader.exe";
			SEPluginLoc = "NVSE/Plugins";
		} else if (game == SKYRIM) {  //Skyrim
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

			buffer << LIST_ITEM_SPAN_CLASS_MOD_OPEN << SE << SPAN_CLOSE;
			if (ver.length() != 0)
				buffer << SPAN_CLASS_VERSION_OPEN << "Version: " << ver << SPAN_CLOSE;
			buffer << SPAN_CLASS_CRC_OPEN << "Checksum: " << CRC << SPAN_CLOSE;

			if (!fs::is_directory(data_path / SEPluginLoc)) {
				LOG_DEBUG("Script extender plugins directory not detected");
			} else {
				for (fs::directory_iterator itr(data_path / SEPluginLoc); itr!=fs::directory_iterator(); ++itr) {
					const fs::path filename = itr->path().filename();
					const string ext = Tidy(itr->path().extension().string());
					if (fs::is_regular_file(itr->status()) && ext==".dll") {
						string CRC = IntToHexString(GetCrc32(itr->path()));
						string ver = GetExeDllVersion(itr->path());

						buffer << LIST_ITEM_SPAN_CLASS_MOD_OPEN << filename.string() << SPAN_CLOSE;
						if (ver.length() != 0)
							buffer << SPAN_CLASS_VERSION_OPEN << "Version: " + ver << SPAN_CLOSE;
						buffer << SPAN_CLASS_CRC_OPEN << "Checksum: " + CRC << SPAN_CLOSE;
					}
				}
			}
			outputBuffer = buffer.AsString();
			return SE;
		}
	}

	//Sort recognised mods. Usage internal to BOSS-Common.
	void SortRecognisedMods(ItemList& modlist, string& outputBuffer, const time_t esmtime, summaryCounters& counters) {
		Outputter buffer(log_format);
		time_t modfiletime = 0;
		LOG_INFO("Applying calculated ordering to user files...");
		vector<Item>::iterator iter = modlist.items.begin();
		vector<Message>::iterator messageIter;
		for (iter; iter != modlist.lastRecognisedPos+1; ++iter) {
			if (iter->type == MOD && iter->Exists()) {  //Only act on mods that exist.
				buffer << LIST_ITEM_SPAN_CLASS_MOD_OPEN << iter->name.string() << SPAN_CLOSE;
				if (!skip_version_parse) {
					string version = iter->GetHeader();
					if (!version.empty())
						buffer << SPAN_CLASS_VERSION_OPEN << "Version " << version << SPAN_CLOSE;
				}
				if (iter->IsGhosted()) {
					buffer << SPAN_CLASS_GHOSTED_OPEN << "Ghosted" << SPAN_CLOSE;
					counters.ghosted++;
				}
				if (show_CRCs)
					buffer << SPAN_CLASS_CRC_OPEN << "Checksum: " << IntToHexString(GetCrc32(data_path / iter->name)) << SPAN_CLOSE;
			
				if (!trial_run) {
					modfiletime = esmtime + counters.recognised*60;  //time_t is an integer number of seconds, so adding 60 on increases it by a minute. Using recModNo instead of i to avoid increases for group entries.
					//Re-date file. Provide exception handling in case their permissions are wrong.
					LOG_DEBUG(" -- Setting last modified time for file: \"%s\"", iter->name.string().c_str());
					try {
						fs::last_write_time(data_path / iter->name,modfiletime);
					} catch(fs::filesystem_error e) {
						buffer << SPAN_CLASS_ERROR_OPEN << "Error: Could not change the date of \"" << iter->name.string() << "\", check the Troubleshooting section of the ReadMe for more information and possible solutions." << SPAN_CLOSE;
					}
				}
				//Finally, print the mod's messages.
				if (!iter->messages.empty()) {
					buffer << LIST_OPEN;
					for (messageIter = iter->messages.begin(); messageIter != iter->messages.end(); ++messageIter) {
						buffer << *messageIter;
						counters.messages++;
						if (messageIter->key == WARN)
							counters.warnings++;
						else if (messageIter->key == ERR)
							counters.errors++;
					}
					buffer << LIST_CLOSE;
				}
				counters.recognised++;
			}
		}
		outputBuffer = buffer.AsString();
		LOG_INFO("User file ordering applied successfully.");		
	}

	//List unrecognised mods. Usage internal to BOSS-Common.
	void ListUnrecognisedMods(ItemList& modlist, string& outputBuffer, const time_t esmtime, summaryCounters& counters) {
		Outputter buffer(log_format);
		time_t modfiletime = 0;
		//Find and show found mods not recognised. These are the mods that are found at and after index x in the mods vector.
		//Order their dates to be i days after the master esm to ensure they load last.
		LOG_INFO("Reporting unrecognized mods...");
		vector<Item>::iterator iter = modlist.lastRecognisedPos+1;
		for (iter; iter != modlist.items.end(); ++iter) {
			if (iter->type == MOD && iter->Exists()) {  //Only act on mods that exist.
				buffer << LIST_ITEM_SPAN_CLASS_MOD_OPEN << iter->name.string() << SPAN_CLOSE;
				if (!skip_version_parse) {
					string version = iter->GetHeader();
					if (!version.empty())
						buffer << SPAN_CLASS_VERSION_OPEN << "Version " << version << SPAN_CLOSE;
				}
				if (iter->IsGhosted()) {
					buffer << SPAN_CLASS_GHOSTED_OPEN << "Ghosted" << SPAN_CLOSE;
					counters.ghosted++;
				}
				if (show_CRCs)
					buffer << SPAN_CLASS_CRC_OPEN << "Checksum: " << IntToHexString(GetCrc32(data_path / iter->name)) << SPAN_CLOSE;

				if (!trial_run) {
					modfiletime = esmtime + 86400 + (counters.recognised + counters.unrecognised)*60;  //time_t is an integer number of seconds, so adding 60 on increases it by a minute and adding 86,400 on increases it by a day. Using unrecModNo instead of i to avoid increases for group entries.
					//Re-date file. Provide exception handling in case their permissions are wrong.
					LOG_DEBUG(" -- Setting last modified time for file: \"%s\"", iter->name.string().c_str());
					try {
						fs::last_write_time(data_path / iter->name,modfiletime);
					} catch(fs::filesystem_error e) {
						buffer << SPAN_CLASS_ERROR_OPEN << "Error: Could not change the date of \"" << iter->name.string() << "\", check the Troubleshooting section of the ReadMe for more information and possible solutions." << SPAN_CLOSE;
					}
				}
				counters.unrecognised++;
			}
		}
		if (modlist.lastRecognisedPos+1 == modlist.items.end())
			buffer << ITALIC_OPEN << "No unrecognised plugins." << ITALIC_CLOSE;

		outputBuffer = buffer.AsString();
		LOG_INFO("Unrecognized mods reported.");
	}

	//Prints the full BOSSlog.
	BOSS_COMMON_EXP void PrintBOSSlog(fs::path file, bosslogContents contents, const summaryCounters counters, const string scriptExtender) {

		Outputter bosslog(log_format);
		bosslog.PrintHeader();
		bosslog.SetHTMLSpecialEscape(false);

		/////////////////////////////
		// Print BOSSLog Filters
		/////////////////////////////
	
		if (log_format == HTML) {  //Since this bit is HTML-only, don't bother using formatting placeholders.

			bosslog << "<ul id='filters'>";
			if (UseDarkColourScheme)
				bosslog << "<li><input type='checkbox' checked='checked' id='b1' onclick='swapColorScheme(this)' /><label for='b1'>Use Dark Colour Scheme</label>";
			else
				bosslog << "<li><input type='checkbox' id='b1' onclick='swapColorScheme(this)' /><label for='b1'>Use Dark Colour Scheme</label>";

			if (HideRuleWarnings)
				bosslog << "<li><input type='checkbox' checked='checked' id='b12' onclick='toggleRuleListWarnings(this)' /><label for='b12'>Hide Rule Warnings</label>";
			else
				bosslog << "<li><input type='checkbox' id='b12' onclick='toggleRuleListWarnings(this)' /><label for='b12'>Hide Rule Warnings</label>";
		
			if (HideVersionNumbers)
				bosslog << "<li><input type='checkbox' checked='checked' id='b2' onclick='toggleDisplayCSS(this,\".version\",\"inline\")' /><label for='b2'>Hide Version Numbers</label>";
			else
				bosslog << "<li><input type='checkbox' id='b2' onclick='toggleDisplayCSS(this,\".version\",\"inline\")' /><label for='b2'>Hide Version Numbers</label>";

			if (HideGhostedLabel)
				bosslog << "<li><input type='checkbox' checked='checked' id='b3' onclick='toggleDisplayCSS(this,\".ghosted\",\"inline\")' /><label for='b3'>Hide 'Ghosted' Label</label>";
			else
				bosslog << "<li><input type='checkbox' id='b3' onclick='toggleDisplayCSS(this,\".ghosted\",\"inline\")' /><label for='b3'>Hide 'Ghosted' Label</label>";

			if (HideChecksums)
				bosslog << "<li><input type='checkbox' checked='checked' id='b4' onclick='toggleDisplayCSS(this,\".crc\",\"inline\")' /><label for='b4'>Hide Checksums</label>";
			else
				bosslog << "<li><input type='checkbox' id='b4' onclick='toggleDisplayCSS(this,\".crc\",\"inline\")' /><label for='b4'>Hide Checksums</label>";

			if (HideMessagelessMods)
				bosslog << "<li><input type='checkbox' checked='checked' id='noMessageModFilter' onclick='toggleMods()' /><label for='noMessageModFilter'>Hide Messageless Mods</label>";
			else
				bosslog << "<li><input type='checkbox' id='noMessageModFilter' onclick='toggleMods()' /><label for='noMessageModFilter'>Hide Messageless Mods</label>";

			if (HideGhostedMods)
				bosslog << "<li><input type='checkbox' checked='checked' id='ghostModFilter' onclick='toggleMods()' /><label for='ghostModFilter'>Hide Ghosted Mods</label>";
			else
				bosslog << "<li><input type='checkbox' id='ghostModFilter' onclick='toggleMods()' /><label for='ghostModFilter'>Hide Ghosted Mods</label>";

			if (HideCleanMods)
				bosslog << "<li><input type='checkbox' checked='checked' id='cleanModFilter' onclick='toggleMods()' /><label for='cleanModFilter'>Hide Clean Mods</label>";
			else
				bosslog << "<li><input type='checkbox' id='cleanModFilter' onclick='toggleMods()' /><label for='cleanModFilter'>Hide Clean Mods</label>";

			if (HideAllModMessages)
				bosslog << "<li><input type='checkbox' checked='checked' id='b7' onclick='toggleDisplayCSS(this,\"li ul\",\"block\")' /><label for='b7'>Hide All Mod Messages</label>";
			else
				bosslog << "<li><input type='checkbox' id='b7' onclick='toggleDisplayCSS(this,\"li ul\",\"block\")' /><label for='b7'>Hide All Mod Messages</label>";

			if (HideNotes)
				bosslog << "<li><input type='checkbox' checked='checked' id='b8' onclick='toggleDisplayCSS(this,\".note\",\"table\")' /><label for='b8'>Hide Notes</label>";
			else
				bosslog << "<li><input type='checkbox' id='b8' onclick='toggleDisplayCSS(this,\".note\",\"table\")' /><label for='b8'>Hide Notes</label>";

			if (HideBashTagSuggestions)
				bosslog << "<li><input type='checkbox' checked='checked' id='b9' onclick='toggleDisplayCSS(this,\".tag\",\"table\")' /><label for='b9'>Hide Bash Tag Suggestions</label>";
			else
				bosslog << "<li><input type='checkbox' id='b9' onclick='toggleDisplayCSS(this,\".tag\",\"table\")' /><label for='b9'>Hide Bash Tag Suggestions</label>";

			if (HideRequirements)
				bosslog << "<li><input type='checkbox' checked='checked' id='b10' onclick='toggleDisplayCSS(this,\".req\",\"table\")' /><label for='b10'>Hide Requirements</label>";
			else
				bosslog << "<li><input type='checkbox' id='b10' onclick='toggleDisplayCSS(this,\".req\",\"table\")' /><label for='b10'>Hide Requirements</label>";

			if (HideIncompatibilities)
				bosslog << "<li><input type='checkbox' checked='checked' id='b11' onclick='toggleDisplayCSS(this,\".inc\",\"table\")' /><label for='b11'>Hide Incompatibilities</label>";
			else
				bosslog << "<li><input type='checkbox' id='b11' onclick='toggleDisplayCSS(this,\".inc\",\"table\")' /><label for='b11'>Hide Incompatibilities</label>";

			if (HideDoNotCleanMessages)
				bosslog << "<li><input type='checkbox' checked='checked' id='b13' onclick='toggleDoNotClean(this,\"table\")' /><label for='b13'>Hide 'Do Not Clean' Messages</label>";
			else
				bosslog << "<li><input type='checkbox' id='b13' onclick='toggleDoNotClean(this,\"table\")' /><label for='b13'>Hide 'Do Not Clean' Messages</label>";

			bosslog << "</ul>";
		}


		/////////////////////////////
		// Display Global Messages
		/////////////////////////////

		if (!contents.globalMessages.empty() || !contents.iniParsingError.empty() || !contents.criticalError.empty() || !contents.updaterErrors.empty()) {

			bosslog << HEADING_OPEN << "General Messages" << HEADING_CLOSE << LIST_OPEN;
			if (!contents.criticalError.empty())		//Print masterlist parsing error.
				bosslog << contents.criticalError;
			if (!contents.iniParsingError.empty())		//Print ini parsing error.
				bosslog << contents.iniParsingError;
			bosslog << contents.updaterErrors;

			size_t size = contents.globalMessages.size();
			for (size_t i=0; i<size; i++)
				bosslog << contents.globalMessages[i];  //Print global messages.
			
			bosslog << LIST_CLOSE;
			if (!contents.criticalError.empty()) {  //Exit early.
				bosslog.PrintFooter();
				bosslog.Save(file, true);
				return;
			}
		}

		/////////////////////////////
		// Print Summary
		/////////////////////////////

		bosslog << HEADING_OPEN << "Summary" << HEADING_CLOSE << DIV_OPEN;

		if (contents.oldRecognisedPlugins == contents.recognisedPlugins)
			bosslog << PARAGRAPH << "No change in recognised plugin list since last run.";

		if (!contents.summary.empty())
			bosslog << contents.summary;
	
		bosslog << TABLE_OPEN
			<< TABLE_ROW << TABLE_DATA << "Recognised plugins:" << TABLE_DATA << counters.recognised << TABLE_DATA << "Warning messages:" << TABLE_DATA << counters.warnings
			<< TABLE_ROW << TABLE_DATA << "Unrecognised plugins:" << TABLE_DATA << counters.unrecognised << TABLE_DATA << "Error messages:" << TABLE_DATA << counters.errors
			<< TABLE_ROW << TABLE_DATA << "Ghosted plugins:" << TABLE_DATA << counters.ghosted << TABLE_DATA << "Total number of messages:" << TABLE_DATA << counters.messages
			<< TABLE_ROW << TABLE_DATA << "Total number of plugins:" << TABLE_DATA << (counters.recognised+counters.unrecognised) << TABLE_DATA << TABLE_DATA
			<< TABLE_CLOSE
			<< PARAGRAPH << "Mods sorted by your userlist are counted as recognised, not unrecognised, plugins."
			<< DIV_CLOSE;

		
		/////////////////////////////
		// Display RuleList Messages
		/////////////////////////////
		bosslog.SetHTMLSpecialEscape(false);
		if (!contents.userlistMessages.empty() || !contents.userlistParsingError.empty() || !contents.userlistSyntaxErrors.empty()) {
			
			bosslog << HEADING_OPEN << "Userlist Messages" << HEADING_CLOSE << LIST_ID_USERLIST_MESSAGES_OPEN;
			if (!contents.userlistParsingError.empty())  //First print parser/syntax error messages.
				bosslog << contents.userlistParsingError;

			size_t size = contents.userlistSyntaxErrors.size();
			for (size_t i=0;i<size;i++)
				bosslog << contents.userlistSyntaxErrors[i];

			bosslog << contents.userlistMessages  //Now print the rest of the userlist messages.
				<< LIST_CLOSE;
		}


		/////////////////////////////////
		// Display Script Extender Info
		/////////////////////////////////

		if (!contents.seInfo.empty())
			bosslog << HEADING_OPEN << scriptExtender << " And " << scriptExtender << " Plugin Checksums" << HEADING_CLOSE << LIST_OPEN
				<< contents.seInfo
				<< LIST_CLOSE;


		/////////////////////////////////
		// Display Recognised Mods
		/////////////////////////////////

		if (revert < 1) 
			bosslog << HEADING_OPEN << "Recognised And Re-ordered Plugins" << HEADING_CLOSE << LIST_ID_RECOGNISED_OPEN;
		else if (revert == 1)
			bosslog << HEADING_OPEN << "Restored Load Order (Using modlist.txt)" << HEADING_CLOSE << LIST_ID_RECOGNISED_OPEN;
		else if (revert == 2) 
			bosslog << HEADING_OPEN << "Restored Load Order (Using modlist.old)" << HEADING_CLOSE << LIST_ID_RECOGNISED_OPEN;
		bosslog << contents.recognisedPlugins
			<< LIST_CLOSE;


		/////////////////////////////////
		// Display Unrecognised Mods
		/////////////////////////////////

		bosslog << HEADING_OPEN << "Unrecognised Plugins" << HEADING_CLOSE << DIV_OPEN 
			<< PARAGRAPH << "Reorder these by hand using your favourite mod ordering utility." << LIST_OPEN
			<< contents.unrecognisedPlugins
			<< LIST_CLOSE << DIV_CLOSE;


		////////////////
		// Finish
		////////////////

		bosslog << HEADING_ID_END_OPEN << "Execution Complete" << HEADING_CLOSE;
		bosslog.PrintFooter();
		bosslog.Save(file, true);
	}
}