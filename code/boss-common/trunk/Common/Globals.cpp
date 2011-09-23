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

	ofstream bosslog;					//BOSSlog.txt - output file.

	fs::path data_path			= fs::path("..") / "Data";
	fs::path boss_path			= fs::path(".");
	fs::path bosslog_html_path	= boss_path / "BOSSlog.html";
	fs::path bosslog_text_path	= boss_path / "BOSSlog.txt";
	fs::path masterlist_path	= boss_path / "masterlist.txt";
	fs::path userlist_path		= boss_path / "userlist.txt";
	fs::path curr_modlist_path	= boss_path / "modlist.txt";
	fs::path prev_modlist_path	= boss_path / "modlist.old";
	fs::path ini_path			= boss_path / "BOSS.ini";
	fs::path debug_log_path		= boss_path / "BOSSDebugLog.txt";

	const string g_version     = "1.9";
	const string g_releaseDate = "X Y, 2011";

	//Command line variables
	string log_format		= "html";	// what format the output should be in.
	string proxy_type		= "direct";
	string proxy_host		= "none";
	int proxy_port			= 0;
	int game				= 0;		// What game's mods are we sorting? 1 = Oblivion, 2 = Fallout 3, 3 = Nehrim, 4 = Fallout: New Vegas, 5 = Skyrim.
	int revert              = 0;		// what level to revert to
	int debug_verbosity     = 0;		// log levels above INFO to output
	bool update				= true;		// update the masterlist?
	bool update_only        = false;	// only update the masterlist and don't sort currently.
	bool silent             = false;	// silent mode?
	bool skip_version_parse = false;	// enable parsing of mod's headers to look for version strings
	bool debug_with_source  = false;	// whether to include origin information in logging statements
	bool show_CRCs			= false;	// whether or not to show mod CRCs.
	bool trial_run			= false;	// If true, don't redate files.
	bool log_debug_output		= false;  //If true, logs the Logger output in BOSSDebugLog.txt.
	bool do_startup_update_check	= true;	// Whether or not to check for updates on startup.

	//GUI variables
	int run_type					= 1;  // 1 = sort mods, 2 = only update, 3 = undo changes.
	bool use_user_rules_editor		= false;		//Use the User Rules Editor or edit userlist.txt directly?
}