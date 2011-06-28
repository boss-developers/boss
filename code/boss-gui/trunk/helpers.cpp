/*	General User Interface for BOSS (Better Oblivion Sorting Software)
	
	Providing a graphical frontend to BOSS's functions.

    Copyright (C) 2011 WrinklyNinja & the BOSS development team.
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

//Header file for some functions that are helpful or required for the GUI to work,
//but not actually GUI-based.

#include "helpers.h"
#include <boost/spirit/include/karma.hpp>
#include <boost/algorithm/string.hpp>

namespace boss {
	using namespace std;
	namespace fs = boost::filesystem;
	namespace karma = boost::spirit::karma;

	//Version info.
	const string boss_version     = "1.6";				//TEMPORARY CHANGE TO TEST UPDATER
	const string boss_releaseDate = "June 10, 2011";

	//The run_type type decides on which variables are applied, not all are appropriate for all run_type types.
	int run_type			= 1;     // 1 = sort mods, 2 = only update, 3 = undo changes.
	
	//Command line variables
	string log_format		= "html";	// what format the output should be in.
	int game				= 0;		// What game's mods are we sorting? 1 = Oblivion, 2 = Fallout 3, 3 = Nehrim, 4 = Fallout: New Vegas.
	int revert              = 0;		// what level to revert to
	int verbosity           = 0;		// log levels above INFO to output
	bool update				= true;		// update the masterlist? THIS DOESN'T BEHAVE LIKE THE CLI PARAMETER - IF FALSE, IT SETS -U.
	bool silent             = false;	// silent mode?
	bool debug              = false;	// whether to include origin information in logging statements
	bool trial_run			= false;	// If true, don't redate files.
	bool sort_skip_version_parse   = false;	// enable parsing of mod's headers to look for version strings
	bool sort_show_CRCs			   = false;	// whether or not to show mod CRCs.
	bool revert_skip_version_parse = false;	// enable parsing of mod's headers to look for version strings
	bool revert_show_CRCs		   = false;	// whether or not to show mod CRCs.

	bool logCL				= false; // whether or not to log the command line output to BOSSDebugLog.txt.
	bool do_startup_update_check   = true;
	string proxy_type = "direct";
	string proxy_host = "none";
	string proxy_port = "0";

	//Returns the name of the game that BOSS is currently running for.
	string GetGame() {
		if (fs::exists("../Oblivion.exe")) {
			if (fs::exists("../Data/Nehrim.esm"))
				return "Nehrim";
			else
				return "Oblivion";
		} else if (fs::exists("../Fallout3.exe"))
			return "Fallout 3";
		else if (fs::exists("../FalloutNV.exe"))
			return "Fallout: New Vegas";
		else 
			return "Game Not Detected";
	}

	//Runs BOSS with arguments according to the settings of the run_type variables.
	void RunBOSS() {
		string params = "BOSS.exe";
		//Set format output params.
		if (silent)
			params += " -s";
		if (debug)
			params += " -d";
		if (verbosity > 0)
			params += " -v" + IntToString(verbosity);
		if (log_format == "text")
			params += " -f text";
		//Set run_type-dependent params.
		if (run_type == 1) {  //Sort mods.
			if (update)
				params += " -u";
			else
				params += " -U";
			if (sort_skip_version_parse)
				params += " -n";
			if (sort_show_CRCs)
				params += " -c";
			if (trial_run)
				params += " -t";
		} else if (run_type == 2) {  //Update masterlist only.
			params += " -o";
			if (game == 1)
				params += " -g Oblivion";
			else if (game == 2)
				params += " -g Fallout3";
			else if (game == 3)
				params += " -g Nehrim";
			else if (game == 4)
				params += " -g FalloutNV";
		} else if (run_type == 3) {  //Undo changes.
			if (revert > 0)
				params += " -r" + IntToString(revert);
			if (revert_skip_version_parse)
				params += " -n";
			if (revert_show_CRCs)
				params += " -c";
		}
		if (logCL)
			params += " > BOSSCommandLineLog.txt";
		//Now actually run_type BOSS.
		system(params.c_str());
		return;
	}

	//Opens the given file in the default system program.
	void OpenInSysDefault(fs::path& file) {
		string command =
#if _WIN32 || _WIN64
			"start ";
#else
			"xdg-open ";
#endif
		if (file.extension() == ".lnk\"" || file.extension() == ".html\"")
			command = "";
		command += file.string();
		system(command.c_str());
		return;
	}

	//Converts an integer to a string using BOOST's Spirit.Karma, which is apparently a lot faster than a stringstream conversion...
	string IntToString(unsigned int n) {
		string out;
		back_insert_iterator<string> sink(out);
		karma::generate(sink,karma::upper[karma::uint_],n);
		return out;
	}

	//Converts a boolean to a string representation (0/1)
	string BoolToString(bool b) {
		if (b)
			return "1";
		else
			return "0";
	}

	int versionStringToInt(string version) {
		boost::replace_all(version,".","");
		return atoi(version.c_str());
	}
}