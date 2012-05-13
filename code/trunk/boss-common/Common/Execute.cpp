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

#include "Common/Execute.h"
#include "Common/Error.h"
#include "Common/Globals.h"
#include "Support/Helpers.h"
#include "Support/Logger.h"
#include "Output/Output.h"

#include <boost/algorithm/string.hpp>

namespace boss {
	using namespace std;

	using boost::algorithm::to_lower_copy;

	////////////////////////
	// Internal Functions
	////////////////////////

	//Lists Script Extender plugin info in the output buffer. Usage internal to BOSS-Common.
	void GetSEPluginInfo(Outputter& buffer, const Game& game) {
		if (!fs::exists(game.SEExecutable()))
			LOG_DEBUG("Script Extender not detected");
		else {
			string CRC = IntToHexString(GetCrc32(game.SEExecutable()));
			string ver = Version(game.SEExecutable()).AsString();

			buffer << LIST_ITEM << SPAN_CLASS_MOD_OPEN << game.ScriptExtender() << SPAN_CLOSE;
			if (ver.length() != 0)
				buffer << SPAN_CLASS_VERSION_OPEN << "Version: " << ver << SPAN_CLOSE;
			if (gl_show_CRCs)
				buffer << SPAN_CLASS_CRC_OPEN << "Checksum: " << CRC << SPAN_CLOSE;

			if (!fs::is_directory(game.SEPluginsFolder())) {
				LOG_DEBUG("Script extender plugins directory not detected");
			} else {
				for (fs::directory_iterator itr(game.SEPluginsFolder()); itr!=fs::directory_iterator(); ++itr) {
					const fs::path filename = itr->path().filename();
					const string ext = to_lower_copy(itr->path().extension().string());
					if (fs::is_regular_file(itr->status()) && ext==".dll") {
						string CRC = IntToHexString(GetCrc32(itr->path()));
						string ver = Version(itr->path()).AsString();

						buffer << LIST_ITEM << SPAN_CLASS_MOD_OPEN << filename.string() << SPAN_CLOSE;
						if (ver.length() != 0)
							buffer << SPAN_CLASS_VERSION_OPEN << "Version: " + ver << SPAN_CLOSE;
						if (gl_show_CRCs)
							buffer << SPAN_CLASS_CRC_OPEN << "Checksum: " + CRC << SPAN_CLOSE;
					}
				}
			}
		}
	}

	//List and redate mods.
	void SortMods(ItemList& modlist, const time_t esmtime, BossLog& bosslog, const Game& game) {
		//Need to obey masters before plugins rule when sorting, but separate display of recognised and unrecognised mods.
		//Also need to display which plugins are active, for both recognised and unrecognised mods.
		bosslog.recognisedPlugins.SetHTMLSpecialEscape(false);
		bosslog.unrecognisedPlugins.SetHTMLSpecialEscape(false);

		//Load active plugin list.
		boost::unordered_set<string> hashset;
		if (fs::exists(game.ActivePluginsFile())) {
			LOG_INFO("Loading plugins.txt into ItemList.");
			ItemList pluginsList;
			try {
				pluginsList.Load(game.ActivePluginsFile());
			} catch (boss_error &e) {
				//Handle exception.
			}
			vector<Item> pluginsEntries = pluginsList.Items();
			size_t pluginsMax = pluginsEntries.size();
			LOG_INFO("Populating hashset with ItemList contents.");
			for (size_t i=0; i<pluginsMax; i++) {
				if (pluginsEntries[i].Type() == MOD)
					hashset.insert(to_lower_copy(pluginsEntries[i].Name()));
			}
			if (game.Id() == SKYRIM) {  //Update.esm and Skyrim.esm are always active.
				if (hashset.find("skyrim.esm") == hashset.end())
					hashset.insert("skyrim.esm");
				if (hashset.find("update.esm") == hashset.end())
					hashset.insert("update.esm");
			}
		}

		//modlist stores recognised mods then unrecognised mods in order. Make a hashset of unrecognised mods.
		boost::unordered_set<string> unrecognised;
		vector<Item> items = modlist.Items();
		size_t max = items.size();
		for (size_t i=modlist.LastRecognisedPos()+1; i < max; i++)
			unrecognised.insert(items[i].Name());

		//Now apply master partition to get modlist to obey masters before plugins rule. 
		//This retains recognised before unrecognised, with the exception of unrecognised masters, which get put after recognised masters.
		modlist.ApplyMasterPartition();
		items = modlist.Items();

		//Now loop through items, redating and outputting. Check against unrecognised hashset and treat unrecognised mods appropriately.
		time_t modfiletime = 0;
		boost::unordered_set<string>::iterator setPos;

		bool isSkyrim1426plus = (game.Id() == SKYRIM && game.GetVersion() >= Version("1.4.26.0"));

		LOG_INFO("Applying calculated ordering to user files...");
		for (vector<Item>::iterator itemIter = items.begin(); itemIter != items.end(); ++itemIter) {
			if (itemIter->Type() == MOD && itemIter->Exists()) {  //Only act on mods that exist.
				Outputter buffer(gl_log_format);
				buffer << LIST_ITEM << SPAN_CLASS_MOD_OPEN << itemIter->Name() << SPAN_CLOSE;
				string version = itemIter->GetVersion().AsString();
				if (!version.empty())
						buffer << SPAN_CLASS_VERSION_OPEN << "Version " << version << SPAN_CLOSE;
				if (hashset.find(to_lower_copy(itemIter->Name())) != hashset.end())  //Plugin is active.
					buffer << SPAN_CLASS_ACTIVE_OPEN << "Active" << SPAN_CLOSE;
				else
					bosslog.inactive++;
				if (gl_show_CRCs && itemIter->IsGhosted()) {
					buffer << SPAN_CLASS_CRC_OPEN << "Checksum: " << IntToHexString(GetCrc32(game.DataFolder() / fs::path(itemIter->Name() + ".ghost"))) << SPAN_CLOSE;
				} else if (gl_show_CRCs)
					buffer << SPAN_CLASS_CRC_OPEN << "Checksum: " << IntToHexString(GetCrc32(game.DataFolder() / itemIter->Name())) << SPAN_CLOSE;
		
		/*		if (itemIter->IsFalseFlagged()) {
					itemIter->InsertMessage(0, Message(WARN, "This plugin's internal master bit flag value does not match its file extension. This issue should be reported to the mod's author, and can be fixed by changing the file extension from .esp to .esm or vice versa."));
					counters.warnings++;
				}
		*/	
				if (!gl_trial_run && !itemIter->IsGameMasterFile() && !isSkyrim1426plus) {
					//time_t is an integer number of seconds, so adding 60 on increases it by a minute. Using recModNo instead of i to avoid increases for group entries.
					LOG_DEBUG(" -- Setting last modified time for file: \"%s\"", itemIter->Name().c_str());
					try {
						itemIter->SetModTime(esmtime + (bosslog.recognised + bosslog.unrecognised)*60);
					} catch(boss_error &e) {
						itemIter->InsertMessage(0, Message(ERR, "Error: " + e.getString()));
						LOG_ERROR(" * Error: %s", e.getString().c_str());
					}
				}
				//Print the mod's messages. Unrecognised plugins might have a redate error message.
				if (!itemIter->Messages().empty()) {
					vector<Message> messages = itemIter->Messages();
					size_t jmax = messages.size();
					buffer << LIST_OPEN;
					for (size_t j=0; j < jmax; j++) {
						buffer << messages[j];
						bosslog.messages++;
						if (messages[j].Key() == WARN)
							bosslog.warnings++;
						else if (messages[j].Key() == ERR)
							bosslog.errors++;
					}
					buffer << LIST_CLOSE;
				}
				if (unrecognised.find(itemIter->Name()) == unrecognised.end()) {  //Recognised plugin.
					bosslog.recognised++;
					bosslog.recognisedPlugins << buffer.AsString();
				} else {  //Unrecognised plugin.
					bosslog.unrecognised++;
					bosslog.unrecognisedPlugins << buffer.AsString();
				}
			}
		}
		LOG_INFO("User plugin ordering applied successfully.");
	}

	//Structures necessary for case-insensitive hashsets used in BuildWorkingModlist. Taken from the BOOST docs.
	struct iequal_to : std::binary_function<std::string, std::string, bool> {
		iequal_to() {}
        explicit iequal_to(std::locale const& l) : locale_(l) {}

        template <typename String1, typename String2>
        bool operator()(String1 const& x1, String2 const& x2) const {
            return boost::algorithm::iequals(x1, x2, locale_);
        }
	private:
		std::locale locale_;
	};

	struct ihash : std::unary_function<std::string, std::size_t> {
		ihash() {}
        explicit ihash(std::locale const& l) : locale_(l) {}

        template <typename String>
        std::size_t operator()(String const& x) const
        {
            std::size_t seed = 0;

            for(typename String::const_iterator it = x.begin();
                it != x.end(); ++it)
            {
                boost::hash_combine(seed, std::toupper(*it, locale_));
            }

            return seed;
        }
    private:
        std::locale locale_;
	};


	//////////////////////////////////
	// Externally-Visible Functions
	//////////////////////////////////

	BOSS_COMMON void PerformSortingFunctionality(fs::path file,
												BossLog& bosslog,
												ItemList& modlist,
												ItemList& masterlist,
												RuleList& userlist,
												const time_t esmtime,
												const Game& game) {
		BuildWorkingModlist(modlist, masterlist, userlist);
		LOG_INFO("masterlist now filled with ordered mods and modlist filled with unknowns.");

		//Check to see that masterlist and modlist obey the masters before plugins rule.
		//If they don't, add a global warning saying so.
		try {
			//Modlist.
			size_t size = modlist.Items().size();
			size_t pos = modlist.GetNextMasterPos(modlist.GetLastMasterPos() + 1);
			if (pos != size)   //Masters exist after the initial set of masters. Not allowed. Since order is not decided by BOSS though, silently fix.
				modlist.ApplyMasterPartition();
			//Masterlist.
			size = masterlist.Items().size();
			pos = masterlist.GetNextMasterPos(masterlist.GetLastMasterPos() + 1);
			if (pos != size)  //Masters exist after the initial set of masters. Not allowed.
				throw boss_error(BOSS_ERROR_PLUGIN_BEFORE_MASTER, masterlist.ItemAt(pos).Name());
		} catch (boss_error &e) {
			bosslog.globalMessages.push_back(Message(SAY, "The order of plugins set by BOSS differs from their order in its masterlist, as one or more of the installed plugins is false-flagged. For more information, see the readme section on False-Flagged Plugins."));
			masterlist.ApplyMasterPartition();
			LOG_WARN("The order of plugins set by BOSS differs from their order in its masterlist, as one or more of the installed plugins is false-flagged. For more information, see the readme section on False-Flagged Plugins.");
		}

		//Now stick them back together.
		modlist.Insert(0,masterlist.Items(), 0, masterlist.Items().size());
		modlist.LastRecognisedPos(masterlist.LastRecognisedPos());

		ApplyUserRules(modlist, userlist, bosslog.userRules);
		LOG_INFO("userlist sorting process finished.");

		GetSEPluginInfo(bosslog.sePlugins, game);
		bosslog.scriptExtender = game.ScriptExtender();
		bosslog.gameName = game.Name();

		SortMods(modlist, esmtime, bosslog, game);

		//Now set the load order using Skyrim method.
		if (game.Id() == SKYRIM && game.GetVersion() >= Version("1.4.26.0")) {
			try {
				modlist.SavePluginNames(game.LoadOrderFile(), false, false);
				modlist.SavePluginNames(game.ActivePluginsFile(), true, true);
			} catch (boss_error &e) {
				bosslog.criticalError << LIST_ITEM_CLASS_ERROR << "Critical Error: " << e.getString() << LINE_BREAK
					<< "Check the Troubleshooting section of the ReadMe for more information and possible solutions." << LINE_BREAK
					<< "Utility will end now.";
			}
		}

		try {
			bosslog.Save(file, true);
		} catch (boss_error &e) {
			LOG_ERROR("Critical Error: %s", e.getString().c_str());
		}
	}

	//Create a modlist containing all the mods that are installed or referenced in the userlist with their masterlist messages.
	//Returns the vector position of the last recognised mod in modlist.
	BOSS_COMMON void BuildWorkingModlist(ItemList& modlist, ItemList& masterlist, RuleList& userlist) {
		//Add all modlist and userlist mods and groups referenced in userlist to a hashset to optimise comparison against masterlist.
		boost::unordered_set<string, ihash, iequal_to> mHashset, uHashset, addedItems;  //Holds mods and groups for checking against masterlist.
		boost::unordered_set<string>::iterator setPos;

		LOG_INFO("Populating hashset with modlist.");
		vector<Item> items = modlist.Items();
		size_t modlistSize = items.size();
		for (size_t i=0; i<modlistSize; i++) {
			if (items[i].Type() == MOD)
				mHashset.insert(items[i].Name());
		}

		LOG_INFO("Populating hashset with userlist.");
		vector<Rule> rules = userlist.Rules();
		size_t userlistSize = rules.size();
		for (size_t i=0; i<userlistSize; i++) {
			Item ruleObject(rules[i].Object());
			if (uHashset.find(ruleObject.Name()) == uHashset.end())  //Mod or group not already in hashset, so add to hashset.
				uHashset.insert(ruleObject.Name());
			if (rules[i].Key() != FOR) {  //First line is a sort line.
				Item sortObject(rules[i].LineAt(0).Object());
				if (uHashset.find(sortObject.Name()) == uHashset.end())  //Mod or group not already in hashset, so add to hashset.
					uHashset.insert(sortObject.Name());
			}
		}

		LOG_INFO("Comparing hashset against masterlist.");
		size_t modlistPos;
		items = masterlist.Items();
		size_t max = masterlist.Items().size();
		vector<Item> holdingVec;
		for (size_t i=0; i < max; i++) {
			if (items[i].Type() == MOD) {
				//Check to see if the mod is in the hashset. If it is, or its ghosted version is, also check if 
				//the mod is already in the holding vector. If not, add it.
				setPos = mHashset.find(items[i].Name());
				if (setPos != mHashset.end())  //Mod is installed. Ensure that correct case is recorded.
					items[i].Name(*setPos);
				else if (uHashset.find(items[i].Name()) == uHashset.end())  //Mod not in modlist or userlist, skip.
					continue;
				
				if (addedItems.find(items[i].Name()) == addedItems.end()) {			//The mod is not already in the holding vector.
					holdingVec.push_back(items[i]);
					addedItems.insert(items[i].Name());								//Record it in the holding vector.
					modlistPos = modlist.FindItem(items[i].Name());
					if (modlistPos != modlistSize)
						modlist.Erase(modlistPos);  //Remove recognised plugin from modlist.
				}
			} else if (items[i].Type() == BEGINGROUP || items[i].Type() == ENDGROUP) { //Group lines must stay recorded.
				if (uHashset.find(items[i].Name()) == uHashset.end())  //Mod not in modlist or userlist, skip.
					continue;

				if (addedItems.find(items[i].Name()) == addedItems.end()) {
					holdingVec.push_back(items[i]);
					addedItems.insert(items[i].Name());								//Record it in the holding vector.
					//Don't need to erase groups from modlist, even if they are present (which they aren't, in usual modlist).
				}
			}
		}
		masterlist.Items(holdingVec);  //Masterlist now only contains the items needed to sort the user's mods.
		masterlist.LastRecognisedPos(masterlist.Items().size() - 1);
	}

	//Applies the userlist rules to the working modlist.
	BOSS_COMMON void ApplyUserRules(ItemList& modlist, RuleList& userlist, Outputter& buffer) {
		vector<Rule> rules = userlist.Rules();
		if (rules.empty())
			return;
		//Because erase operations invalidate iterators after the position(s) erased, the last recognised mod needs to be recorded, then
		//set correctly again after all operations have completed.
		//Note that if a mod is sorted after the last recognised mod by the userlist, it becomes the last recognised mod, and the item will
		//need to be re-assigned to this item. This only occurs for BEFORE/AFTER plugin sorting rules.
		string lastRecognisedItem = modlist.ItemAt(modlist.LastRecognisedPos()).Name();

		LOG_INFO("Starting userlist sort process... Total %" PRIuS " user rules statements to process.", rules.size());
		vector<Rule>::iterator ruleIter = rules.begin();
		size_t modlistPos1, modlistPos2;
		uint32_t ruleNo = 0;
		for (ruleIter; ruleIter != rules.end(); ++ruleIter) {
			ruleNo++;
			LOG_DEBUG(" -- Processing rule #%" PRIuS ".", ruleNo);
			if (!ruleIter->Enabled()) {
				buffer << TABLE_ROW_CLASS_WARN << TABLE_DATA << *ruleIter << TABLE_DATA << "✗" << TABLE_DATA << "Rule is disabled.";
				LOG_INFO("Rule beginning \"%s: %s\" is disabled. Rule skipped.", ruleIter->KeyToString().c_str(), ruleIter->Object().c_str());
				continue;
			}
			bool messageLineFail = false;
			size_t i = 0;
			vector<RuleLine> lines = ruleIter->Lines();
			size_t max = lines.size();
			Item ruleItem(ruleIter->Object());
			if (ruleItem.IsPlugin()) {  //Plugin: Can sort or add messages.
				if (ruleIter->Key() != FOR) { //First non-rule line is a sort line.
					if (lines[i].Key() == BEFORE || lines[i].Key() == AFTER) {
						Item mod;
						modlistPos1 = modlist.FindItem(ruleItem.Name());
						//Do checks.
						if (ruleIter->Key() == ADD && modlistPos1 == modlist.Items().size()) {
							buffer << TABLE_ROW_CLASS_WARN << TABLE_DATA << *ruleIter << TABLE_DATA << "✗" << TABLE_DATA << VAR_OPEN << ruleIter->Object() << VAR_CLOSE << " is not installed or in the masterlist.";
							LOG_WARN(" * \"%s\" is not installed.", ruleIter->Object().c_str());
							continue;
						//If it adds a mod already sorted, skip the rule.
						} else if (ruleIter->Key() == ADD  && modlistPos1 <= modlist.LastRecognisedPos()) {
							buffer << TABLE_ROW_CLASS_WARN << TABLE_DATA << *ruleIter << TABLE_DATA << "✗" << TABLE_DATA << VAR_OPEN << ruleIter->Object() << VAR_CLOSE << " is already in the masterlist.";
							LOG_WARN(" * \"%s\" is already in the masterlist.", ruleIter->Object().c_str());
							continue;
						} else if (ruleIter->Key() == OVERRIDE && (modlistPos1 > modlist.LastRecognisedPos())) {
							buffer << TABLE_ROW_CLASS_ERROR << TABLE_DATA << *ruleIter << TABLE_DATA << "✗" << TABLE_DATA << VAR_OPEN << ruleIter->Object() << VAR_CLOSE << " is not in the masterlist, cannot override.";
							LOG_WARN(" * \"%s\" is not in the masterlist, cannot override.", ruleIter->Object().c_str());
							continue;
						}
						modlistPos2 = modlist.FindItem(lines[i].Object());  //Find sort mod.
						//Do checks.
						if (modlistPos2 == modlist.Items().size()) {  //Handle case of mods that don't exist at all.
							buffer << TABLE_ROW_CLASS_WARN << TABLE_DATA << *ruleIter << TABLE_DATA << "✗" << TABLE_DATA << VAR_OPEN << lines[i].Object() << VAR_CLOSE << " is not installed, and is not in the masterlist.";
							LOG_WARN(" * \"%s\" is not installed or in the masterlist.", lines[i].Object().c_str());
							continue;
						} else if (modlistPos2 > modlist.LastRecognisedPos()) {  //Handle the case of a rule sorting a mod into a position in unsorted mod territory.
							buffer << TABLE_ROW_CLASS_ERROR << TABLE_DATA << *ruleIter << TABLE_DATA << "✗" << TABLE_DATA << VAR_OPEN << lines[i].Object() << VAR_CLOSE << " is not in the masterlist and has not been sorted by a rule.";
							LOG_WARN(" * \"%s\" is not in the masterlist and has not been sorted by a rule.", lines[i].Object().c_str());
							continue;
						} else if (lines[i].Key() == AFTER && modlistPos2 == modlist.LastRecognisedPos())
							lastRecognisedItem = modlist.ItemAt(modlistPos1).Name();
						mod = modlist.ItemAt(modlistPos1);  //Record the rule mod in a new variable.
						modlist.Erase(modlistPos1);  //Now remove the rule mod from its old position. This breaks all modlist iterators active.
						//Need to find sort mod pos again, to fix iterator.
						modlistPos2 = modlist.FindItem(lines[i].Object());  //Find sort mod.
						//Insert the mod into its new position.
						if (lines[i].Key() == AFTER)
							++modlistPos2;
						modlist.Insert(modlistPos2, mod);
					} else if (lines[i].Key() == TOP || lines[i].Key() == BOTTOM) {
						Item mod;
						modlistPos1 = modlist.FindItem(ruleItem.Name());
						//Do checks.
						if (ruleIter->Key() == ADD && modlistPos1 == modlist.Items().size()) {
							buffer << TABLE_ROW_CLASS_WARN << TABLE_DATA << *ruleIter << TABLE_DATA << "✗" << TABLE_DATA << VAR_OPEN << ruleIter->Object() << VAR_CLOSE << " is not installed or in the masterlist.";
							LOG_WARN(" * \"%s\" is not installed.", ruleIter->Object().c_str());
							continue;
						//If it adds a mod already sorted, skip the rule.
						} else if (ruleIter->Key() == ADD  && modlistPos1 <= modlist.LastRecognisedPos()) {
							buffer << TABLE_ROW_CLASS_WARN << TABLE_DATA << *ruleIter << TABLE_DATA << "✗" << TABLE_DATA << VAR_OPEN << ruleIter->Object() << VAR_CLOSE << " is already in the masterlist.";
							LOG_WARN(" * \"%s\" is already in the masterlist.", ruleIter->Object().c_str());
							continue;
						} else if (ruleIter->Key() == OVERRIDE && (modlistPos1 > modlist.LastRecognisedPos() || modlistPos1 == modlist.Items().size())) {
							buffer << TABLE_ROW_CLASS_ERROR << TABLE_DATA << *ruleIter << TABLE_DATA << "✗" << TABLE_DATA << VAR_OPEN << ruleIter->Object() << VAR_CLOSE << " is not in the masterlist, cannot override.";
							LOG_WARN(" * \"%s\" is not in the masterlist, cannot override.", ruleIter->Object().c_str());
							continue;
						}
						//Find the group to sort relative to.
						if (lines[i].Key() == TOP)
							modlistPos2 = modlist.FindItem(lines[i].Object()) + 1;  //Find the start, and increment by 1 so that mod is inserted after start.
						else
							modlistPos2 = modlist.FindGroupEnd(lines[i].Object());  //Find the end.
						//Check that the sort group actually exists.
						if (modlistPos2 == modlist.Items().size()) {
							buffer << TABLE_ROW_CLASS_ERROR << TABLE_DATA << *ruleIter << TABLE_DATA << "✗" << TABLE_DATA << "The group " << VAR_OPEN << lines[i].Object() << VAR_CLOSE << " is not in the masterlist or is malformatted.";
							LOG_WARN(" * \"%s\" is not in the masterlist, or is malformatted.", lines[i].Object().c_str());
							continue;
						}
						mod = modlist.ItemAt(modlistPos1);  //Record the rule mod in a new variable.
						modlist.Erase(modlistPos1);  //Now remove the rule mod from its old position. This breaks all modlist iterators active.
						//Need to find group pos again, to fix iterators.
						if (lines[i].Key() == TOP)
							modlistPos2 = modlist.FindItem(lines[i].Object()) + 1;  //Find the start, and increment by 1 so that mod is inserted after start.
						else
							modlistPos2 = modlist.FindGroupEnd(lines[i].Object());  //Find the end.
						modlist.Insert(modlistPos2, mod);  //Now insert the mod into the group. This breaks all modlist iterators active.
					}
					i++;
				}
				for (i; i < max; i++) {  //Message lines.
					//Find the mod which will have its messages edited.
					modlistPos1 = modlist.FindItem(ruleItem.Name());
					if (modlistPos1 == modlist.Items().size()) {  //Rule mod isn't in the modlist (ie. not in masterlist or installed), so can neither add it nor override it.
						buffer << TABLE_ROW_CLASS_WARN << TABLE_DATA << *ruleIter << TABLE_DATA << "✗" << TABLE_DATA << VAR_OPEN << ruleIter->Object() << VAR_CLOSE << " is not installed or in the masterlist.";
						LOG_WARN(" * \"%s\" is not installed.", ruleIter->Object().c_str());
						messageLineFail = true;
						break;
					}
					vector<Item> items = modlist.Items();
					if (lines[i].Key() == REPLACE)  //If the rule is to replace messages, clear existing messages.
						items[modlistPos1].ClearMessages();
					//Append message to message list of mod.
					items[modlistPos1].InsertMessage(items[modlistPos1].Messages().size(), lines[i].ObjectAsMessage());
					modlist.Items(items);
				}
			} else if (lines[i].Key() == BEFORE || lines[i].Key() == AFTER) {  //Group: Can only sort.
				vector<Item> group;
				//Look for group to sort. Find start and end positions.
				modlistPos1 = modlist.FindItem(ruleItem.Name());
				modlistPos2 = modlist.FindGroupEnd(ruleItem.Name());
				//Check to see group actually exists.
				if (modlistPos1 == modlist.Items().size() || modlistPos2 == modlist.Items().size()) {
					buffer << TABLE_ROW_CLASS_ERROR << TABLE_DATA << *ruleIter << TABLE_DATA << "✗" << TABLE_DATA << "The group " << VAR_OPEN << ruleIter->Object() << VAR_CLOSE << " is not in the masterlist or is malformatted.";
					LOG_WARN(" * \"%s\" is not in the masterlist, or is malformatted.", ruleIter->Object().c_str());
					continue;
				}
				//Copy the start, end and everything in between to a new variable.
				vector<Item> items = modlist.Items();
				group.assign(items.begin() + modlistPos1, items.begin() + modlistPos2+1);
				//Now erase group from modlist. This breaks the lastRecognisedPos iterator, so that will be reset after rule application.
				modlist.Erase(modlistPos1,modlistPos2+1);
				//Find the group to sort relative to and insert it before or after it as appropriate.
				if (lines[i].Key() == BEFORE)
					modlistPos2 = modlist.FindItem(lines[i].Object());  //Find the start.
				else
					modlistPos2 = modlist.FindGroupEnd(lines[i].Object());  //Find the end, and add one, as inserting works before the given element.
				//Check that the sort group actually exists.
				if (modlistPos2 == modlist.Items().size()) {
					modlist.Insert(modlistPos1, group, 0, group.size());  //Insert the group back in its old position.
					buffer << TABLE_ROW_CLASS_ERROR << TABLE_DATA << *ruleIter << TABLE_DATA << "✗" << TABLE_DATA << "The group " << VAR_OPEN << lines[i].Object() << VAR_CLOSE << " is not in the masterlist or is malformatted.";
					LOG_WARN(" * \"%s\" is not in the masterlist, or is malformatted.", lines[i].Object().c_str());
					continue;
				}
				if (lines[i].Key() == AFTER)
					modlistPos2++;
				//Now insert the group.
				modlist.Insert(modlistPos2, group, 0, group.size());
			}
			if (!messageLineFail)  //Print success message.
				buffer << TABLE_ROW_CLASS_SUCCESS << TABLE_DATA << *ruleIter << TABLE_DATA << "✓" << TABLE_DATA;
			//Now find that last recognised mod and set the iterator again.
			modlist.LastRecognisedPos(modlist.FindLastItem(lastRecognisedItem));
		}
	}
}