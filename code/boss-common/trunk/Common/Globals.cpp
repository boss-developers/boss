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
#define wxUSE_CHOICEDLG 1

#include "Common/Globals.h"
#include "Parsing/Grammar.h"
#include "Support/Helpers.h"
#include "Support/Logger.h"
#include "Output/Output.h"

#ifdef BOSSGUI
#include <wx/choicdlg.h>
#include <wx/arrstr.h>
#endif

namespace boss {
	using namespace std;

	BOSS_COMMON const string gl_boss_release_date	= "27 March 2012";

	BOSS_COMMON uint32_t gl_current_game			= AUTODETECT;

	BOSS_COMMON extern bool gl_using_local_app_data_folder = true;  //Set by bUseMyGamesDirectory in the game's ini file if different from default.

	///////////////////////////////
	//File/Folder Paths
	///////////////////////////////

	BOSS_COMMON const fs::path boss_path			= fs::path(".");
	BOSS_COMMON fs::path data_path					= boss_path / ".." / "Data";
	BOSS_COMMON const fs::path ini_path				= boss_path / "BOSS.ini";
	BOSS_COMMON const fs::path old_ini_path			= boss_path / "BOSS.ini.old";
	BOSS_COMMON const fs::path debug_log_path		= boss_path / "BOSSDebugLog.txt";
	BOSS_COMMON const fs::path readme_path			= boss_path / "Docs" / "BOSS ReadMe.html";
	BOSS_COMMON const fs::path rules_readme_path	= boss_path / "Docs" / "BOSS User Rules ReadMe.html";
	BOSS_COMMON const fs::path masterlist_doc_path	= boss_path / "Docs" / "BOSS Masterlist Syntax.html";
	BOSS_COMMON const fs::path api_doc_path			= boss_path / "Docs" / "BOSS API ReadMe.html";
	BOSS_COMMON const fs::path licenses_path		= boss_path / "Docs" / "Licenses.txt";

	
	///////////////////////////////
	//File/Folder Path Functions
	///////////////////////////////

	//Path to the BOSS files for the game that BOSS is currently running for, relative to executable (ie. boss_path).
	BOSS_COMMON fs::path boss_game_path() {
		switch (gl_current_game) {
		case OBLIVION:
			return boss_path / "Oblivion";
		case NEHRIM:
			return boss_path / "Nehrim";
		case SKYRIM:
			return boss_path / "Skyrim";
		case FALLOUT3:
			return boss_path / "Fallout 3";
		case FALLOUTNV:
			return boss_path / "Fallout New Vegas";
		default:
			return boss_path;
		}
	}

	BOSS_COMMON fs::path bosslog_path() {
		switch (gl_log_format) {
		case HTML:
			return boss_game_path() / "BOSSlog.html";
		case PLAINTEXT:
			return boss_game_path() / "BOSSlog.txt";
		default:
			return boss_game_path() / "BOSSlog.html";
		}
	}

	BOSS_COMMON fs::path masterlist_path() {
		return boss_game_path() / "masterlist.txt";
	}

	BOSS_COMMON fs::path userlist_path() {
		return boss_game_path() / "userlist.txt";
	}

	BOSS_COMMON fs::path modlist_path() {
		return boss_game_path() / "modlist.txt";
	}

	BOSS_COMMON fs::path old_modlist_path() {
		return boss_game_path() / "modlist.old";
	}

	BOSS_COMMON fs::path plugins_path() {
		if (gl_using_local_app_data_folder || gl_current_game != OBLIVION) {  //The bUseMyGamesDirectory has no effect in games other than Oblivion.
			switch (gl_current_game) {
			case OBLIVION:
				return GetLocalAppDataPath() / "Oblivion" / "plugins.txt";
			case NEHRIM:
				return GetLocalAppDataPath() / "Oblivion" / "plugins.txt";  //Shared with Oblivion.
			case SKYRIM:
				return GetLocalAppDataPath() / "Skyrim" / "plugins.txt";
			case FALLOUT3:
				return GetLocalAppDataPath() / "Fallout3" / "plugins.txt";
			case FALLOUTNV:
				return GetLocalAppDataPath() / "FalloutNV" / "plugins.txt";
			default:
				return boss_path;
			}
		} else
			return data_path.parent_path() / "plugins.txt";
	}

	//This only makes sense for Skyrim.
	BOSS_COMMON fs::path loadorder_path() {
		if (gl_using_local_app_data_folder || gl_current_game != OBLIVION) {  //The bUseMyGamesDirectory has no effect in games other than Oblivion.
			switch (gl_current_game) {
			case OBLIVION:
				return GetLocalAppDataPath() / "Oblivion" / "loadorder.txt";
			case NEHRIM:
				return GetLocalAppDataPath() / "Oblivion" / "loadorder.txt";  //Shared with Oblivion.
			case SKYRIM:
				return GetLocalAppDataPath() / "Skyrim" / "loadorder.txt";
			case FALLOUT3:
				return GetLocalAppDataPath() / "Fallout3" / "loadorder.txt";
			case FALLOUTNV:
				return GetLocalAppDataPath() / "FalloutNV" / "loadorder.txt";
			default:
				return boss_path;
			}
		} else
			return data_path.parent_path() / "loadorder.txt";
	}

	///////////////////////////////
	//Ini Settings
	///////////////////////////////

	//GUI variables
	BOSS_COMMON uint32_t	gl_run_type					= 0;
	BOSS_COMMON	bool		gl_use_user_rules_editor	= true;

	//Command line variables
	BOSS_COMMON string		gl_proxy_host				= "";
	BOSS_COMMON string		gl_proxy_user				= "";
	BOSS_COMMON string		gl_proxy_passwd				= "";
	BOSS_COMMON uint32_t	gl_proxy_port				= 0;
	BOSS_COMMON uint32_t	gl_log_format				= HTML;
	BOSS_COMMON uint32_t	gl_game						= AUTODETECT;
	BOSS_COMMON uint32_t	gl_revert					= 0;
	BOSS_COMMON uint32_t	gl_debug_verbosity			= 0;
	BOSS_COMMON bool		gl_update					= true;
	BOSS_COMMON bool		gl_update_only				= false;
	BOSS_COMMON bool		gl_silent					= false;
	BOSS_COMMON bool		gl_skip_version_parse		= false;
	BOSS_COMMON bool		gl_debug_with_source		= false;
	BOSS_COMMON bool		gl_show_CRCs				= false;
	BOSS_COMMON bool		gl_trial_run				= false;
	BOSS_COMMON bool		gl_log_debug_output			= false;
	BOSS_COMMON bool		gl_do_startup_update_check	= true;


	///////////////////////////////
	//Settings Functions
	///////////////////////////////

	BOSS_COMMON string GetGameString(uint32_t game) {
		switch (game) {
		case OBLIVION:
			return "TES IV: Oblivion";
		case NEHRIM:
			return "Nehrim - At Fate's Edge";
		case SKYRIM:
			return "TES V: Skyrim";
		case FALLOUT3:
			return "Fallout 3";
		case FALLOUTNV:
			return "Fallout: New Vegas";
		default:
			return "No Game Detected";
		}
	}

	BOSS_COMMON string	GetGameMasterFile(uint32_t game) {
		switch (game) {
		case OBLIVION:
			return "Oblivion.esm";
		case NEHRIM:
			return "Nehrim.esm";
		case SKYRIM:
			return "Skyrim.esm";
		case FALLOUT3:
			return "Fallout3.esm";
		case FALLOUTNV:
			return "FalloutNV.esm";
		default:
			return "No Game Detected";
		}
	}

	BOSS_COMMON void SetDataPath(uint32_t game) {
		LOG_INFO("Setting data path for game: \"%s\"", GetGameString(game).c_str());
		if (game == AUTODETECT || fs::exists(data_path / GetGameMasterFile(game))) {
			data_path = boss_path / ".." / "Data";
			return;
		}
		switch (game) {
		case OBLIVION:
			if (RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Bethesda Softworks\\Oblivion"))
				data_path = fs::path(RegKeyStringValue("HKEY_LOCAL_MACHINE", "Software\\Bethesda Softworks\\Oblivion", "Installed Path")) / "Data";
			else if (RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Wow6432Node\\Bethesda Softworks\\Oblivion"))
				data_path = fs::path(RegKeyStringValue("HKEY_LOCAL_MACHINE", "Software\\Wow6432Node\\Bethesda Softworks\\Oblivion", "Installed Path")) / "Data";
			else
				data_path = boss_path / ".." / "Data";
			break;
		case NEHRIM:
			if (RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Nehrim - At Fate's Edge_is1"))
				data_path = fs::path(RegKeyStringValue("HKEY_LOCAL_MACHINE", "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Nehrim - At Fate's Edge_is1", "InstallLocation")) / "Data";
			else
				data_path = boss_path / ".." / "Data";
			break;
		case SKYRIM:
			if (RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Bethesda Softworks\\Skyrim"))
				data_path = fs::path(RegKeyStringValue("HKEY_LOCAL_MACHINE", "Software\\Bethesda Softworks\\Skyrim", "Installed Path")) / "Data";
			else if (RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Wow6432Node\\Bethesda Softworks\\Skyrim"))
				data_path = fs::path(RegKeyStringValue("HKEY_LOCAL_MACHINE", "Software\\Wow6432Node\\Bethesda Softworks\\Skyrim", "Installed Path")) / "Data";
			else
				data_path = boss_path / ".." / "Data";
			break;
		case FALLOUT3:
			if (RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Bethesda Softworks\\Fallout3"))
				data_path = fs::path(RegKeyStringValue("HKEY_LOCAL_MACHINE", "Software\\Bethesda Softworks\\Fallout3", "Installed Path")) / "Data";
			else if (RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Wow6432Node\\Bethesda Softworks\\Fallout3"))
				data_path = fs::path(RegKeyStringValue("HKEY_LOCAL_MACHINE", "Software\\Wow6432Node\\Bethesda Softworks\\Fallout3", "Installed Path")) / "Data";
			else
				data_path = boss_path / ".." / "Data";
			break;
		case FALLOUTNV:
			if (RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Bethesda Softworks\\FalloutNV"))
				data_path = fs::path(RegKeyStringValue("HKEY_LOCAL_MACHINE", "Software\\Bethesda Softworks\\FalloutNV", "Installed Path")) / "Data";
			else if (RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Wow6432Node\\Bethesda Softworks\\FalloutNV"))
				data_path = fs::path(RegKeyStringValue("HKEY_LOCAL_MACHINE", "Software\\Wow6432Node\\Bethesda Softworks\\FalloutNV", "Installed Path")) / "Data";
			else
				data_path = boss_path / ".." / "Data";
			break;
		}
	}

	void AutodetectGame(void * parent) {  //Throws exception if error.
		LOG_INFO("Autodetecting game.");
		if (fs::exists(data_path / "Nehrim.esm"))  //Before Oblivion because Nehrim installs can have Oblivion.esm for porting mods.
			gl_current_game = NEHRIM;
		else if (fs::exists(data_path / "Oblivion.esm"))
			gl_current_game = OBLIVION;
		else if (fs::exists(data_path / "FalloutNV.esm"))   //Before Fallout 3 because some mods for New Vegas require Fallout3.esm.
			gl_current_game = FALLOUTNV;
		else if (fs::exists(data_path / "Fallout3.esm")) 
			gl_current_game = FALLOUT3;
		else if (fs::exists(data_path / "Skyrim.esm")) 
			gl_current_game = SKYRIM;
		else {
			LOG_INFO("Game not detected locally. Checking Registry for paths.");
			/* New behaviour:
			1. Try to detect all games.
			2. If CLI, list detected games. If GUI, list all games, denoting missing games.
			3. If CLI and no games are detected, quit.
			4. If CLI and one game detected, run for that.
			5. Otherwise, use GUI logic.*/
			size_t ans;
#ifdef BOSSGUI
			/* New behaviour:
			1. If a game that is installed is selected, run for that game.
			2. If a game that is not installed is selected, run for that game in update only mode.
			3. If the dialog is cancelled, quit.
			*/
			wxArrayString choices;
			string text;
			text = GetGameString(OBLIVION);
			if (!RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Bethesda Softworks\\Oblivion") && !RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Wow6432Node\\Bethesda Softworks\\Oblivion"))
				text += " (not detected)";
			choices.Add(text);
			text = GetGameString(NEHRIM);
			if (!RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Nehrim - At Fate's Edge_is1"))
				text += " (not detected)";
			choices.Add(text);
			text = GetGameString(SKYRIM);
			if (!RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Bethesda Softworks\\Skyrim") && !RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Wow6432Node\\Bethesda Softworks\\Skyrim"))
				text += " (not detected)";
			choices.Add(text);
			text = GetGameString(FALLOUT3);
			if (!RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Bethesda Softworks\\Fallout3") && !RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Wow6432Node\\Bethesda Softworks\\Fallout3"))
				text += " (not detected)";
			choices.Add(text);
			text = GetGameString(FALLOUTNV);
			if (!RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Bethesda Softworks\\FalloutNV") && !RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Wow6432Node\\Bethesda Softworks\\FalloutNV"))
				text += " (not detected)";
			choices.Add(text);

			wxSingleChoiceDialog* choiceDia = new wxSingleChoiceDialog((wxWindow*)parent, wxT("Please pick which game to run BOSS for:"),
				wxT("BOSS: Select Game"), choices);

			if (choiceDia->ShowModal() != wxID_OK)
				throw boss_error(BOSS_ERROR_NO_MASTER_FILE);

			ans = choiceDia->GetSelection();
			choiceDia->Close(true);

			if (ans == 0)
				gl_current_game = OBLIVION;
			else if (ans == 1)
				gl_current_game = NEHRIM;
			else if (ans == 2)
				gl_current_game = SKYRIM;
			else if (ans == 3)
				gl_current_game = FALLOUT3;
			else if (ans == 4)
				gl_current_game = FALLOUTNV;
			else
				throw boss_error(BOSS_ERROR_NO_MASTER_FILE);
#else
			vector<uint32_t> gamesDetected;
			//Look for Windows Registry entries for the games.
			if (RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Bethesda Softworks\\Oblivion") || RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Wow6432Node\\Bethesda Softworks\\Oblivion")) //Look for Oblivion.
				gamesDetected.push_back(OBLIVION);
			if (RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Nehrim - At Fate's Edge_is1")) //Look for Nehrim.
				gamesDetected.push_back(NEHRIM);
			if (RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Bethesda Softworks\\Skyrim") || RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Wow6432Node\\Bethesda Softworks\\Skyrim")) //Look for Skyrim.
				gamesDetected.push_back(SKYRIM);
			if (RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Bethesda Softworks\\Fallout3") || RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Wow6432Node\\Bethesda Softworks\\Fallout3")) //Look for Fallout 3.
				gamesDetected.push_back(FALLOUT3);
			if (RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Bethesda Softworks\\FalloutNV") || RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Wow6432Node\\Bethesda Softworks\\FalloutNV")) //Look for Fallout New Vegas.
				gamesDetected.push_back(FALLOUTNV);

			//Now check what games were found.
			if (gamesDetected.empty())
				throw boss_error(BOSS_ERROR_NO_MASTER_FILE);
			else if (gamesDetected.size() == 1)
				gl_current_game = gamesDetected.front();
			else {
				//Ask user to choose game.
				cout << endl << "Please pick which game to run BOSS for:" << endl;
				for (size_t i=0; i < gamesDetected.size(); i++)
					cout << i << " : " << GetGameString(gamesDetected[i]) << endl;

				cin >> ans;
				if (ans < 0 || ans >= gamesDetected.size()) {
					cout << "Invalid selection." << endl;
					throw boss_error(BOSS_ERROR_NO_MASTER_FILE);
				}
			}
			gl_current_game = gamesDetected[ans];
#endif
		}
	}

	BOSS_COMMON vector<uint32_t> DetectGame(void * parent) {
		//Detect all installed games.
		vector<uint32_t> games;
		if (fs::exists(data_path / "Oblivion.esm") || RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Bethesda Softworks\\Oblivion") || RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Wow6432Node\\Bethesda Softworks\\Oblivion")) //Look for Oblivion.
			games.push_back(OBLIVION);
		if (fs::exists(data_path / "Nehrim.esm") || RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Nehrim - At Fate's Edge_is1")) //Look for Nehrim.
			games.push_back(NEHRIM);
		if (fs::exists(data_path / "Skyrim.esm") || RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Bethesda Softworks\\Skyrim") || RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Wow6432Node\\Bethesda Softworks\\Skyrim")) //Look for Skyrim.
			games.push_back(SKYRIM);
		if (fs::exists(data_path / "Fallout3.esm") || RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Bethesda Softworks\\Fallout3") || RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Wow6432Node\\Bethesda Softworks\\Fallout3")) //Look for Fallout 3.
			games.push_back(FALLOUT3);
		if (fs::exists(data_path / "FalloutNV.esm") || RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Bethesda Softworks\\FalloutNV") || RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Wow6432Node\\Bethesda Softworks\\FalloutNV")) //Look for Fallout New Vegas.
			games.push_back(FALLOUTNV);
		//Now set gl_current_game.
		if (gl_game != AUTODETECT) {
			if (gl_update_only)
				gl_current_game = gl_game;  //Assumed to be local, no data_path change needed.
			else {
				//Check for game. Check locally and for both registry entries.
				if (fs::exists(data_path / GetGameMasterFile(gl_game)))
					gl_current_game = gl_game;
				else if (gl_game == OBLIVION && (RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Bethesda Softworks\\Oblivion") || RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Wow6432Node\\Bethesda Softworks\\Oblivion")))  //Look for Oblivion.
					gl_current_game = gl_game;
				else if (gl_game == NEHRIM && RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Nehrim - At Fate's Edge_is1"))  //Look for Nehrim.
					gl_current_game = gl_game;
				else if (gl_game == SKYRIM && (RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Bethesda Softworks\\Skyrim") || RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Wow6432Node\\Bethesda Softworks\\Skyrim")))  //Look for Skyrim.
					gl_current_game = gl_game;
				else if (gl_game == FALLOUT3 && (RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Bethesda Softworks\\Fallout3") || RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Wow6432Node\\Bethesda Softworks\\Fallout3")))  //Look for Fallout 3.
					gl_current_game = gl_game;
				else if (gl_game == FALLOUTNV && (RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Bethesda Softworks\\FalloutNV") || RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Wow6432Node\\Bethesda Softworks\\FalloutNV")))  //Look for Fallout New Vegas.
					gl_current_game = gl_game;
				else
					AutodetectGame(parent);  //Game not found. Autodetect.
			}
		} else
			AutodetectGame(parent);
		//Now set data_path.
		SetDataPath(gl_current_game);
		//Make sure that boss_game_path() exists.
		try {
			if (!fs::exists(boss_game_path()))
				fs::create_directory(boss_game_path());
		} catch (fs::filesystem_error e) {
			throw boss_error(BOSS_ERROR_FS_CREATE_DIRECTORY_FAIL, GetGameMasterFile(gl_current_game), e.what());
		}
		return games;
	}

	BOSS_COMMON time_t GetMasterTime() {  //Throws exception if error.
		try {
			switch (gl_current_game) {
			case OBLIVION:
				return fs::last_write_time(data_path / "Oblivion.esm");
			case NEHRIM:
				return fs::last_write_time(data_path / "Nehrim.esm");
			case SKYRIM:
				return fs::last_write_time(data_path / "Skyrim.esm");
			case FALLOUT3:
				return fs::last_write_time(data_path / "Fallout3.esm");
			case FALLOUTNV:
				return fs::last_write_time(data_path / "FalloutNV.esm");
			default:
				throw boss_error(BOSS_ERROR_NO_MASTER_FILE);
			}
		} catch(fs::filesystem_error e) {
			throw boss_error(BOSS_ERROR_FS_FILE_MOD_TIME_READ_FAIL, GetGameMasterFile(gl_current_game), e.what());
		}
	}


	///////////////////////////////
	//Settings Class
	///////////////////////////////

	void	Settings::Load			(fs::path file) {
		Skipper skipper;
		ini_grammar grammar;
		string::const_iterator begin, end;
		string contents;
		
		skipper.SkipIniComments(true);
		grammar.SetErrorBuffer(&errorBuffer);

		fileToBuffer(file,contents);

		begin = contents.begin();
		end = contents.end();
		
	//	iterator_type u32b(begin);
	//	iterator_type u32e(end);

	//	bool r = phrase_parse(u32b, u32e, grammar, skipper, iniSettings);
		bool r = phrase_parse(begin, end, grammar, skipper, iniSettings);

		if (!r || begin != end)  //This might not work correctly.
			throw boss_error(BOSS_ERROR_FILE_PARSE_FAIL, file.string());

		iniFile = file.filename().string();
		ApplyIniSettings();
	}

	void	Settings::Save			(fs::path file) {
		ofstream ini(file.c_str(), ios_base::trunc);
		if (ini.fail())
			throw boss_error(BOSS_ERROR_FILE_WRITE_FAIL, file.string());
		ini <<  '\xEF' << '\xBB' << '\xBF'  //Write UTF-8 BOM to ensure the file is recognised as having the UTF-8 encoding.
			<<	"#---------------" << endl
			<<	"# BOSS Ini File" << endl
			<<	"#---------------" << endl
			<<	"# Settings with names starting with 'b' are boolean and accept values of 'true' or 'false'." << endl
			<<	"# Settings with names starting with 'i' are unsigned integers and accept varying ranges of whole numbers." << endl
			<<	"# Settings with names starting with 's' are strings and their accepted values vary." << endl
			<<	"# See the BOSS ReadMe for details on what each setting does and the accepted values for integer and string settings." << endl << endl

			<<	"[BOSS.GeneralSettings]" << endl
			<<	"bDoStartupUpdateCheck    = " << BoolToString(gl_do_startup_update_check) << endl
			<<	"bUseUserRulesEditor      = " << BoolToString(gl_use_user_rules_editor) << endl << endl

			<<	"[BOSS.InternetSettings]" << endl
			<<	"sProxyHostname           = " << gl_proxy_host << endl
			<<	"iProxyPort               = " << IntToString(gl_proxy_port) << endl
			<<	"sProxyUsername           = " << gl_proxy_user << endl
			<<	"sProxyPassword           = " << gl_proxy_passwd << endl << endl

			<<	"[BOSS.RunOptions]" << endl
			<<	"sGame                    = " << GetIniGameString() << endl
			<<	"sBOSSLogFormat           = " << GetLogFormatString() << endl
			<<	"iDebugVerbosity          = " << IntToString(gl_debug_verbosity) << endl
			<<	"iRevertLevel             = " << IntToString(gl_revert) << endl
			<<	"bUpdateMasterlist        = " << BoolToString(gl_update) << endl
			<<	"bOnlyUpdateMasterlist    = " << BoolToString(gl_update_only) << endl
			<<	"bSilentRun               = " << BoolToString(gl_silent) << endl
			<<	"bNoVersionParse          = " << BoolToString(gl_skip_version_parse) << endl
			<<	"bDebugWithSourceRefs     = " << BoolToString(gl_debug_with_source) << endl
			<<	"bDisplayCRCs             = " << BoolToString(gl_show_CRCs) << endl
			<<	"bDoTrialRun              = " << BoolToString(gl_trial_run) << endl
			<<	"bLogDebugOutput          = " << BoolToString(gl_log_debug_output) << endl << endl
			
			<<	"[BOSSLog.Filters]" << endl
			<<	"bUseDarkColourScheme     = " << BoolToString(UseDarkColourScheme) << endl
			<<	"bHideVersionNumbers      = " << BoolToString(HideVersionNumbers) << endl
			<<	"bHideGhostedLabel        = " << BoolToString(HideGhostedLabel) << endl
			<<	"bHideActiveLabel         = " << BoolToString(HideActiveLabel) << endl
			<<	"bHideChecksums           = " << BoolToString(HideChecksums) << endl
			<<	"bHideMessagelessMods     = " << BoolToString(HideMessagelessMods) << endl
			<<	"bHideGhostedMods         = " << BoolToString(HideGhostedMods) << endl
			<<	"bHideCleanMods           = " << BoolToString(HideCleanMods) << endl
			<<	"bHideRuleWarnings        = " << BoolToString(HideRuleWarnings) << endl
			<<	"bHideAllModMessages      = " << BoolToString(HideAllModMessages) << endl
			<<	"bHideNotes               = " << BoolToString(HideNotes) << endl
			<<	"bHideBashTagSuggestions  = " << BoolToString(HideBashTagSuggestions) << endl
			<<	"bHideRequirements        = " << BoolToString(HideRequirements) << endl
			<<	"bHideIncompatibilities   = " << BoolToString(HideIncompatibilities) << endl
			<<	"bHideDoNotCleanMessages  = " << BoolToString(HideDoNotCleanMessages) << endl
			<<	"bHideInactiveMods        = " << BoolToString(HideInactivePlugins) << endl << endl

			<<	"[BOSSLog.Styles]" << endl
			<<	"# A style with nothing specified uses the coded defaults." << endl
			<<	"# These defaults are given in the BOSS ReadMe as with the rest of the ini settings." << endl
			<<	"\"body\"                                     = " << CSSBody << endl
			<<	"\"#darkBody\"                                = " << CSSDarkBody << endl
			<<	"\".darkLink:link\"                           = " << CSSDarkLink << endl
			<<	"\".darkLink:visited\"                        = " << CSSDarkLinkVisited << endl
			<<	"\"#filters\"                                 = " << CSSFilters << endl
			<<	"\"#filters > li\"                            = " << CSSFiltersList << endl
			<<	"\"#darkFilters\"                             = " << CSSDarkFilters << endl
			<<	"\"body > div:first-child\"                   = " << CSSTitle << endl
			<<	"\"body > div:first-child + div\"             = " << CSSCopyright << endl
			<<	"\"h3 + *\"                                   = " << CSSSections << endl
			<<	"\"h3\"                                       = " << CSSSectionTitle << endl
			<<	"\"h3 > span\"                                = " << CSSSectionPlusMinus << endl
			<<	"\"#end\"                                     = " << CSSLastSection << endl
			<<	"\"td\"                                       = " << CSSTable << endl
			<<	"\"ul\"                                       = " << CSSList << endl
			<<	"\"ul li\"                                    = " << CSSListItem << endl
			<<	"\"li ul\"                                    = " << CSSSubList << endl
			<<	"\"input[type='checkbox']\"                   = " << CSSCheckbox << endl
			<<	"\"blockquote\"                               = " << CSSBlockquote << endl
			<<	"\".error\"                                   = " << CSSError << endl
			<<	"\".warn\"                                    = " << CSSWarning << endl
			<<	"\".success\"                                 = " << CSSSuccess << endl
			<<	"\".version\"                                 = " << CSSVersion << endl
			<<	"\".ghosted\"                                 = " << CSSGhost << endl
			<<	"\".crc\"                                     = " << CSSCRC << endl
			<<	"\".active\"                                  = " << CSSActive << endl
			<<	"\".tagPrefix\"                               = " << CSSTagPrefix << endl
			<<	"\".dirty\"                                   = " << CSSDirty << endl
			<<	"\".message\"                                 = " << CSSQuotedMessage << endl
			<<	"\".mod\"                                     = " << CSSMod << endl
			<<	"\".tag\"                                     = " << CSSTag << endl
			<<	"\".note\"                                    = " << CSSNote << endl
			<<	"\".req\"                                     = " << CSSRequirement << endl
			<<	"\".inc\"                                     = " << CSSIncompatibility;
		ini.close();
	}

	string	Settings::GetIniGameString	() const {
		if (gl_game == AUTODETECT)
			return "auto";
		else if (gl_game == OBLIVION)
			return "Oblivion";
		else if (gl_game == FALLOUT3)
			return "Fallout3";
		else if (gl_game == NEHRIM)
			return "Nehrim";
		else if (gl_game == FALLOUTNV)
			return "FalloutNV";
		else if (gl_game == SKYRIM)
			return "Skyrim";
		else
			return "";
	}

	string Settings::GetLogFormatString() const {
		if (gl_log_format == HTML)
			return "html";
		else
			return "text";
	}

	ParsingError Settings::ErrorBuffer() const {
		return errorBuffer;
	}

	void Settings::ErrorBuffer(ParsingError buffer) {
		errorBuffer = buffer;
	}

	void Settings::ApplyIniSettings() {
		if (iniFile == "BOSS.ini") {
			for (vector<IniPair>::iterator iter = iniSettings.begin(); iter != iniSettings.end(); ++iter) {
				if (iter->value.empty())
					continue;
				boost::algorithm::trim(iter->key);  //Make sure there are no preceding or trailing spaces.
				boost::algorithm::trim(iter->value);  //Make sure there are no preceding or trailing spaces.

				//String settings.
				if (iter->key == "sProxyHostname")
					gl_proxy_host = iter->value;
				else if (iter->key == "sProxyUsername")
					gl_proxy_user = iter->value;
				else if (iter->key == "sProxyPassword")
					gl_proxy_passwd = iter->value;
				else if (iter->key == "sBOSSLogFormat") {
					if (iter->value == "html")
						gl_log_format = HTML;
					else
						gl_log_format = PLAINTEXT;
				} else if (iter->key == "sGame") {
					if (iter->value == "auto")
						gl_game = AUTODETECT;
					else if (iter->value == "Oblivion")
						gl_game = OBLIVION;
					else if (iter->value == "Nehrim")
						gl_game = NEHRIM;
					else if (iter->value == "Fallout3")
						gl_game = FALLOUT3;
					else if (iter->value == "FalloutNV")
						gl_game = FALLOUTNV;
					else if (iter->value == "Skyrim")
						gl_game = SKYRIM;
				}
				else if (iter->key == "body")
					CSSBody = iter->value;
				else if (iter->key == "#darkBody")
					CSSDarkBody = iter->value;
				else if (iter->key == ".darkLink:link")
					CSSDarkLink = iter->value;
				else if (iter->key == ".darkLink:visited")
					CSSDarkLinkVisited = iter->value;
				else if (iter->key == "#filters")
					CSSFilters = iter->value;
				else if (iter->key == "#filters > li")
					CSSFiltersList = iter->value;
				else if (iter->key == "#darkFilters")
					CSSDarkFilters = iter->value;
				else if (iter->key == "body > div:first-child")
					CSSTitle = iter->value;
				else if (iter->key == "body > div:first-child + div")
					CSSCopyright = iter->value;
				else if (iter->key == "h3 + *")
					CSSSections = iter->value;
				else if (iter->key == "h3")
					CSSSectionTitle = iter->value;
				else if (iter->key == "h3 > span")
					CSSSectionPlusMinus = iter->value;
				else if (iter->key == "#end")
					CSSLastSection = iter->value;
				else if (iter->key == "td")
					CSSTable = iter->value;
				else if (iter->key == "ul")
					CSSList = iter->value;
				else if (iter->key == "ul li")
					CSSListItem = iter->value;
				else if (iter->key == "li ul")
					CSSSubList = iter->value;
				else if (iter->key == "input[type='checkbox']")
					CSSCheckbox = iter->value;
				else if (iter->key == "blockquote")
					CSSBlockquote = iter->value;
				else if (iter->key == ".error")
					CSSError = iter->value;
				else if (iter->key == ".warn")
					CSSWarning = iter->value;
				else if (iter->key == ".success")
					CSSSuccess = iter->value;
				else if (iter->key == ".version")
					CSSVersion = iter->value;
				else if (iter->key == ".ghosted")
					CSSGhost = iter->value;
				else if (iter->key == ".crc")
					CSSCRC = iter->value;
				else if (iter->key == ".active")
					CSSActive = iter->value;
				else if (iter->key == ".tagPrefix")
					CSSTagPrefix = iter->value;
				else if (iter->key == ".dirty")
					CSSDirty = iter->value;
				else if (iter->key == ".message")
					CSSQuotedMessage = iter->value;
				else if (iter->key == ".mod")
					CSSMod = iter->value;
				else if (iter->key == ".tag")
					CSSTag = iter->value;
				else if (iter->key == ".note")
					CSSNote = iter->value;
				else if (iter->key == ".req")
					CSSRequirement = iter->value;
				else if (iter->key == ".inc")
					CSSIncompatibility = iter->value;
				//Now integers.
				else if (iter->key == "iProxyPort")
					gl_proxy_port = atoi(iter->value.c_str());
				else if (iter->key == "iRevertLevel") {
					uint32_t value = atoi(iter->value.c_str());
					if (value >= 0 && value < 3)
						gl_revert = value;
				} else if (iter->key == "iDebugVerbosity") {
					uint32_t value = atoi(iter->value.c_str());
					if (value >= 0 && value < 4)
						gl_debug_verbosity = value;
				//Now on to BOSS.ini boolean settings.
				} else if (iter->key == "bDoStartupUpdateCheck")
					gl_do_startup_update_check = StringToBool(iter->value);
				else if (iter->key == "bUseUserRulesEditor")
					gl_use_user_rules_editor = StringToBool(iter->value);
				if (iter->key == "bUpdateMasterlist")
					gl_update = StringToBool(iter->value);
				else if (iter->key == "bOnlyUpdateMasterlist")
					gl_update_only = StringToBool(iter->value);
				else if (iter->key == "bSilentRun")
					gl_silent = StringToBool(iter->value);
				else if (iter->key == "bNoVersionParse")
					gl_skip_version_parse = StringToBool(iter->value);
				else if (iter->key == "bDebugWithSourceRefs")
					gl_debug_with_source = StringToBool(iter->value);
				else if (iter->key == "bDisplayCRCs")
					gl_show_CRCs = StringToBool(iter->value);
				else if (iter->key == "bDoTrialRun")
					gl_trial_run = StringToBool(iter->value);
				else if (iter->key == "bLogDebugOutput")
					gl_log_debug_output = StringToBool(iter->value);
				if (iter->key == "bUseDarkColourScheme")
					UseDarkColourScheme = StringToBool(iter->value);
				else if (iter->key == "bHideVersionNumbers")
					HideVersionNumbers = StringToBool(iter->value);
				else if (iter->key == "bHideGhostedLabel")
					HideGhostedLabel = StringToBool(iter->value);
				else if (iter->key == "bHideActiveLabel")
					HideActiveLabel = StringToBool(iter->value);
				else if (iter->key == "bHideChecksums")
					HideChecksums = StringToBool(iter->value);
				else if (iter->key == "bHideMessagelessMods")
					HideMessagelessMods = StringToBool(iter->value);
				else if (iter->key == "bHideGhostedMods")
					HideGhostedMods = StringToBool(iter->value);
				else if (iter->key == "bHideCleanMods")
					HideCleanMods = StringToBool(iter->value);
				else if (iter->key == "bHideRuleWarnings")
					HideRuleWarnings = StringToBool(iter->value);
				else if (iter->key == "bHideAllModMessages")
					HideAllModMessages = StringToBool(iter->value);
				else if (iter->key == "bHideNotes")
					HideNotes = StringToBool(iter->value);
				else if (iter->key == "bHideBashTagSuggestions")
					HideBashTagSuggestions = StringToBool(iter->value);
				else if (iter->key == "bHideRequirements")
					HideRequirements = StringToBool(iter->value);
				else if (iter->key == "bHideIncompatibilities")
					HideIncompatibilities = StringToBool(iter->value);
				else if (iter->key == "bHideDoNotCleanMessages")
					HideDoNotCleanMessages = StringToBool(iter->value);
				else if (iter->key == "bHideInactivePlugins")
					HideInactivePlugins = StringToBool(iter->value);
			}
		} else if (iniFile == "Oblivion.ini") {
			for (vector<IniPair>::iterator iter = iniSettings.begin(); iter != iniSettings.end(); ++iter) {
				if (iter->value.empty())
					continue;
				boost::algorithm::trim(iter->key);  //Make sure there are no preceding or trailing spaces.
				boost::algorithm::trim(iter->value);  //Make sure there are no preceding or trailing spaces.

				//Don't evaluate whether these should have any effect here, as the game will not have been set yet.
				if (iter->key == "bUseMyGamesDirectory")
					gl_using_local_app_data_folder = StringToBool(iter->value);
				/*else if (iter->key == "sLocalMasterPath")
					//This setting does not seem to function correctly on Oblivion, and isn't used in Fallout 3, Fallout New Vegas and Skyrim.
					gl_local_data_path = fs::path(iter->value);  //Incorrect if absolute paths can be specified.*/
			}
		}
	}
}