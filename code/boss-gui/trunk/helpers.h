/*	General User Interface for BOSS (Better Oblivion Sorting Software)
	
	Providing a graphical frontend to BOSS's functions.

    Copyright (C) 2011 WrinklyNinja & the BOSS development team.
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

//Header file for some functions that are helpful or required for the GUI to work,
//but not actually GUI-based.

/*Libraries to include: 
For wxWidgets:
wxmsw29u_core.lib
wxbase29u.lib
comctl32.lib
rpcrt4.lib

For Curl:
libcurl.lib
wldap32.lib
ws2_32.lib

Other:
Version.lib
*/

#ifndef __HELPERS__HPP__
#define __HELPERS__HPP__

#ifndef _UNICODE
#define _UNICODE	// Tell compiler we're using Unicode, notice the _
#endif

#include <string>
#include "boost/filesystem.hpp"

namespace boss {
	using namespace std;
	namespace fs = boost::filesystem;

	//Version info.
	extern const string boss_version;
	extern const string boss_releaseDate;

	//Run type
	//The run_type decides on which variables are applied, not all are appropriate for all run_types.
	extern int run_type;		// 1 = sort mods, 2 = only update, 3 = undo changes.
	
	//Command line variables
	extern string log_format;		// what format the output should be in.
	extern int game;			// Force what game? 0 = allow autodetection, 1 = Oblivion, 2 = Fallout 3, 3 = Nehrim, 4 = Fallout: New Vegas.
	extern int revert;			// what level to revert to
	extern int verbosity;		// Command-line output verbosity.
	extern bool update;			// update the masterlist? THIS DOESN'T BEHAVE LIKE THE CLI PARAMETER - IF FALSE, IT SETS -U.
	extern bool silent;				// silent mode?
	extern bool debug;				// whether to include origin information in logging statements
	extern bool trial_run;			// If true, don't redate files.
	extern bool sort_skip_version_parse; // enable parsing of mod's headers to look for version strings
	extern bool sort_show_CRCs;			// whether or not to show mod CRCs.
	extern bool revert_skip_version_parse; // enable parsing of mod's headers to look for version strings
	extern bool revert_show_CRCs;			// whether or not to show mod CRCs.

	extern bool logCL;			// whether or not to log the command line output to BOSSDebugLog.txt.

	//Returns the name of the game that BOSS is currently running for.
	string GetGame();

	//Runs BOSS with arguments according to the settings of the run_type variables.
	void RunBOSS();

	//Opens the given file in the default system program.
	void OpenInSysDefault(fs::path& file);

	//Converts an integer to a string using BOOST's Spirit.Karma. Faster than a stringstream conversion.
	string IntToString(unsigned int n);

	//Converts a boolean to a string representation (0/1)
	string BoolToString(bool b);

	//Strips the decimal points from a version number to obtain it as an integer. Trailing zeros will cause comparisons to return incorrect values.
	int versionStringToInt(string version);
}
#endif