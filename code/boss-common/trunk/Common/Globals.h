/*	Better Oblivion Sorting Software
	
	A "one-click" program for users that quickly optimises and avoids 
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge, 
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

    Copyright (C) 2011    BOSS Development Team.

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

	$Revision: 3135 $, $Date: 2011-08-17 22:01:17 +0100 (Wed, 17 Aug 2011) $
*/

#ifndef __BOSS_GLOBALS_H__
#define __BOSS_GLOBALS_H__

#ifndef _UNICODE
#define _UNICODE	// Tell compiler we're using Unicode, notice the _
#endif

#include <string>
#include <boost/filesystem.hpp>
#include "Common/DllDef.h"

namespace boss {
	using namespace std;
	namespace fs = boost::filesystem;

#	define BOSS_VERSION_MAJOR 1
#	define BOSS_VERSION_MINOR 9
#	define BOSS_VERSION_PATCH 0

	BOSS_COMMON_EXP extern const string boss_release_date;

	//These paths can't be constant because the API may require data_path and boss_path to be different.
	BOSS_COMMON_EXP extern fs::path data_path;			// Holds the path to the data directory.
	BOSS_COMMON_EXP extern const fs::path boss_path;			// Holds the path to the BOSS directory.
	BOSS_COMMON_EXP extern const fs::path bosslog_html_path;	// BOSSlog full HTML file name
	BOSS_COMMON_EXP extern const fs::path bosslog_text_path;	// BOSSlog full text file name
	BOSS_COMMON_EXP extern fs::path masterlist_path;	// Hold both location and file name for masterlist.txt
	BOSS_COMMON_EXP extern fs::path userlist_path;		// Hold both location and file name for userlist.txt 
	BOSS_COMMON_EXP extern const fs::path curr_modlist_path;	// Hold both location and file name for modlist.txt
	BOSS_COMMON_EXP extern const fs::path prev_modlist_path;	// Hold both location and file name for modlist.old
	BOSS_COMMON_EXP extern const fs::path ini_path;			// Holds the path to the BOSS.ini.
	BOSS_COMMON_EXP extern const fs::path debug_log_path;		// Holds the path to BOSSDebugLog.txt.

	enum {
		//Games (for 'game' global)
		AUTODETECT,
		OBLIVION,
		FALLOUT3,
		FALLOUTNV,
		NEHRIM,
		SKYRIM,
		//BOSS Log Formats (for 'log_format' global)
		HTML,
		PLAINTEXT
	};

	//Command line variables.
	
	BOSS_COMMON_EXP extern string proxy_host;
	BOSS_COMMON_EXP extern string proxy_user;
	BOSS_COMMON_EXP extern string proxy_passwd;
	BOSS_COMMON_EXP extern unsigned int proxy_port;
	BOSS_COMMON_EXP extern unsigned int log_format;		// what format the output should be in.  Uses the enums defined above.
	BOSS_COMMON_EXP extern unsigned int game;				// What game's mods are we sorting? Uses the enums defined above.
	BOSS_COMMON_EXP extern unsigned int revert;				// what level to revert to
	BOSS_COMMON_EXP extern unsigned int debug_verbosity;			// log levels above INFO to output
	BOSS_COMMON_EXP extern bool update;				// update the masterlist?
	BOSS_COMMON_EXP extern bool update_only;		// only update the masterlist and don't sort currently.
	BOSS_COMMON_EXP extern bool silent;				// silent mode?
	BOSS_COMMON_EXP extern bool skip_version_parse; // enable parsing of mod's headers to look for version strings
	BOSS_COMMON_EXP extern bool debug_with_source;				// whether to include origin information in logging statements
	BOSS_COMMON_EXP extern bool show_CRCs;			// whether or not to show mod CRCs.
	BOSS_COMMON_EXP extern bool trial_run;			// If true, don't redate files.
	BOSS_COMMON_EXP extern bool log_debug_output;		//If true, logs command line output in BOSSDebugLog.txt.
	BOSS_COMMON_EXP extern bool do_startup_update_check;	// Whether or not to check for updates on startup.
	
	//GUI variables
	BOSS_COMMON_EXP extern unsigned int run_type;					// 1 = sort mods, 2 = only update, 3 = undo changes.
	BOSS_COMMON_EXP extern bool use_user_rules_editor;		//Use the User Rules Editor or edit userlist.txt directly?
}

#endif
