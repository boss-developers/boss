/*	Better Oblivion Sorting Software
	
	A "one-click" program for users that quickly optimises and avoids 
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge, 
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

    Copyright (C) 2009-2012    BOSS Development Team.

	This file is part of Better Oblivion Sorting Software.

    Better Oblivion Sorting Software is free software: you can redistribute 
	it and/or modify it under the terms of the GNU General Public License 
	as published by the Free Software Foundation, either version 3 of 
	the License, or (at your option) any later version.

    Better Oblivion Sorting Software is distributed in the hope that it will 
	be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Better Oblivion Sorting Software.  If not, see 
	<http://www.gnu.org/licenses/>.

	$Revision: 3184 $, $Date: 2011-08-26 20:52:13 +0100 (Fri, 26 Aug 2011) $
*/
#define wxUSE_CHOICEDLG 1

#include "Common/Globals.h"
#include "Parsing/Grammar.h"
#include "Support/Helpers.h"
#include "Output/Output.h"

#ifdef BOSSGUI
#include <wx/choicdlg.h>
#include <wx/arrstr.h>
#endif

namespace boss {
	using namespace std;

	BOSS_COMMON const string gl_boss_release_date	= "1 January 2012";

	BOSS_COMMON uint32_t gl_current_game			= AUTODETECT;

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
		return GetLocalAppDataPath() / "Skyrim" / "plugins.txt";
	}

	BOSS_COMMON fs::path loadorder_path() {
		return GetLocalAppDataPath() / "Skyrim" / "loadorder.txt";
	}

	///////////////////////////////
	//Ini Settings
	///////////////////////////////

	//GUI variables
	BOSS_COMMON uint32_t	gl_run_type					= 0;
	BOSS_COMMON	bool		gl_use_user_rules_editor	= false;

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
		if (gl_update_only || game == AUTODETECT || fs::exists(data_path / GetGameMasterFile(game))) {
			data_path = boss_path / ".." / "Data";
			return;
		}
		switch (game) {
		case OBLIVION:
			if (RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Bethesda Softworks\\Oblivion"))
				data_path = fs::path(RegKeyStringValue("HKEY_LOCAL_MACHINE", "Software\\Bethesda Softworks\\Oblivion", "Installed Path") + "Data");
			else if (RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Wow6432Node\\Bethesda Softworks\\Oblivion"))
				data_path = fs::path(RegKeyStringValue("HKEY_LOCAL_MACHINE", "Software\\Wow6432Node\\Bethesda Softworks\\Oblivion", "Installed Path") + "Data");
			break;
		case NEHRIM:
			if (RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Nehrim - At Fate's Edge_is1"))
				data_path = fs::path(RegKeyStringValue("HKEY_LOCAL_MACHINE", "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Nehrim - At Fate's Edge_is1", "InstallLocation") + "Data");
			break;
		case SKYRIM:
			if (RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Bethesda Softworks\\Skyrim"))
				data_path = fs::path(RegKeyStringValue("HKEY_LOCAL_MACHINE", "Software\\Bethesda Softworks\\Skyrim", "Installed Path") + "Data");
			else if (RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Wow6432Node\\Bethesda Softworks\\Skyrim"))
				data_path = fs::path(RegKeyStringValue("HKEY_LOCAL_MACHINE", "Software\\Wow6432Node\\Bethesda Softworks\\Skyrim", "Installed Path") + "Data");
			break;
		case FALLOUT3:
			if (RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Bethesda Softworks\\Fallout3"))
				data_path = fs::path(RegKeyStringValue("HKEY_LOCAL_MACHINE", "Software\\Bethesda Softworks\\Fallout3", "Installed Path") + "Data");
			else if (RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Wow6432Node\\Bethesda Softworks\\Fallout3"))
				data_path = fs::path(RegKeyStringValue("HKEY_LOCAL_MACHINE", "Software\\Wow6432Node\\Bethesda Softworks\\Fallout3", "Installed Path") + "Data");
			break;
		case FALLOUTNV:
			if (RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Bethesda Softworks\\FalloutNV"))
				data_path = fs::path(RegKeyStringValue("HKEY_LOCAL_MACHINE", "Software\\Bethesda Softworks\\FalloutNV", "Installed Path") + "Data");
			else if (RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\Wow6432Node\\Bethesda Softworks\\FalloutNV"))
				data_path = fs::path(RegKeyStringValue("HKEY_LOCAL_MACHINE", "Software\\Wow6432Node\\Bethesda Softworks\\FalloutNV", "Installed Path") + "Data");
			break;
		}
	}

	void AutodetectGame(void * parent) {  //Throws exception if error.
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
				size_t ans;
				if (parent != NULL) {
#ifdef BOSSGUI
					wxArrayString choices;
					for (size_t i=0; i < gamesDetected.size(); i++)
						choices.Add(GetGameString(gamesDetected[i]));

					wxSingleChoiceDialog* choiceDia = new wxSingleChoiceDialog((wxWindow*)parent, wxT("Please pick which game to run BOSS for:"),
						wxT("BOSS: Select Game"), choices);

					if (choiceDia->ShowModal() == wxID_OK) {
						ans = choiceDia->GetSelection();
						choiceDia->Close(true);
					} else
						throw boss_error(BOSS_ERROR_NO_MASTER_FILE);
#endif
				} else {
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
			}
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
		Skipper skipper(true);
		ini_grammar grammar;
		string::const_iterator begin, end;
		string contents;

		grammar.SetErrorBuffer(&errorBuffer);

		fileToBuffer(file,contents);

		begin = contents.begin();
		end = contents.end();

		bool r = phrase_parse(begin, end, grammar, skipper);

		if (!r || begin != end)  //This might not work correctly.
			throw boss_error(BOSS_ERROR_FILE_PARSE_FAIL, file.string());
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
			<<	"bHideDoNotCleanMessages  = " << BoolToString(HideDoNotCleanMessages) << endl << endl

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
}