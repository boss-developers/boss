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

#include "Common/Game.h"
#include "Common/Globals.h"
#include "Support/Helpers.h"
#include "Support/Logger.h"
#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>

#if _WIN32 || _WIN64
#	include <windows.h>
#	include <shlobj.h>
#endif

namespace boss {
	using namespace std;
	namespace loc = boost::locale;

	//DO NOT CHANGE THESE VALUES. THEY MUST BE CONSTANT FOR API USERS.
	BOSS_COMMON const uint32_t LOMETHOD_TIMESTAMP = 0;
	BOSS_COMMON const uint32_t LOMETHOD_TEXTFILE  = 1;

	uint32_t AutodetectGame(vector<uint32_t> detectedGames) {  //Throws exception if error.
		if (gl_last_game != AUTODETECT) {
			for (size_t i = 0, max = detectedGames.size();
			     i < max; i++) {
				if (gl_last_game == detectedGames[i])
					return gl_last_game;
			}
		}
		LOG_INFO("Autodetecting game.");

		if (Game(NEHRIM, "", true).IsInstalledLocally())  //Before Oblivion because Nehrim installs can have Oblivion.esm for porting mods.
			return NEHRIM;
		else if (Game(OBLIVION, "", true).IsInstalledLocally())
			return OBLIVION;
		else if (Game(SKYRIM, "", true).IsInstalledLocally())
			return SKYRIM;
		else if (Game(FALLOUTNV, "", true).IsInstalledLocally())  //Before Fallout 3 because some mods for New Vegas require Fallout3.esm.
			return FALLOUTNV;
		else if (Game(FALLOUT3, "", true).IsInstalledLocally())
			return FALLOUT3;
		else {
			LOG_INFO("No game detected locally. Using Registry paths.");
			return AUTODETECT;
		}
	}

	BOSS_COMMON uint32_t DetectGame(vector<uint32_t>& detectedGames,
	                                vector<uint32_t>& undetectedGames) {
		//Detect all installed games.
		if (Game(OBLIVION, "", true).IsInstalled())  //Look for Oblivion.
			detectedGames.push_back(OBLIVION);
		else
			undetectedGames.push_back(OBLIVION);
		if (Game(NEHRIM, "", true).IsInstalled())  //Look for Nehrim.
			detectedGames.push_back(NEHRIM);
		else
			undetectedGames.push_back(NEHRIM);
		if (Game(SKYRIM, "", true).IsInstalled())  //Look for Skyrim.
			detectedGames.push_back(SKYRIM);
		else
			undetectedGames.push_back(SKYRIM);
		if (Game(FALLOUT3, "", true).IsInstalled())  //Look for Fallout 3.
			detectedGames.push_back(FALLOUT3);
		else
			undetectedGames.push_back(FALLOUT3);
		if (Game(FALLOUTNV, "", true).IsInstalled())  //Look for Fallout New Vegas.
			detectedGames.push_back(FALLOUTNV);
		else
			undetectedGames.push_back(FALLOUTNV);

		//Now set return a game.
		if (gl_game != AUTODETECT) {
			if (gl_update_only)
				return gl_game;
			else if (Game(gl_game, "", true).IsInstalled())
				return gl_game;
			else
				return AutodetectGame(detectedGames);  //Game not found. Autodetect.
		} else
			return AutodetectGame(detectedGames);
	}

	//Structures necessary for case-insensitive hashsets used in BuildWorkingModlist. Taken from the BOOST docs.
	struct iequal_to : std::binary_function<std::string, std::string,
	                                        bool> {
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
		std::size_t operator()(String const& x) const {
			std::size_t seed = 0;

			for (typename String::const_iterator it = x.begin();
			     it != x.end(); ++it) {
				boost::hash_combine(seed, std::toupper(*it, locale_));
			}

			return seed;
		}
	private:
		std::locale locale_;
	};


	////////////////////////////
	// Game Class Functions
	////////////////////////////

	Game::Game() : id(AUTODETECT) {}

	Game::Game(const uint32_t gameCode, const string path,
	           const bool noPathInit) : id(gameCode) {
		// MCP Note: Possibly turn this into a switch-statement?
		if (Id() == OBLIVION) {
			name = "TES IV: Oblivion";

			executable = "Oblivion.exe";
			masterFile = "Oblivion.esm";
			scriptExtender = "OBSE";
			seExecutable = "obse_1_2_416.dll";

			registryKey = "Software\\Bethesda Softworks\\Oblivion";
			registrySubKey = "Installed Path";

			bossFolderName = "Oblivion";
			appdataFolderName = "Oblivion";
			pluginsFolderName = "Data";
			pluginsFileName = "plugins.txt";
		} else if (Id() == NEHRIM) {
			name = "Nehrim - At Fate's Edge";

			executable = "Oblivion.exe";
			masterFile = "Nehrim.esm";
			scriptExtender = "OBSE";
			seExecutable = "obse_1_2_416.dll";

			registryKey = "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Nehrim - At Fate's Edge_is1";
			registrySubKey = "InstallLocation";

			bossFolderName = "Nehrim";
			appdataFolderName = "Oblivion";
			pluginsFolderName = "Data";
			pluginsFileName = "plugins.txt";
		} else if (Id() == SKYRIM) {
			name = "TES V: Skyrim";

			executable = "TESV.exe";
			masterFile = "Skyrim.esm";
			scriptExtender = "SKSE";
			seExecutable = "skse_loader.exe";

			registryKey = "Software\\Bethesda Softworks\\Skyrim";
			registrySubKey = "Installed Path";

			bossFolderName = "Skyrim";
			appdataFolderName = "Skyrim";
			pluginsFolderName = "Data";
			pluginsFileName = "plugins.txt";
		} else if (Id() == FALLOUT3) {
			name = "Fallout 3";

			executable = "Fallout3.exe";
			masterFile = "Fallout3.esm";
			scriptExtender = "FOSE";
			seExecutable = "fose_loader.exe";

			registryKey = "Software\\Bethesda Softworks\\Fallout3";
			registrySubKey = "Installed Path";

			bossFolderName = "Fallout 3";
			appdataFolderName = "Fallout3";
			pluginsFolderName = "Data";
			pluginsFileName = "plugins.txt";
		} else if (Id() == FALLOUTNV) {
			name = "Fallout: New Vegas";

			executable = "FalloutNV.exe";
			masterFile = "FalloutNV.esm";
			scriptExtender = "NVSE";
			seExecutable = "nvse_loader.exe";

			registryKey = "Software\\Bethesda Softworks\\FalloutNV";
			registrySubKey = "Installed Path";

			bossFolderName = "Fallout New Vegas";
			appdataFolderName = "FalloutNV";
			pluginsFolderName = "Data";
			pluginsFileName = "plugins.txt";
		} else
			throw boss_error(BOSS_ERROR_NO_GAME_DETECTED);

		//BOSS Log init.
		bosslog.scriptExtender = ScriptExtender();
		bosslog.gameName = Name();
		bosslog.recognisedPlugins.SetHTMLSpecialEscape(false);
		bosslog.unrecognisedPlugins.SetHTMLSpecialEscape(false);

		if (!noPathInit) {
			if (path.empty()) {
				//First look for local install, then look for Registry.
				if (fs::exists(boss_path / ".." / pluginsFolderName / masterFile))
					gamePath = boss_path / "..";
				else if (RegKeyExists("HKEY_LOCAL_MACHINE",
				                      registryKey, registrySubKey))
					gamePath = fs::path(RegKeyStringValue("HKEY_LOCAL_MACHINE",
					                                      registryKey,
					                                      registrySubKey));
				else if (gl_update_only)  //Update only games are treated as installed locally if not actually installed.
					gamePath = boss_path / "..";
				else
					throw boss_error(BOSS_ERROR_NO_GAME_DETECTED);
			} else
				gamePath = fs::path(path);

			//Check if game master file exists. Requires data path to be set.
			if (!MasterFile().Exists(*this))
				throw boss_error(BOSS_ERROR_FILE_NOT_FOUND,
				                 MasterFile().Name());

			//Requires data path to be set.
			if (Id() == OBLIVION &&
			    fs::exists(GameFolder() / "Oblivion.ini")) {
				//Looking up bUseMyGamesDirectory, which only has effect if =0 and exists in Oblivion folder.
				Settings oblivionIni;
				oblivionIni.Load(GameFolder() / "Oblivion.ini");  //This also sets the variable up.

				if (oblivionIni.GetValue("bUseMyGamesDirectory") == "0") {
					pluginsPath = GameFolder() / pluginsFileName;
					loadorderPath = GameFolder() / "loadorder.txt";
				} else {
					pluginsPath = GetLocalAppDataPath() / appdataFolderName / pluginsFileName;
					loadorderPath = GetLocalAppDataPath() / appdataFolderName / "loadorder.txt";
				}
			} else {
				pluginsPath = GetLocalAppDataPath() / appdataFolderName / pluginsFileName;
				loadorderPath = GetLocalAppDataPath() / appdataFolderName / "loadorder.txt";
			}

			//Load order method init. Requires game path to be set.
			if (Id() == SKYRIM && GetVersion() >= Version("1.4.26.0"))
				loMethod = LOMETHOD_TEXTFILE;
			else
				loMethod = LOMETHOD_TIMESTAMP;
		}
	}

	bool Game::IsInstalled() const {
		return (IsInstalledLocally() ||
		        RegKeyExists("HKEY_LOCAL_MACHINE", registryKey,
		                     registrySubKey));
	}

	bool Game::IsInstalledLocally() const {
		return fs::exists(boss_path / ".." / pluginsFolderName / masterFile);
	}

	uint32_t Game::Id() const {
		return id;
	}

	string Game::Name() const {
		return name;
	}

	string Game::ScriptExtender() const {
		return scriptExtender;
	}

	Item Game::MasterFile() const {
		return Item(masterFile);
	}

	Version Game::GetVersion() const {
		return Version(Executable());
	}

	uint32_t Game::GetLoadOrderMethod() const {
		return loMethod;
	}

	fs::path Game::Executable() const {
		return GameFolder() / executable;
	}

	fs::path Game::GameFolder() const {
		return gamePath;
	}

	fs::path Game::DataFolder() const {
		return GameFolder() / pluginsFolderName;
	}

	fs::path Game::SEPluginsFolder() const {
		return DataFolder() / scriptExtender / "Plugins";
	}

	fs::path Game::SEExecutable() const {
		return GameFolder() / seExecutable;
	}

	fs::path Game::ActivePluginsFile() const {
		return pluginsPath;
	}

	fs::path Game::LoadOrderFile() const {
		return loadorderPath;
	}

	fs::path Game::Masterlist() const {
		return boss_path / bossFolderName / "masterlist.txt";
	}

	fs::path Game::Userlist() const {
		return boss_path / bossFolderName / "userlist.txt";
	}

	fs::path Game::Modlist() const {
		return boss_path / bossFolderName / "modlist.txt";
	}

	fs::path Game::OldModlist() const {
		return boss_path / bossFolderName / "modlist.old";
	}

	fs::path Game::Log(uint32_t format) const {
		if (format == HTML)
			return boss_path / bossFolderName / "BOSSlog.html";
		else
			return boss_path / bossFolderName / "BOSSlog.txt";
	}

	//Can be used to get the location of the LOCALAPPDATA folder (and its Windows XP equivalent).
	fs::path Game::GetLocalAppDataPath() {
#if _WIN32 || _WIN64
		HWND owner = NULL;
		TCHAR path[MAX_PATH];

		HRESULT res = SHGetFolderPath(owner, CSIDL_LOCAL_APPDATA, NULL,
		                              SHGFP_TYPE_CURRENT, path);

		if (res == S_OK)
			return fs::path(path);
		else
			return fs::path("");
#else
		return fs::path("");
#endif
	}

	void Game::CreateBOSSGameFolder() {
		//Make sure that the BOSS game path exists.
		try {
			if (!fs::exists(boss_path / bossFolderName))
				fs::create_directory(boss_path / bossFolderName);
		} catch (fs::filesystem_error e) {
			throw boss_error(BOSS_ERROR_FS_CREATE_DIRECTORY_FAIL,
			                 fs::path(boss_path / bossFolderName).string(),
			                 e.what());
		}
	}

	void Game::ApplyMasterlist() {
		//Add all modlist and userlist mods and groups referenced in userlist to a hashset to optimise comparison against masterlist.
		boost::unordered_set<string, ihash, iequal_to> mHashset, uHashset, addedItems;  //Holds mods and groups for checking against masterlist.
		boost::unordered_set<string>::iterator setPos;

		LOG_INFO("Populating hashset with modlist.");
		vector<Item> items = modlist.Items();
		size_t modlistSize = items.size();
		for (size_t i = 0; i < modlistSize; i++) {
			if (items[i].Type() == MOD)
				mHashset.insert(items[i].Name());
		}

		LOG_INFO("Populating hashset with userlist.");
		vector<Rule> rules = userlist.Rules();
		size_t userlistSize = rules.size();
		for (size_t i = 0; i < userlistSize; i++) {
			Item ruleObject(rules[i].Object());
			if (uHashset.find(ruleObject.Name()) == uHashset.end())  //Mod or group not already in hashset, so add to hashset.
				uHashset.insert(ruleObject.Name());
			if (rules[i].Key() != FOR) {  //First line is a sort line.
				Item sortObject(rules[i].LineAt(1).Object());
				if (uHashset.find(sortObject.Name()) == uHashset.end())  //Mod or group not already in hashset, so add to hashset.
					uHashset.insert(sortObject.Name());
			}
		}

		LOG_INFO("Comparing hashset against masterlist.");
		size_t addedNum = 0;
		items = masterlist.Items();
		for (size_t i = 0, max = items.size(); i < max; i++) {
			if (items[i].Type() == MOD) {
				//Check to see if the mod is in the hashset. If it is, also check if
				//the mod is already in the holding vector. If not, add it.
				setPos = mHashset.find(items[i].Name());
				if (setPos != mHashset.end())  //Mod is installed. Ensure that correct case is recorded.
					items[i].Name(*setPos);
				else if (uHashset.find(items[i].Name()) == uHashset.end())  //Mod not in modlist or userlist, skip.
					continue;

				if (addedItems.find(items[i].Name()) == addedItems.end()) {  //The mod is not already in the holding vector.
					addedItems.insert(items[i].Name());                      //Record it in the holding vector.
					modlist.Move(addedNum, items[i]);
					addedNum++;
				}
			} else if (items[i].Type() == BEGINGROUP ||
			           items[i].Type() == ENDGROUP) {
				if (uHashset.find(items[i].Name()) == uHashset.end())  //Mod not in modlist or userlist, skip.
					continue;

				//Don't check if it has already been added, as groups will have two entries anyway.
				//Also don't Move it, for the same reason.
				modlist.Insert(addedNum, items[i]);
				addedNum++;
			}
		}
		if (addedNum > 0)
			modlist.LastRecognisedPos(addedNum - 1);
		//Modlist now contains the files in Data folder in load order, interspersed by other files that are not installed but referenced by the userlist, and groups.
		//This is followed by the unrecognised plugins in their load order, and the last recognised position has been recorded.
	}

	void Game::ApplyUserlist() {
		vector<Rule> rules = userlist.Rules();
		if (rules.empty())
			return;
		//Because erase operations invalidate iterators after the position(s) erased, the last recognised mod needs to be recorded, then
		//set correctly again after all operations have completed.
		//Note that if a mod is sorted after the last recognised mod by the userlist, it becomes the last recognised mod, and the item will
		//need to be re-assigned to this item. This only occurs for BEFORE/AFTER plugin sorting rules.
		Item lastRecognisedItem = modlist.ItemAt(modlist.LastRecognisedPos());

		LOG_INFO("Starting userlist sort process... Total %" PRIuS " user rules statements to process.",
		         rules.size());
		vector<Rule>::iterator ruleIter = rules.begin();
		size_t modlistPos1, modlistPos2;
		uint32_t ruleNo = 0;
		for (ruleIter; ruleIter != rules.end(); ++ruleIter) {
			ruleNo++;
			LOG_DEBUG(" -- Processing rule #%" PRIuS ".", ruleNo);
			if (!ruleIter->Enabled()) {
				bosslog.userRules << TABLE_ROW_CLASS_WARN << TABLE_DATA << *ruleIter << TABLE_DATA << "✗" << TABLE_DATA << loc::translate("Rule is disabled.");
				LOG_INFO("Rule beginning \"%s: %s\" is disabled. Rule skipped.",
				         ruleIter->KeyToString().c_str(),
				         ruleIter->Object().c_str());
				continue;
			}
			bool messageLineFail = false;
			size_t i = 0;
			vector<RuleLine> lines = ruleIter->Lines();
			size_t max = lines.size();
			Item ruleItem(ruleIter->Object());
			if (ruleItem.IsPlugin()) {  //Plugin: Can sort or add messages.
				if (ruleIter->Key() != FOR) {  //First non-rule line is a sort line.
					if (lines[i].Key() == BEFORE ||
					    lines[i].Key() == AFTER) {
						Item mod;
						modlistPos1 = modlist.FindItem(ruleItem.Name(),
						                               MOD);
						//Do checks.
						if (ruleIter->Key() == ADD &&
						    modlistPos1 == modlist.Items().size()) {
							bosslog.userRules << TABLE_ROW_CLASS_WARN << TABLE_DATA << *ruleIter << TABLE_DATA << "✗" << TABLE_DATA << VAR_OPEN << ruleIter->Object() << VAR_CLOSE << loc::translate(" is not installed or in the masterlist.");
							LOG_WARN(" * \"%s\" is not in the masterlist or installed.",
							         ruleIter->Object().c_str());
							continue;
						} else if (ruleIter->Key() == ADD  &&
						           modlistPos1 <= modlist.LastRecognisedPos()) {  //If it adds a mod already sorted, skip the rule.
							bosslog.userRules << TABLE_ROW_CLASS_WARN << TABLE_DATA << *ruleIter << TABLE_DATA << "✗" << TABLE_DATA << VAR_OPEN << ruleIter->Object() << VAR_CLOSE << loc::translate(" is already in the masterlist.");
							LOG_WARN(" * \"%s\" is already in the masterlist.", ruleIter->Object().c_str());
							continue;
						} else if (ruleIter->Key() == OVERRIDE &&
						          (modlistPos1 > modlist.LastRecognisedPos())) {
							bosslog.userRules << TABLE_ROW_CLASS_ERROR << TABLE_DATA << *ruleIter << TABLE_DATA << "✗" << TABLE_DATA << VAR_OPEN << ruleIter->Object() << VAR_CLOSE << loc::translate(" is not in the masterlist, cannot override.");
							LOG_WARN(" * \"%s\" is not in the masterlist, cannot override.",
							         ruleIter->Object().c_str());
							continue;
						} else if (modlistPos1 == modlist.LastRecognisedPos())  //The last recognised item is being moved. It can only be moved earlier, so set the previous item as the last recognised item.
							lastRecognisedItem = modlist.ItemAt(modlistPos1 - 1);
						modlistPos2 = modlist.FindItem(lines[i].Object(),
						                               MOD);  //Find sort mod.
						//Do checks.
						if (modlistPos2 == modlist.Items().size()) {  //Handle case of mods that don't exist at all.
							bosslog.userRules << TABLE_ROW_CLASS_WARN << TABLE_DATA << *ruleIter << TABLE_DATA << "✗" << TABLE_DATA << VAR_OPEN << lines[i].Object() << VAR_CLOSE << loc::translate(" is not installed, and is not in the masterlist.");
							LOG_WARN(" * \"%s\" is not installed or in the masterlist.",
							         lines[i].Object().c_str());
							continue;
						} else if (modlistPos2 > modlist.LastRecognisedPos()) {  //Handle the case of a rule sorting a mod into a position in unsorted mod territory.
							bosslog.userRules << TABLE_ROW_CLASS_ERROR << TABLE_DATA << *ruleIter << TABLE_DATA << "✗" << TABLE_DATA << VAR_OPEN << lines[i].Object() << VAR_CLOSE << loc::translate(" is not in the masterlist and has not been sorted by a rule.");
							LOG_WARN(" * \"%s\" is not in the masterlist and has not been sorted by a rule.",
							         lines[i].Object().c_str());
							continue;
						} else if (lines[i].Key() == AFTER &&
						           modlistPos2 == modlist.LastRecognisedPos())
							lastRecognisedItem = modlist.ItemAt(modlistPos1);
						mod = modlist.ItemAt(modlistPos1);  //Record the rule mod in a new variable.
						modlist.Erase(modlistPos1);  //Now remove the rule mod from its old position. This breaks all modlist iterators active.
						//Need to find sort mod pos again, to fix iterator.
						modlistPos2 = modlist.FindItem(lines[i].Object(),
						                               MOD);  //Find sort mod.
						//Insert the mod into its new position.
						if (lines[i].Key() == AFTER)
							++modlistPos2;
						modlist.Insert(modlistPos2, mod);
					} else if (lines[i].Key() == TOP ||
					           lines[i].Key() == BOTTOM) {
						Item mod;
						modlistPos1 = modlist.FindItem(ruleItem.Name(),
						                               MOD);
						//Do checks.
						if (ruleIter->Key() == ADD &&
						    modlistPos1 == modlist.Items().size()) {
							bosslog.userRules << TABLE_ROW_CLASS_WARN << TABLE_DATA << *ruleIter << TABLE_DATA << "✗" << TABLE_DATA << VAR_OPEN << ruleIter->Object() << VAR_CLOSE << loc::translate(" is not installed or in the masterlist.");
							LOG_WARN(" * \"%s\" is not installed.",
							         ruleIter->Object().c_str());
							continue;
						//If it adds a mod already sorted, skip the rule.
						} else if (ruleIter->Key() == ADD  &&
						           modlistPos1 <= modlist.LastRecognisedPos()) {
							bosslog.userRules << TABLE_ROW_CLASS_WARN << TABLE_DATA << *ruleIter << TABLE_DATA << "✗" << TABLE_DATA << VAR_OPEN << ruleIter->Object() << VAR_CLOSE << loc::translate(" is already in the masterlist.");
							LOG_WARN(" * \"%s\" is already in the masterlist.",
							         ruleIter->Object().c_str());
							continue;
						} else if (ruleIter->Key() == OVERRIDE &&
						          (modlistPos1 > modlist.LastRecognisedPos() ||
						           modlistPos1 == modlist.Items().size())) {
							bosslog.userRules << TABLE_ROW_CLASS_ERROR << TABLE_DATA << *ruleIter << TABLE_DATA << "✗" << TABLE_DATA << VAR_OPEN << ruleIter->Object() << VAR_CLOSE << loc::translate(" is not in the masterlist, cannot override.");
							LOG_WARN(" * \"%s\" is not in the masterlist, cannot override.",
							         ruleIter->Object().c_str());
							continue;
						}
						//Find the group to sort relative to.
						if (lines[i].Key() == TOP)
							modlistPos2 = modlist.FindItem(lines[i].Object(), BEGINGROUP) + 1;  //Find the start, and increment by 1 so that mod is inserted after start.
						else
							modlistPos2 = modlist.FindLastItem(lines[i].Object(),
							                                   ENDGROUP);  //Find the end.
						//Check that the sort group actually exists.
						if (modlistPos2 == modlist.Items().size()) {
							bosslog.userRules << TABLE_ROW_CLASS_ERROR << TABLE_DATA << *ruleIter << TABLE_DATA << "✗" << TABLE_DATA << loc::translate("The group ") << VAR_OPEN << lines[i].Object() << VAR_CLOSE << loc::translate(" is not in the masterlist or is malformatted.");
							LOG_WARN(" * \"%s\" is not in the masterlist, or is malformatted.",
							         lines[i].Object().c_str());
							continue;
						}
						mod = modlist.ItemAt(modlistPos1);  //Record the rule mod in a new variable.
						modlist.Erase(modlistPos1);  //Now remove the rule mod from its old position. This breaks all modlist iterators active.
						//Need to find group pos again, to fix iterators.
						if (lines[i].Key() == TOP)
							modlistPos2 = modlist.FindItem(lines[i].Object(), BEGINGROUP) + 1;  //Find the start, and increment by 1 so that mod is inserted after start.
						else
							modlistPos2 = modlist.FindLastItem(lines[i].Object(),
							                                   ENDGROUP);  //Find the end.
						modlist.Insert(modlistPos2, mod);  //Now insert the mod into the group. This breaks all modlist iterators active.
					}
					i++;
				}
				for (i; i < max; i++) {  //Message lines.
					//Find the mod which will have its messages edited.
					modlistPos1 = modlist.FindItem(ruleItem.Name(),
					                               MOD);
					if (modlistPos1 == modlist.Items().size()) {  //Rule mod isn't in the modlist (ie. not in masterlist or installed), so can neither add it nor override it.
						bosslog.userRules << TABLE_ROW_CLASS_WARN << TABLE_DATA << *ruleIter << TABLE_DATA << "✗" << TABLE_DATA << VAR_OPEN << ruleIter->Object() << VAR_CLOSE << loc::translate(" is not installed or in the masterlist.");
						LOG_WARN(" * \"%s\" is not installed.",
						         ruleIter->Object().c_str());
						messageLineFail = true;
						break;
					}
					vector<Item> items = modlist.Items();
					if (lines[i].Key() == REPLACE)  //If the rule is to replace messages, clear existing messages.
						items[modlistPos1].ClearMessages();
					//Append message to message list of mod.
					items[modlistPos1].InsertMessage(items[modlistPos1].Messages().size(),
					                                 lines[i].ObjectAsMessage());
					modlist.Items(items);
				}
			} else if (lines[i].Key() == BEFORE ||
			           lines[i].Key() == AFTER) {  //Group: Can only sort.
				vector<Item> group;
				//Look for group to sort. Find start and end positions.
				modlistPos1 = modlist.FindItem(ruleItem.Name(),
				                               BEGINGROUP);
				modlistPos2 = modlist.FindLastItem(ruleItem.Name(),
				                                   ENDGROUP);
				//Check to see group actually exists.
				if (modlistPos1 == modlist.Items().size() ||
				    modlistPos2 == modlist.Items().size()) {
					bosslog.userRules << TABLE_ROW_CLASS_ERROR << TABLE_DATA << *ruleIter << TABLE_DATA << "✗" << TABLE_DATA << loc::translate("The group ") << VAR_OPEN << ruleIter->Object() << VAR_CLOSE << loc::translate(" is not in the masterlist or is malformatted.");
					LOG_WARN(" * \"%s\" is not in the masterlist, or is malformatted.",
					         ruleIter->Object().c_str());
					continue;
				} else if (ruleItem.Name() == lastRecognisedItem.Name())  //Last recognised item is being moved. Set the item before the start of this group to the lastRecognisedItem.
					lastRecognisedItem = modlist.ItemAt(modlistPos1 - 1);
				//Copy the start, end and everything in between to a new variable.
				vector<Item> items = modlist.Items();
				group.assign(items.begin() + modlistPos1,
				             items.begin() + modlistPos2 + 1);
				//Now erase group from modlist.
				modlist.Erase(modlistPos1, modlistPos2 + 1);
				//Find the group to sort relative to and insert it before or after it as appropriate.
				if (lines[i].Key() == BEFORE)
					modlistPos2 = modlist.FindItem(lines[i].Object(),
					                               BEGINGROUP);  //Find the start.
				else
					modlistPos2 = modlist.FindLastItem(lines[i].Object(),
					                                   ENDGROUP);  //Find the end, and add one, as inserting works before the given element.
				//Check that the sort group actually exists.
				if (modlistPos2 == modlist.Items().size()) {
					modlist.Insert(modlistPos1, group, 0, group.size());  //Insert the group back in its old position.
					bosslog.userRules << TABLE_ROW_CLASS_ERROR << TABLE_DATA << *ruleIter << TABLE_DATA << "✗" << TABLE_DATA << loc::translate("The group ") << VAR_OPEN << lines[i].Object() << VAR_CLOSE << loc::translate(" is not in the masterlist or is malformatted.");
					LOG_WARN(" * \"%s\" is not in the masterlist, or is malformatted.",
					         lines[i].Object().c_str());
					continue;
				} else if (lines[i].Key() == AFTER &&
				           lines[i].Object() == lastRecognisedItem.Name())  //Sorting after the last recognised item (which is a group). Set the item to be sorted as the last recognised.
					lastRecognisedItem = Item(ruleItem.Name(),
					                          ENDGROUP);

				if (lines[i].Key() == AFTER)
					modlistPos2++;
				//Now insert the group.
				modlist.Insert(modlistPos2, group, 0, group.size());
			}
			if (!messageLineFail) {  //Print success message.
				LOG_DEBUG("Rule #%" PRIuS " applied successfully.",
				          ruleNo);
				bosslog.userRules << TABLE_ROW_CLASS_SUCCESS << TABLE_DATA << *ruleIter << TABLE_DATA << "✓" << TABLE_DATA;
			}

			//Now find that last recognised mod and set the iterator again.
			modlist.LastRecognisedPos(modlist.FindLastItem(lastRecognisedItem.Name(), lastRecognisedItem.Type()));
		}

		//Now that all the rules have been applied, there is no need for groups or plugins that are not installed to be listed in
		//modlist. Scan through it and remove these lines.
		LOG_INFO("Removing unnecessary items...");
		vector<Item> items = modlist.Items();
		vector<Item>::iterator it = items.begin();
		size_t lastRecPos = modlist.LastRecognisedPos();
		while (it != items.end()) {
			if (it->Type() != MOD || !it->Exists(*this)) {
				if ((size_t)abs(std::distance(items.begin(), it)) <= lastRecPos)
					lastRecPos--;
				it = items.erase(it);
			} else
				++it;
		}
		modlist.Items(items);
		modlist.LastRecognisedPos(lastRecPos);
	}

	//Scans the data folder for script extender plugins and outputs their info to the bosslog.
	void Game::ScanSEPlugins() {
		LOG_INFO("Looking for Script Extender...");
		if (!fs::exists(SEExecutable()))
			LOG_DEBUG("Script Extender not detected.");
		else {
			string CRC = IntToHexString(GetCrc32(SEExecutable()));
			string ver = Version(SEExecutable()).AsString();

			bosslog.sePlugins << LIST_ITEM << SPAN_CLASS_MOD_OPEN << ScriptExtender() << SPAN_CLOSE;
			if (!ver.empty())
				bosslog.sePlugins << SPAN_CLASS_VERSION_OPEN << loc::translate("Version: ") << ver << SPAN_CLOSE;
			if (gl_show_CRCs)
				bosslog.sePlugins << SPAN_CLASS_CRC_OPEN << loc::translate("Checksum: ") << CRC << SPAN_CLOSE;

			LOG_INFO("Looking for Script Extender plugins...");
			if (!fs::is_directory(SEPluginsFolder())) {
				LOG_DEBUG("Script extender plugins directory not detected.");
			} else {
				for (fs::directory_iterator itr(SEPluginsFolder());
				     itr != fs::directory_iterator(); ++itr) {
					const string ext = itr->path().extension().string();
					if (fs::is_regular_file(itr->status()) &&
					    boost::iequals(ext, ".dll")) {
						string CRC = IntToHexString(GetCrc32(itr->path()));
						string ver = Version(itr->path()).AsString();

						bosslog.sePlugins << LIST_ITEM << SPAN_CLASS_MOD_OPEN << itr->path().filename().string() << SPAN_CLOSE;
						if (!ver.empty())
							bosslog.sePlugins << SPAN_CLASS_VERSION_OPEN << loc::translate("Version: ") << ver << SPAN_CLOSE;
						if (gl_show_CRCs)
							bosslog.sePlugins << SPAN_CLASS_CRC_OPEN << loc::translate("Checksum: ") << CRC << SPAN_CLOSE;
					}
				}
			}
		}
	}

	//Sorts the plugins in the data folder, changing timestamps or plugins.txt/loadorder.txt as required.
	void Game::SortPlugins() {
		//Get the master esm time.
		time_t esmtime = MasterFile().GetModTime(*this);

		LOG_INFO("Filling hashset of unrecognised and active plugins...");
		//Load active plugin list.
		boost::unordered_set<string> hashset;
		if (fs::exists(ActivePluginsFile())) {
			LOG_INFO("Loading plugins.txt into ItemList.");
			ItemList pluginsList;
			pluginsList.Load(*this, ActivePluginsFile());
			vector<Item> pluginsEntries = pluginsList.Items();
			size_t pluginsMax = pluginsEntries.size();
			LOG_INFO("Populating hashset with ItemList contents.");
			for (size_t i = 0; i < pluginsMax; i++) {
				if (pluginsEntries[i].Type() == MOD)
					hashset.insert(boost::to_lower_copy(pluginsEntries[i].Name()));
			}
			if (Id() == SKYRIM) {  //Update.esm and Skyrim.esm are always active.
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
		for (size_t i = modlist.LastRecognisedPos() + 1; i < max; i++)
			unrecognised.insert(items[i].Name());

		LOG_INFO("Enforcing masters before plugins rule...");
		//Now check that the recognised plugins in their masterlist order obey the "masters before plugins" rule, and if not post a BOSS Log message saying that the
		//masterlist order has been altered to reflect the "masters before plugins" rule. Then apply the rule. This retains recognised before unrecognised, with the
		//exception of unrecognised masters, which get put after recognised masters.
		try {
			size_t size = modlist.Items().size();
			size_t pos = modlist.GetNextMasterPos(*this, modlist.GetLastMasterPos(*this) + 1);
			modlist.ApplyMasterPartition(*this);
			if (pos <= modlist.LastRecognisedPos())  //Masters exist after the initial set of masters in the recognised load order. Not allowed by game.
				throw boss_error(BOSS_ERROR_PLUGIN_BEFORE_MASTER,
				                 modlist.ItemAt(pos).Name());
		} catch (boss_error /*&e*/) {
			try {
				bosslog.globalMessages.push_back(Message(SAY, loc::translate("The order of plugins set by BOSS differs from their order in its masterlist, as one or more of the installed plugins is false-flagged. For more information, see \"file:../Docs/BOSS%20Readme.html#appendix-c False-Flagged Plugins\".")));
				LOG_WARN("The order of plugins set by BOSS differs from their order in its masterlist, as one or more of the installed plugins is false-flagged. For more information, see the readme section on False-Flagged Plugins.");
			} catch (boss_error &ee) {
				bosslog.globalMessages.push_back(Message(ERR, loc::translate("Could not enforce load order master/plugin partition. Details: ").str() + ee.getString()));
				LOG_ERROR("Error: %s", ee.getString().c_str());
			}
		}

		//Now loop through items, redating and outputting. Check against unrecognised hashset and treat unrecognised mods appropriately.
		//Only act on mods that exist. However, all items that aren't installed mods are removed after applying user rules (even if there were no rules), so nothing needs to be checked.
		time_t modfiletime = 0;
		items = modlist.Items();
		boost::unordered_set<string>::iterator setPos;
		bosslog.recognisedPlugins.SetHTMLSpecialEscape(false);
		bosslog.unrecognisedPlugins.SetHTMLSpecialEscape(false);

		LOG_INFO("Applying calculated ordering to user files...");
		// MCP Note: Look at replacing this with a for-each loop?
		for (vector<Item>::iterator itemIter = items.begin();
		     itemIter != items.end(); ++itemIter) {
			Outputter buffer(gl_log_format);
			buffer << LIST_ITEM << SPAN_CLASS_MOD_OPEN << itemIter->Name() << SPAN_CLOSE;
			string version = itemIter->GetVersion(*this).AsString();
			if (!version.empty())
				buffer << SPAN_CLASS_VERSION_OPEN << loc::translate("Version ") << version << SPAN_CLOSE;
			if (hashset.find(boost::to_lower_copy(itemIter->Name())) != hashset.end())  //Plugin is active.
				buffer << SPAN_CLASS_ACTIVE_OPEN << loc::translate("Active") << SPAN_CLOSE;
			else
				bosslog.inactive++;
			if (gl_show_CRCs && itemIter->IsGhosted(*this)) {
				buffer << SPAN_CLASS_CRC_OPEN << loc::translate("Checksum: ") << IntToHexString(GetCrc32(DataFolder() / fs::path(itemIter->Name() + ".ghost"))) << SPAN_CLOSE;
			} else if (gl_show_CRCs)
				buffer << SPAN_CLASS_CRC_OPEN << loc::translate("Checksum: ") << IntToHexString(GetCrc32(DataFolder() / itemIter->Name())) << SPAN_CLOSE;

			/*if (itemIter->IsFalseFlagged()) {
				itemIter->InsertMessage(0, Message(WARN, "This plugin's internal master bit flag value does not match its file extension. This issue should be reported to the mod's author, and can be fixed by changing the file extension from .esp to .esm or vice versa."));
				counters.warnings++;
			}*/
			if (GetLoadOrderMethod() == LOMETHOD_TIMESTAMP &&
			    !gl_trial_run && !itemIter->IsGameMasterFile(*this)) {
				// time_t is an integer number of seconds, so adding 60 on increases it by a minute.
				// Using recModNo instead of i to avoid increases for group entries.
				LOG_DEBUG(" -- Setting last modified time for file: \"%s\"",
				          itemIter->Name().c_str());
				try {
					itemIter->SetModTime(*this, esmtime + (bosslog.recognised + bosslog.unrecognised) * 60);
				} catch(boss_error &e) {
					itemIter->InsertMessage(0, Message(ERR, loc::translate("Error: ").str() + e.getString()));
					LOG_ERROR(" * Error: %s", e.getString().c_str());
				}
			}
			//Print the mod's messages. Unrecognised plugins might have a redate error message.
			if (!itemIter->Messages().empty()) {
				vector<Message> messages = itemIter->Messages();
				size_t jmax = messages.size();
				buffer << LIST_OPEN;
				for (size_t j = 0; j < jmax; j++) {
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
		LOG_INFO("User plugin ordering applied successfully.");

		//Now set the load order using Skyrim method.
		if (GetLoadOrderMethod() == LOMETHOD_TEXTFILE) {
			try {
				modlist.SavePluginNames(*this, LoadOrderFile(), false,
				                        false);
				modlist.SavePluginNames(*this, ActivePluginsFile(),
				                        true, true);
			} catch (boss_error &e) {
				bosslog.criticalError << LIST_ITEM_CLASS_ERROR << loc::translate("Critical Error: ") << e.getString() << LINE_BREAK
					<< loc::translate("Check the Troubleshooting section of the ReadMe for more information and possible solutions.") << LINE_BREAK
					<< loc::translate("Utility will end now.");
			}
		}
	}
}