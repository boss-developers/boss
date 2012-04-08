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
	string GetSEPluginInfo(string& outputBuffer) {
		Outputter buffer(gl_log_format);
		string SE;
		fs::path SELoc, SEPluginLoc;
		if (gl_current_game == OBLIVION || gl_current_game == NEHRIM) {
			SE = "OBSE";
			SELoc = data_path.parent_path() / "obse_1_2_416.dll";
			SEPluginLoc = data_path / "OBSE" / "Plugins";
		} else if (gl_current_game == FALLOUT3) {  //Fallout 3
			SE = "FOSE";
			SELoc = data_path.parent_path() / "fose_loader.exe";
			SEPluginLoc = data_path / "FOSE" / "Plugins";
		} else if (gl_current_game == FALLOUTNV) {  //Fallout: New Vegas
			SE = "NVSE";
			SELoc = data_path.parent_path() / "nvse_loader.exe";
			SEPluginLoc = data_path / "NVSE" / "Plugins";
		} else if (gl_current_game == SKYRIM) {  //Skyrim
			SE = "SKSE";
			SELoc = data_path.parent_path() / "skse_loader.exe";
			SEPluginLoc = data_path / "SKSE" / "Plugins";
		} else if (gl_current_game == MORROWIND) {
			SE = "MWSE";
			SELoc = data_path.parent_path() / "MWSE.dll";  //MWSE works differently from the other script extenders.
			SEPluginLoc = data_path / "MWSE" / "Plugins";  //AFAIK MWSE doesn't have a plugin system, but setting a path will be handled correctly below.
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
			if (gl_show_CRCs)
				buffer << SPAN_CLASS_CRC_OPEN << "Checksum: " << CRC << SPAN_CLOSE;

			if (!fs::is_directory(SEPluginLoc)) {
				LOG_DEBUG("Script extender plugins directory not detected");
			} else {
				for (fs::directory_iterator itr(SEPluginLoc); itr!=fs::directory_iterator(); ++itr) {
					const fs::path filename = itr->path().filename();
					const string ext = to_lower_copy(itr->path().extension().string());
					if (fs::is_regular_file(itr->status()) && ext==".dll") {
						string CRC = IntToHexString(GetCrc32(itr->path()));
						string ver = GetExeDllVersion(itr->path());

						buffer << LIST_ITEM_SPAN_CLASS_MOD_OPEN << filename.string() << SPAN_CLOSE;
						if (ver.length() != 0)
							buffer << SPAN_CLASS_VERSION_OPEN << "Version: " + ver << SPAN_CLOSE;
						if (gl_show_CRCs)
							buffer << SPAN_CLASS_CRC_OPEN << "Checksum: " + CRC << SPAN_CLOSE;
					}
				}
			}
			outputBuffer = buffer.AsString();
			return SE;
		}
	}

	//List and redate mods.
	void SortMods(ItemList& modlist, bosslogContents& contents, const time_t esmtime, summaryCounters& counters) {
		//Need to obey masters before plugins rule when sorting, but separate display of recognised and unrecognised mods.
		//Also need to display which plugins are active, for both recognised and unrecognised mods.

		//Load active plugin list.
		boost::unordered_set<string> hashset;
		if (fs::exists(plugins_path())) {
			LOG_INFO("Loading plugins.txt into ItemList.");
			ItemList pluginsList;
			try {
				pluginsList.Load(plugins_path());
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
			if (gl_current_game == SKYRIM) {  //Update.esm and Skyrim.esm are always active.
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

		bool isSkyrim1426plus = (gl_current_game == SKYRIM && Version(GetExeDllVersion(data_path.parent_path() / "TESV.exe")) >= Version("1.4.26.0"));

		LOG_INFO("Applying calculated ordering to user files...");
		for (vector<Item>::iterator itemIter = items.begin(); itemIter != items.end(); ++itemIter) {
			if (itemIter->Type() == MOD && itemIter->Exists()) {  //Only act on mods that exist.
				Outputter buffer(gl_log_format);
				buffer << LIST_ITEM_SPAN_CLASS_MOD_OPEN << itemIter->Name() << SPAN_CLOSE;
			/*	if (unrecognised.find(itemIter->Name()) != unrecognised.end())
					buffer << BUTTON_SUBMIT_PLUGIN;*/
				if (!gl_skip_version_parse) {
					string version = itemIter->GetVersion();
					if (!version.empty())
						buffer << SPAN_CLASS_VERSION_OPEN << "Version " << version << SPAN_CLOSE;
				}
				if (hashset.find(to_lower_copy(itemIter->Name())) != hashset.end())  //Plugin is active.
					buffer << SPAN_CLASS_ACTIVE_OPEN << "Active" << SPAN_CLOSE;
				if (itemIter->IsGhosted()) {
					buffer << SPAN_CLASS_GHOSTED_OPEN << "Ghosted" << SPAN_CLOSE;
					counters.ghosted++;
					if (gl_show_CRCs)
						buffer << SPAN_CLASS_CRC_OPEN << "Checksum: " << IntToHexString(GetCrc32(data_path / fs::path(itemIter->Name() + ".ghost"))) << SPAN_CLOSE;
				} else if (gl_show_CRCs)
					buffer << SPAN_CLASS_CRC_OPEN << "Checksum: " << IntToHexString(GetCrc32(data_path / itemIter->Name())) << SPAN_CLOSE;
			
				if (!gl_trial_run && !itemIter->IsGameMasterFile() && !isSkyrim1426plus) {
					//time_t is an integer number of seconds, so adding 60 on increases it by a minute. Using recModNo instead of i to avoid increases for group entries.
					LOG_DEBUG(" -- Setting last modified time for file: \"%s\"", itemIter->Name().c_str());
					try {
						itemIter->SetModTime(esmtime + (counters.recognised + counters.unrecognised)*60);
					} catch(boss_error &e) {
						buffer << SPAN_CLASS_ERROR_OPEN << "Error: " << e.getString() << SPAN_CLOSE;
						LOG_ERROR(" * Error: %s", e.getString().c_str());
					}
				}
				if (unrecognised.find(itemIter->Name()) == unrecognised.end()) {  //Recognised plugin.
					//Finally, print the mod's messages.
					if (!itemIter->Messages().empty()) {
						vector<Message> messages = itemIter->Messages();
						size_t jmax = messages.size();
						buffer << LIST_OPEN;
						for (size_t j=0; j < jmax; j++) {
							buffer << messages[j];
							counters.messages++;
							if (messages[j].Key() == WARN)
								counters.warnings++;
							else if (messages[j].Key() == ERR)
								counters.errors++;
						}
			/*			if (itemIter->IsFalseFlagged())
							buffer << Message(WARN, "This plugin's internal master bit flag value does not match its file extension. This issue should be reported to the mod's author, and can be fixed by changing the file extension from .esp to .esm or vice versa.");
			*/			buffer << LIST_CLOSE;
			/*		} else if (itemIter->IsFalseFlagged()) {
						buffer << LIST_OPEN
								<< Message(WARN, "This plugin's internal master bit flag value does not match its file extension. This issue should be reported to the mod's author, and can be fixed by changing the file extension from .esp to .esm or vice versa.")
								<< LIST_CLOSE;
						counters.warnings++;
			*/		}
					counters.recognised++;
					contents.recognisedPlugins += buffer.AsString();
				} else {  //Unrecognised plugin.
					counters.unrecognised++;
					contents.unrecognisedPlugins += buffer.AsString();
				}
			}
		}
		LOG_INFO("User plugin ordering applied successfully.");
	}

	//Prints the full BOSSlog.
	void PrintBOSSlog(fs::path file, bosslogContents contents, const summaryCounters counters, const string scriptExtender) {

		Outputter bosslog(gl_log_format);
		bosslog.PrintHeader();
		bosslog.SetHTMLSpecialEscape(false);

		// Print BOSSLog Filters
		if (gl_log_format == HTML) {  //Since this bit is HTML-only, don't bother using formatting placeholders.
			bosslog << "<ul id='filters'>";
			
			bosslog << "<li><input type='checkbox' ";
			if (UseDarkColourScheme)
				bosslog << "checked='checked' ";
			bosslog << "id='b1' onclick='swapColorScheme(this)' /><label for='b1'>Use Dark Colour Scheme</label>";

			bosslog << "<li><input type='checkbox' ";
			if (HideRuleWarnings)
				bosslog << "checked='checked' ";
			bosslog << "id='b2' onclick='toggleRuleListWarnings(this)' /><label for='b2'>Hide Rule Warnings</label>";
		
			bosslog << "<li><input type='checkbox' ";
			if (HideVersionNumbers)
				bosslog << "checked='checked' ";
			bosslog << "id='b3' onclick='toggleDisplayCSS(this,\".version\",\"inline\")' /><label for='b3'>Hide Version Numbers</label>";

			bosslog << "<li><input type='checkbox' ";
			if (HideGhostedLabel)
				bosslog << "checked='checked' ";
			bosslog << "id='b4' onclick='toggleDisplayCSS(this,\".ghosted\",\"inline\")' /><label for='b4'>Hide 'Ghosted' Label</label>";

			bosslog << "<li><input type='checkbox' ";
			if (HideInactivePlugins)
				bosslog << "checked='checked' ";
			bosslog << "id='b16' onclick='toggleDisplayCSS(this,\".active\")' /><label for='b16'>Hide 'Active' Label</label>";
			
			bosslog << "<li><input type='checkbox' ";
			if (HideChecksums)
				bosslog << "checked='checked' ";
			bosslog << "id='b5' onclick='toggleDisplayCSS(this,\".crc\")' /><label for='b5'>Hide Checksums</label>";

			bosslog << "<li><input type='checkbox' ";
			if (HideInactivePlugins)
				bosslog << "checked='checked' ";
			bosslog << "id='b15' onclick='toggleMessages()' /><label for='b15'>Hide Inactive Mods</label>";
			
			bosslog << "<li><input type='checkbox' ";
			if (HideMessagelessMods)
				bosslog << "checked='checked' ";
			bosslog << "id='b6' onclick='toggleMessages()' /><label for='b6'>Hide Messageless Mods</label>";
			
			bosslog << "<li><input type='checkbox' ";
			if (HideGhostedMods)
				bosslog << "checked='checked' ";
			bosslog << "id='b7' onclick='toggleMessages()' /><label for='b7'>Hide Ghosted Mods</label>";
			
			bosslog << "<li><input type='checkbox' ";
			if (HideCleanMods)
				bosslog << "checked='checked' ";
			bosslog << "id='b8' onclick='toggleMessages()' /><label for='b8'>Hide Clean Mods</label>";
			
			bosslog << "<li><input type='checkbox' ";
			if (HideAllModMessages)
				bosslog << "checked='checked' ";
			bosslog << "id='b9' onclick='toggleMessages()' /><label for='b9'>Hide All Mod Messages</label>";
			
			bosslog << "<li><input type='checkbox' ";
			if (HideNotes)
				bosslog << "checked='checked' ";
			bosslog << "id='b10' onclick='toggleMessages()' /><label for='b10'>Hide Notes</label>";
			
			bosslog << "<li><input type='checkbox' ";
			if (HideBashTagSuggestions)
				bosslog << "checked='checked' ";
			bosslog << "id='b11' onclick='toggleMessages()' /><label for='b11'>Hide Bash Tag Suggestions</label>";
			
			bosslog << "<li><input type='checkbox' ";
			if (HideRequirements)
				bosslog << "checked='checked' ";
			bosslog << "id='b12' onclick='toggleMessages()' /><label for='b12'>Hide Requirements</label>";
			
			bosslog << "<li><input type='checkbox' ";
			if (HideIncompatibilities)
				bosslog << "checked='checked' ";
			bosslog << "id='b13' onclick='toggleMessages()' /><label for='b13'>Hide Incompatibilities</label>";
			
			bosslog << "<li><input type='checkbox' ";
			if (HideDoNotCleanMessages)
				bosslog << "checked='checked' ";
			bosslog << "id='b14' onclick='toggleMessages()' /><label for='b14'>Hide 'Do Not Clean' Messages</label>";

			bosslog << "</ul>" << "<i><span id='hp'>0</span> of " << (counters.recognised+counters.unrecognised) << " plugins hidden. <span id='hm'>0</span> of " << counters.messages << " messages hidden.</i>";
		}

		// Display Global Messages
		if (!contents.globalMessages.empty() || !contents.iniParsingError.empty() || !contents.criticalError.empty() || !contents.updaterErrors.empty() || !contents.regexError.empty()) {

			bosslog << HEADING_OPEN << "General Messages" << HEADING_CLOSE << LIST_OPEN;
			if (!contents.criticalError.empty())		//Print masterlist parsing error.
				bosslog << contents.criticalError;
			else {
				if (!contents.iniParsingError.empty())		//Print ini parsing error.
					bosslog << contents.iniParsingError;
				if (!contents.updaterErrors.empty())
					bosslog << contents.updaterErrors;
				if (!contents.regexError.empty())
					bosslog << contents.regexError;

				bosslog.SetHTMLSpecialEscape(true);
				size_t size = contents.globalMessages.size();
				for (size_t i=0; i<size; i++)
					bosslog << contents.globalMessages[i];  //Print global messages.
				bosslog.SetHTMLSpecialEscape(false);
			}

			bosslog << LIST_CLOSE;
			if (!contents.criticalError.empty()) {  //Exit early.
				bosslog.PrintFooter();
				bosslog.Save(file, true);
				return;
			}
		}

		// Print Summary
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
			<< PARAGRAPH << "Plugins sorted by user rules are counted as recognised plugins."
			<< DIV_CLOSE;

		// Display RuleList Messages
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

		// Display Script Extender Info
		if (!contents.seInfo.empty())
			bosslog << HEADING_OPEN << "Script Extender And Script Extender Plugins" << HEADING_CLOSE << LIST_OPEN
				<< contents.seInfo
				<< LIST_CLOSE;

		// Display Recognised Mods
		if (gl_revert < 1) 
			bosslog << HEADING_OPEN << "Recognised And Re-ordered Plugins" << HEADING_CLOSE << LIST_ID_RECOGNISED_OPEN;
		else if (gl_revert == 1)
			bosslog << HEADING_OPEN << "Restored Load Order (Using modlist.txt)" << HEADING_CLOSE << LIST_ID_RECOGNISED_OPEN;
		else if (gl_revert == 2) 
			bosslog << HEADING_OPEN << "Restored Load Order (Using modlist.old)" << HEADING_CLOSE << LIST_ID_RECOGNISED_OPEN;
		bosslog << contents.recognisedPlugins
			<< LIST_CLOSE;

		// Display Unrecognised Mods
		if (!contents.unrecognisedPlugins.empty())
			bosslog << HEADING_OPEN << "Unrecognised Plugins" << HEADING_CLOSE << DIV_OPEN 
				<< PARAGRAPH << "Reorder these by hand using your favourite mod ordering utility." << LIST_OPEN
				<< contents.unrecognisedPlugins
				<< LIST_CLOSE << DIV_CLOSE;

		// Finish
		bosslog << HEADING_ID_END_OPEN << "Execution Complete" << HEADING_CLOSE;
		bosslog.PrintFooter();
		bosslog.Save(file, true);
	}

	//////////////////////////////////
	// Externally-Visible Functions
	//////////////////////////////////

	summaryCounters::summaryCounters()
		: recognised(0), unrecognised(0), ghosted(0), messages(0), warnings(0), errors(0) {}

	//Record recognised mod list from last HTML BOSSlog generated.
	BOSS_COMMON string GetOldRecognisedList(const fs::path log) {
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

	BOSS_COMMON void PerformSortingFunctionality(fs::path file,
												ItemList& modlist,
												ItemList& masterlist,
												RuleList& userlist,
												const time_t esmtime,
												bosslogContents contents) {
		string SE;
		summaryCounters counters;

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
				throw boss_error(BOSS_ERROR_PLUGIN_BEFORE_MASTER, masterlist.Items()[pos].Name());
		} catch (boss_error &e) {
			contents.globalMessages.push_back(Message(SAY, "The order of plugins set by BOSS differs from their order in its masterlist, as one or more of the installed plugins is false-flagged. For more information, see the readme section on False-Flagged Plugins."));
			masterlist.ApplyMasterPartition();
			LOG_WARN("The order of plugins set by BOSS differs from their order in its masterlist, as one or more of the installed plugins is false-flagged. For more information, see the readme section on False-Flagged Plugins.");
		}

		//Now stick them back together.
		modlist.Insert(0,masterlist.Items(), 0, masterlist.Items().size());
		modlist.LastRecognisedPos(masterlist.LastRecognisedPos());

		ApplyUserRules(modlist, userlist, contents.userlistMessages);
		LOG_INFO("userlist sorting process finished.");

		SE = GetSEPluginInfo(contents.seInfo);

		SortMods(modlist, contents, esmtime, counters);

		//Now set the load order using Skyrim method.
		if (gl_current_game == SKYRIM && Version(GetExeDllVersion(data_path.parent_path() / "TESV.exe")) >= Version("1.4.26.0")) {
			try {
				modlist.SavePluginNames(loadorder_path(), false, false);
				modlist.SavePluginNames(plugins_path(), true, true);
			} catch (boss_error &e) {
				Outputter output(HTML);
				output << LIST_ITEM_CLASS_ERROR << "Critical Error: " << e.getString() << LINE_BREAK
					<< "Check the Troubleshooting section of the ReadMe for more information and possible solutions." << LINE_BREAK
					<< "Utility will end now.";
				contents.criticalError = output.AsString();
			}
		}

		PrintBOSSlog(file, contents, counters, SE);
	}

	//Create a modlist containing all the mods that are installed or referenced in the userlist with their masterlist messages.
	//Returns the vector position of the last recognised mod in modlist.
	BOSS_COMMON void BuildWorkingModlist(ItemList& modlist, ItemList& masterlist, RuleList& userlist) {
		//Add all modlist and userlist mods to a hashset to optimise comparison against masterlist.
		boost::unordered_set<string> hashset;  //Holds mods for checking against masterlist
		boost::unordered_set<string>::iterator setPos;

		vector<Item> items = modlist.Items();
		size_t modlistSize = items.size();
		size_t userlistSize = userlist.rules.size();
		/* Hashset must be a set of unique mods.
		Ghosted mods take priority over non-ghosted mods, as they are specifically what is installed. 
		*/

		LOG_INFO("Populating hashset with modlist.");
		for (size_t i=0; i<modlistSize; i++) {
			if (items[i].Type() == MOD)
				hashset.insert(to_lower_copy(items[i].Name()));
		}
		LOG_INFO("Populating hashset with userlist.");
		//Need to get ruleObject and sort line object if plugins.
		for (size_t i=0; i<userlistSize; i++) {
			if (Item(userlist.rules[i].Object()).IsPlugin()) {
				setPos = hashset.find(to_lower_copy(userlist.rules[i].Object()));
				if (setPos == hashset.end()) {  //Mod not already in hashset.
					setPos = hashset.find(to_lower_copy(userlist.rules[i].Object() + ".ghost"));
					if (setPos == hashset.end()) {  //Ghosted mod not already in hashset. 
						//Unique plugin, so add to hashset.
						hashset.insert(to_lower_copy(userlist.rules[i].Object()));
					}
				}
			}
			if (userlist.rules[i].Key() != FOR) {  //First line is a sort line.
				if (Item(userlist.rules[i].Lines()[0].Object()).IsPlugin()) {
					setPos = hashset.find(to_lower_copy(userlist.rules[i].Lines()[0].Object()));
					if (setPos == hashset.end()) {  //Mod not already in hashset.
						setPos = hashset.find(to_lower_copy(userlist.rules[i].Lines()[0].Object() + ".ghost"));
						if (setPos == hashset.end()) {  //Ghosted mod not already in hashset. 
							//Unique plugin, so add to hashset.
							hashset.insert(to_lower_copy(userlist.rules[i].Lines()[0].Object()));
						}
					}
				}
			}
		}

		//Now compare masterlist against hashset.
		size_t modlistPos;
		items = masterlist.Items();
		size_t max = masterlist.Items().size();
		vector<Item> holdingVec;
		boost::unordered_set<string>::iterator addedPos;
		boost::unordered_set<string> addedMods;
		LOG_INFO("Comparing hashset against masterlist.");
		for (size_t i=0; i < max; i++) {
			if (items[i].Type() == MOD) {
				//Check to see if the mod is in the hashset. If it is, or its ghosted version is, also check if 
				//the mod is already in the holding vector. If not, add it.
				setPos = hashset.find(to_lower_copy(items[i].Name()));
				if (setPos == hashset.end()) {
					items[i].Name(items[i].Name() + ".ghost");		//Add ghost extension to mod name.
					setPos = hashset.find(to_lower_copy(items[i].Name()));
				}
				if (setPos != hashset.end()) {										//Mod found in hashset. 
					addedPos = addedMods.find(to_lower_copy(items[i].Name()));
					if (addedPos == addedMods.end()) {								//The mod is not already in the holding vector.
						holdingVec.push_back(items[i]);									//Record it in the holding vector.
						modlistPos = modlist.FindItem(items[i].Name());
						if (modlistPos != modlist.Items().size())
							modlist.Erase(modlistPos);
						addedMods.insert(to_lower_copy(items[i].Name()));
					}
				}
			} else //Group lines must stay recorded.
				holdingVec.push_back(items[i]);
		}
		masterlist.Items(holdingVec);  //Masterlist now only contains the items needed to sort the user's mods.
		masterlist.LastRecognisedPos(masterlist.Items().size()-1);
	}

	//Applies the userlist rules to the working modlist.
	BOSS_COMMON void ApplyUserRules(ItemList& modlist, RuleList& userlist, string& outputBuffer) {
		if (userlist.rules.empty())
			return;
		//Because erase operations invalidate iterators after the position(s) erased, the last recognised mod needs to be recorded, then
		//set correctly again after all operations have completed.
		//Note that if a mod is sorted after the last recognised mod by the userlist, it becomes the last recognised mod, and the item will
		//need to be re-assigned to this item. This only occurs for BEFORE/AFTER plugin sorting rules.
		string lastRecognisedItem = modlist.Items()[modlist.LastRecognisedPos()].Name();

		Outputter buffer(gl_log_format);

		LOG_INFO("Starting userlist sort process... Total %" PRIuS " user rules statements to process.", userlist.rules.size());
		vector<Rule>::iterator ruleIter = userlist.rules.begin();
		size_t modlistPos1, modlistPos2;
		uint32_t ruleNo = 0;
		for (ruleIter; ruleIter != userlist.rules.end(); ++ruleIter) {
			ruleNo++;
			LOG_DEBUG(" -- Processing rule #%" PRIuS ".", ruleNo);
			if (!ruleIter->Enabled()) {
				buffer << LIST_ITEM_CLASS_SUCCESS << "The rule beginning \"" << ruleIter->KeyToString() << ": " << ruleIter->Object() << "\" is disabled. Rule skipped.";
				LOG_INFO("Rule beginning \"%s: %s\" is disabled. Rule skipped.", ruleIter->KeyToString().c_str(), ruleIter->Object().c_str());
				continue;
			}	
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
							buffer << LIST_ITEM_CLASS_WARN << "\"" << ruleIter->Object() << "\" is not installed or in the masterlist. Rule skipped.";
							LOG_WARN(" * \"%s\" is not installed.", ruleIter->Object().c_str());
							continue;
						//If it adds a mod already sorted, skip the rule.
						} else if (ruleIter->Key() == ADD  && modlistPos1 <= modlist.LastRecognisedPos()) {
							buffer << LIST_ITEM_CLASS_WARN << "\"" << ruleIter->Object() << "\" is already in the masterlist. Rule skipped.";
							LOG_WARN(" * \"%s\" is already in the masterlist.", ruleIter->Object().c_str());
							continue;
						} else if (ruleIter->Key() == OVERRIDE && (modlistPos1 > modlist.LastRecognisedPos())) {
							buffer << LIST_ITEM_CLASS_ERROR << "\"" << ruleIter->Object() << "\" is not in the masterlist, cannot override. Rule skipped.";
							LOG_WARN(" * \"%s\" is not in the masterlist, cannot override.", ruleIter->Object().c_str());
							continue;
						}
						modlistPos2 = modlist.FindItem(lines[i].Object());  //Find sort mod.
						//Do checks.
						if (modlistPos2 == modlist.Items().size()) {  //Handle case of mods that don't exist at all.
							buffer << LIST_ITEM_CLASS_WARN << "\"" << lines[i].Object() << "\" is not installed, and is not in the masterlist. Rule skipped.";
							LOG_WARN(" * \"%s\" is not installed or in the masterlist.", lines[i].Object().c_str());
							continue;
						} else if (modlistPos2 > modlist.LastRecognisedPos()) {  //Handle the case of a rule sorting a mod into a position in unsorted mod territory.
							buffer << LIST_ITEM_CLASS_ERROR << "\"" << lines[i].Object() << "\" is not in the masterlist and has not been sorted by a rule. Rule skipped.";
							LOG_WARN(" * \"%s\" is not in the masterlist and has not been sorted by a rule.", lines[i].Object().c_str());
							continue;
						} else if (lines[i].Key() == AFTER && modlistPos2 == modlist.LastRecognisedPos())
							lastRecognisedItem = modlist.Items()[modlistPos1].Name();
						mod = modlist.Items()[modlistPos1];  //Record the rule mod in a new variable.
						modlist.Erase(modlistPos1);  //Now remove the rule mod from its old position. This breaks all modlist iterators active.
						//Need to find sort mod pos again, to fix iterator.
						modlistPos2 = modlist.FindItem(lines[i].Object());  //Find sort mod.
						//Insert the mod into its new position.
						if (lines[i].Key() == AFTER)
							++modlistPos2;
						modlist.Insert(modlistPos2, mod);
						buffer << LIST_ITEM_CLASS_SUCCESS << "\"" << ruleIter->Object() << "\" has been sorted " << lines[i].KeyToString() << " \"" << lines[i].Object() << "\".";
					} else if (lines[i].Key() == TOP || lines[i].Key() == BOTTOM) {
						Item mod;
						modlistPos1 = modlist.FindItem(ruleItem.Name());
						//Do checks.
						if (ruleIter->Key() == ADD && modlistPos1 == modlist.Items().size()) {
							buffer << LIST_ITEM_CLASS_WARN << "\"" << ruleIter->Object() << "\" is not installed or in the masterlist. Rule skipped.";
							LOG_WARN(" * \"%s\" is not installed.", ruleIter->Object().c_str());
							continue;
						//If it adds a mod already sorted, skip the rule.
						} else if (ruleIter->Key() == ADD  && modlistPos1 <= modlist.LastRecognisedPos()) {
							buffer << LIST_ITEM_CLASS_WARN << "\"" << ruleIter->Object() << "\" is already in the masterlist. Rule skipped.";
							LOG_WARN(" * \"%s\" is already in the masterlist.", ruleIter->Object().c_str());
							continue;
						} else if (ruleIter->Key() == OVERRIDE && (modlistPos1 > modlist.LastRecognisedPos() || modlistPos1 == modlist.Items().size())) {
							buffer << LIST_ITEM_CLASS_ERROR << "\"" << ruleIter->Object() << "\" is not in the masterlist, cannot override. Rule skipped.";
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
							buffer << LIST_ITEM_CLASS_ERROR << "The group \"" << lines[i].Object() << "\" is not in the masterlist or is malformatted. Rule skipped.";
							LOG_WARN(" * \"%s\" is not in the masterlist, or is malformatted.", lines[i].Object().c_str());
							continue;
						}
						mod = modlist.Items()[modlistPos1];  //Record the rule mod in a new variable.
						modlist.Erase(modlistPos1);  //Now remove the rule mod from its old position. This breaks all modlist iterators active.
						//Need to find group pos again, to fix iterators.
						if (lines[i].Key() == TOP)
							modlistPos2 = modlist.FindItem(lines[i].Object()) + 1;  //Find the start, and increment by 1 so that mod is inserted after start.
						else
							modlistPos2 = modlist.FindGroupEnd(lines[i].Object());  //Find the end.
						modlist.Insert(modlistPos2, mod);  //Now insert the mod into the group. This breaks all modlist iterators active.
						//Print success message.
						buffer << LIST_ITEM_CLASS_SUCCESS << "\"" << ruleIter->Object() << "\" inserted at the " << lines[i].KeyToString() << " of group \"" << lines[i].Object() << "\".";
					}
					i++;
				}
				for (i; i < max; i++) {  //Message lines.
					//Find the mod which will have its messages edited.
					modlistPos1 = modlist.FindItem(ruleItem.Name());
					if (modlistPos1 == modlist.Items().size()) {  //Rule mod isn't in the modlist (ie. not in masterlist or installed), so can neither add it nor override it.
						buffer << LIST_ITEM_CLASS_WARN << "\"" << ruleIter->Object() << "\" is not installed or in the masterlist. Rule skipped.";
						LOG_WARN(" * \"%s\" is not installed.", ruleIter->Object().c_str());
						break;
					}
					Message newMessage = Message(lines[i].ObjectMessageKey(), lines[i].ObjectMessageData());
					vector<Message> messages = modlist.Items()[modlistPos1].Messages();
					if (lines[i].Key() == REPLACE)  //If the rule is to replace messages, clear existing messages.
						messages.clear();
					//Append message to message list of mod.
					messages.push_back(newMessage);
					vector<Item> items = modlist.Items();
					items[modlistPos1].Messages(messages);
					modlist.Items(items);
					//Output confirmation.
					buffer << LIST_ITEM_CLASS_SUCCESS << "\"" << SPAN_CLASS_MESSAGE_OPEN << lines[i].Object() << SPAN_CLOSE <<"\"";
					if (lines[i].Key() == APPEND)
						buffer << " appended to ";
					else
						buffer << " replaced ";
					buffer << "messages attached to \"" << ruleIter->Object() << "\".";
				}
			} else if (lines[i].Key() == BEFORE || lines[i].Key() == AFTER) {  //Group: Can only sort.
				vector<Item> group;
				//Look for group to sort. Find start and end positions.
				modlistPos1 = modlist.FindItem(ruleItem.Name());
				modlistPos2 = modlist.FindGroupEnd(ruleItem.Name());
				//Check to see group actually exists.
				if (modlistPos1 == modlist.Items().size() || modlistPos2 == modlist.Items().size()) {
					buffer << LIST_ITEM_CLASS_ERROR << "The group \"" << ruleIter->Object() << "\" is not in the masterlist or is malformatted. Rule skipped.";
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
					buffer << LIST_ITEM_CLASS_ERROR << "The group \"" << lines[i].Object() << "\" is not in the masterlist or is malformatted. Rule skipped.";
					LOG_WARN(" * \"%s\" is not in the masterlist, or is malformatted.", lines[i].Object().c_str());
					continue;
				}
				if (lines[i].Key() == AFTER)
					modlistPos2++;
				//Now insert the group.
				modlist.Insert(modlistPos2, group, 0, group.size());
				//Print success message.
				buffer << LIST_ITEM_CLASS_SUCCESS << "The group \"" << ruleIter->Object() << "\" has been sorted " << lines[i].KeyToString() << " the group \"" << lines[i].Object() << "\".";
			}
			//Now find that last recognised mod and set the iterator again.
			modlist.LastRecognisedPos(modlist.FindLastItem(lastRecognisedItem));
		}


		if (userlist.rules.empty()) 
			buffer << ITALIC_OPEN << "No valid rules were found in your userlist.txt." << ITALIC_CLOSE;
		outputBuffer = buffer.AsString();
	}
}