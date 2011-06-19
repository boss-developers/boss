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

	const string g_version;
	const string g_releaseDate;

	//Command line variables.
	int game;				//What game's mods are we sorting? 1 = Oblivion, 2 = Fallout 3, 3 = Nehrim, 4 = Fallout: New Vegas.
	bool update;			// update masterlist?
	bool update_only;		// only update the masterlist and don't sort currently.
	bool silent;			// silent mode?
	bool skip_version_parse; // enable parsing of mod's headers to look for version strings
	int revert;				// what level to revert to
	int verbosity;			// log levels above INFO to output
	bool debug;				// whether to include origin information in logging statements
	bool show_CRCs;			// whether or not to show mod CRCs.
	string format;			// what format the output should be in.
	bool trial_run;			//If true, don't redate files.
}

#endif
