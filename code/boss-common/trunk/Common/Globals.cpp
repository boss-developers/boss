/*	Better Oblivion Sorting Software
	
	A "one-click" program for users that quickly optimises and avoids 
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge, 
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

    Copyright (C) 2011  Random/Random007/jpearce, WrinklyNinja & the BOSS 
	development team. Copyright license:
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 3184 $, $Date: 2011-08-26 20:52:13 +0100 (Fri, 26 Aug 2011) $
*/

#include "Common/Globals.h"

namespace boss {
	using namespace std;

	BOSS_COMMON_EXP const string boss_release_date = "13 November, 2011";

	BOSS_COMMON_EXP fs::path data_path			= fs::path("..") / "Data";
	BOSS_COMMON_EXP const fs::path boss_path			= fs::path(".");
	BOSS_COMMON_EXP const fs::path bosslog_html_path	= boss_path / "BOSSlog.html";
	BOSS_COMMON_EXP const fs::path bosslog_text_path	= boss_path / "BOSSlog.txt";
	BOSS_COMMON_EXP fs::path masterlist_path	= boss_path / "masterlist.txt";
	BOSS_COMMON_EXP fs::path userlist_path		= boss_path / "userlist.txt";
	BOSS_COMMON_EXP const fs::path curr_modlist_path	= boss_path / "modlist.txt";
	BOSS_COMMON_EXP const fs::path prev_modlist_path	= boss_path / "modlist.old";
	BOSS_COMMON_EXP const fs::path ini_path			= boss_path / "BOSS.ini";
	BOSS_COMMON_EXP const fs::path debug_log_path		= boss_path / "BOSSDebugLog.txt";

	//Command line variables
	BOSS_COMMON_EXP string proxy_host		= "none";
	BOSS_COMMON_EXP string proxy_user		= "";
	BOSS_COMMON_EXP string proxy_passwd		= "";
	BOSS_COMMON_EXP unsigned int proxy_port			= 0;
	BOSS_COMMON_EXP unsigned int log_format		= HTML;	// what format the output should be in.
	BOSS_COMMON_EXP unsigned int game				= AUTODETECT;		// What game's mods are we sorting? 1 = Oblivion, 2 = Fallout 3, 3 = Nehrim, 4 = Fallout: New Vegas, 5 = Skyrim.
	BOSS_COMMON_EXP unsigned int revert              = 0;		// what level to revert to
	BOSS_COMMON_EXP unsigned int debug_verbosity     = 0;		// log levels above INFO to output
	BOSS_COMMON_EXP bool update				= true;		// update the masterlist?
	BOSS_COMMON_EXP bool update_only        = false;	// only update the masterlist and don't sort currently.
	BOSS_COMMON_EXP bool silent             = false;	// silent mode?
	BOSS_COMMON_EXP bool skip_version_parse = false;	// enable parsing of mod's headers to look for version strings
	BOSS_COMMON_EXP bool debug_with_source  = false;	// whether to include origin information in logging statements
	BOSS_COMMON_EXP bool show_CRCs			= false;	// whether or not to show mod CRCs.
	BOSS_COMMON_EXP bool trial_run			= false;	// If true, don't redate files.
	BOSS_COMMON_EXP bool log_debug_output		= false;  //If true, logs the Logger output in BOSSDebugLog.txt.
	BOSS_COMMON_EXP bool do_startup_update_check	= true;	// Whether or not to check for updates on startup.

	//GUI variables
	BOSS_COMMON_EXP unsigned int run_type					= 1;  // 1 = sort mods, 2 = only update, 3 = undo changes.
	BOSS_COMMON_EXP bool use_user_rules_editor		= false;		//Use the User Rules Editor or edit userlist.txt directly?
}