/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

	Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
	http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision$, $Date$
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

	extern const fs::path data_path;			// Holds the path to the data directory.
	extern const fs::path bosslog_html_path;	// BOSSlog full HTML file name
	extern const fs::path bosslog_text_path;	// BOSSlog full text file name
	extern const fs::path masterlist_path;		// Hold both location and file name for masterlist.txt
	extern const fs::path userlist_path;		// Hold both location and file name for userlist.txt 
	extern const fs::path curr_modlist_path;	// Hold both location and file name for modlist.txt
	extern const fs::path prev_modlist_path;	// Hold both location and file name for modlist.old

	extern const string g_version;
	extern const string g_releaseDate;

	//Command line variables.
	extern string log_format;		// what format the output should be in.
	extern string proxy_type;
	extern string proxy_host;
	extern string proxy_port;
	extern int game;				// What game's mods are we sorting? 1 = Oblivion, 2 = Fallout 3, 3 = Nehrim, 4 = Fallout: New Vegas, 5 = Skyrim.
	extern int revert;				// what level to revert to
	extern int verbosity;			// log levels above INFO to output
	extern bool update;				// update the masterlist?
	extern bool update_only;		// only update the masterlist and don't sort currently.
	extern bool silent;				// silent mode?
	extern bool skip_version_parse; // enable parsing of mod's headers to look for version strings
	extern bool debug;				// whether to include origin information in logging statements
	extern bool show_CRCs;			// whether or not to show mod CRCs.
	extern bool trial_run;			// If true, don't redate files.
}

#endif
