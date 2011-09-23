/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

	Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
	http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 3135 $, $Date: 2011-08-17 22:01:17 +0100 (Wed, 17 Aug 2011) $
*/

#ifndef __BOSS_GLOBALS_H__
#define __BOSS_GLOBALS_H__

#ifndef _UNICODE
#define _UNICODE	// Tell compiler we're using Unicode, notice the _
#endif

#include <string>
#include <fstream>
#include <boost/filesystem.hpp>


namespace boss {
	using namespace std;
	namespace fs = boost::filesystem;

	extern ofstream bosslog;					//BOSSlog.html output file

	//These paths can't be constant because the API may require data_path and boss_path to be different.
	extern fs::path data_path;			// Holds the path to the data directory.
	extern fs::path boss_path;			// Holds the path to the BOSS directory.
	extern fs::path bosslog_html_path;	// BOSSlog full HTML file name
	extern fs::path bosslog_text_path;	// BOSSlog full text file name
	extern fs::path masterlist_path;	// Hold both location and file name for masterlist.txt
	extern fs::path userlist_path;		// Hold both location and file name for userlist.txt 
	extern fs::path curr_modlist_path;	// Hold both location and file name for modlist.txt
	extern fs::path prev_modlist_path;	// Hold both location and file name for modlist.old
	extern fs::path ini_path;			// Holds the path to the BOSS.ini.
	extern fs::path debug_log_path;		// Holds the path to BOSSDebugLog.txt.

	extern const string g_version;
	extern const string g_releaseDate;

	//Command line variables.
	extern string log_format;		// what format the output should be in.
	extern string proxy_type;
	extern string proxy_host;
	extern int proxy_port;
	extern int game;				// What game's mods are we sorting? 1 = Oblivion, 2 = Fallout 3, 3 = Nehrim, 4 = Fallout: New Vegas, 5 = Skyrim.
	extern int revert;				// what level to revert to
	extern int debug_verbosity;			// log levels above INFO to output
	extern bool update;				// update the masterlist?
	extern bool update_only;		// only update the masterlist and don't sort currently.
	extern bool silent;				// silent mode?
	extern bool skip_version_parse; // enable parsing of mod's headers to look for version strings
	extern bool debug_with_source;				// whether to include origin information in logging statements
	extern bool show_CRCs;			// whether or not to show mod CRCs.
	extern bool trial_run;			// If true, don't redate files.
	extern bool log_debug_output;		//If true, logs command line output in BOSSDebugLog.txt.
	extern bool do_startup_update_check;	// Whether or not to check for updates on startup.
	
	//GUI variables
	extern int run_type;					// 1 = sort mods, 2 = only update, 3 = undo changes.
	extern bool use_user_rules_editor;		//Use the User Rules Editor or edit userlist.txt directly?
}

#endif
