/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

	Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
	http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 3184 $, $Date: 2011-08-26 20:52:13 +0100 (Fri, 26 Aug 2011) $
*/

#include "Common/Globals.h"

namespace boss {
	using namespace std;

	BOSS_COMMON const string boss_release_date = "X Y, 2011";

	BOSS_COMMON ofstream bosslog;					//BOSSlog.txt - output file.

	BOSS_COMMON fs::path data_path			= fs::path("..") / "Data";
	BOSS_COMMON fs::path boss_path			= fs::path(".");
	BOSS_COMMON const fs::path bosslog_html_path	= boss_path / "BOSSlog.html";
	BOSS_COMMON const fs::path bosslog_text_path	= boss_path / "BOSSlog.txt";
	BOSS_COMMON fs::path masterlist_path	= boss_path / "masterlist.txt";
	BOSS_COMMON fs::path userlist_path		= boss_path / "userlist.txt";
	BOSS_COMMON const fs::path curr_modlist_path	= boss_path / "modlist.txt";
	BOSS_COMMON const fs::path prev_modlist_path	= boss_path / "modlist.old";
	BOSS_COMMON const fs::path ini_path			= boss_path / "BOSS.ini";
	BOSS_COMMON const fs::path debug_log_path		= boss_path / "BOSSDebugLog.txt";

	//Command line variables
	BOSS_COMMON string log_format		= "html";	// what format the output should be in.
	BOSS_COMMON string proxy_type		= "direct";
	BOSS_COMMON string proxy_host		= "none";
	BOSS_COMMON int proxy_port			= 0;
	BOSS_COMMON int game				= 0;		// What game's mods are we sorting? 1 = Oblivion, 2 = Fallout 3, 3 = Nehrim, 4 = Fallout: New Vegas, 5 = Skyrim.
	BOSS_COMMON int revert              = 0;		// what level to revert to
	BOSS_COMMON int debug_verbosity     = 0;		// log levels above INFO to output
	BOSS_COMMON bool update				= true;		// update the masterlist?
	BOSS_COMMON bool update_only        = false;	// only update the masterlist and don't sort currently.
	BOSS_COMMON bool silent             = false;	// silent mode?
	BOSS_COMMON bool skip_version_parse = false;	// enable parsing of mod's headers to look for version strings
	BOSS_COMMON bool debug_with_source  = false;	// whether to include origin information in logging statements
	BOSS_COMMON bool show_CRCs			= false;	// whether or not to show mod CRCs.
	BOSS_COMMON bool trial_run			= false;	// If true, don't redate files.
	BOSS_COMMON bool log_debug_output		= false;  //If true, logs the Logger output in BOSSDebugLog.txt.
	BOSS_COMMON bool do_startup_update_check	= true;	// Whether or not to check for updates on startup.

	//GUI variables
	BOSS_COMMON int run_type					= 1;  // 1 = sort mods, 2 = only update, 3 = undo changes.
	BOSS_COMMON bool use_user_rules_editor		= false;		//Use the User Rules Editor or edit userlist.txt directly?
}