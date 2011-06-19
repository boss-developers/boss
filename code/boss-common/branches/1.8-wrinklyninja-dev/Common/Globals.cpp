/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

	Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
	http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision$, $Date$
*/

#include "Globals.h"

namespace boss {
	using namespace std;

	ofstream bosslog;					//BOSSlog.txt - output file.

	const fs::path data_path			= fs::path("..") / "Data";
	const fs::path bosslog_html_path	= "BOSSlog.html";
	const fs::path bosslog_text_path	= "BOSSlog.txt";
	const fs::path masterlist_path		= "masterlist.txt";
	const fs::path userlist_path		= "userlist.txt";
	const fs::path curr_modlist_path	= "modlist.txt";
	const fs::path prev_modlist_path	= "modlist.old";

	const string g_version     = "1.7";
	const string g_releaseDate = "June 10, 2011";

	//Command line variables
	int game				= 0;		//What game's mods are we sorting? 1 = Oblivion, 2 = Fallout 3, 3 = Nehrim, 4 = Fallout: New Vegas.
	bool update             = false; // update masterlist?
	bool update_only         = false; // only update the masterlist and don't sort currently.
	bool silent             = false; // silent mode?
	bool skip_version_parse = false; // enable parsing of mod's headers to look for version strings
	int revert              = 0;     // what level to revert to
	int verbosity           = 0;     // log levels above INFO to output
	bool debug              = false; // whether to include origin information in logging statements
	bool show_CRCs			= false; // whether or not to show mod CRCs.
	string format			= "html";  // what format the output should be in.
	bool trial_run			= false; //If true, don't redate files.
}