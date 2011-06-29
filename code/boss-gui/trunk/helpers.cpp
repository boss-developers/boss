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
#include <boost/crc.hpp>

namespace boss {
	using namespace std;
	namespace fs = boost::filesystem;
	namespace karma = boost::spirit::karma;

	//Version info.
	const string boss_version     = "1.6";				//TEMPORARY CHANGE TO TEST UPDATER
	const string boss_releaseDate = "June 29, 2011";

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

	//Calculate the CRC of the given file for comparison purposes.
	unsigned int GetCrc32(const fs::path& filename) {
		int chksum = 0;
		static const size_t buffer_size = 8192;
		char buffer[buffer_size];
		ifstream ifile(filename.c_str(), ios::binary);
		boost::crc_32_type result;
		if (!ifile.fail()) {
			do {
				ifile.read(buffer, buffer_size);
				result.process_bytes(buffer, ifile.gcount());
			} while (ifile);
			chksum = result.checksum();
		}
        return chksum;
	}

	void GenerateIni() {
		ofstream ini("BOSS.ini");
		if (ini.fail())
			return;
		ini <<	"[BOSS.InternetSettings]" << endl
			<<	"# These settings control BOSS's (BOSS.exe and BOSS GUI.exe) proxy support." << endl
			<<	"# They are used when BOSS.exe runs directly, and when the GUI is run." << endl
			<<	"# There are no GUI equivalents for these settings." << endl
			<<	"# Valid values for UpdaterProxyType are 'direct', 'http', 'http1_0', 'socks4', 'socks4a', 'socks5', 'socks5h'." << endl
			<<	"# Valid values for UpdaterProxyHostname are 'none' and any valid hostname or IP address." << endl
			<<	"# Valid values for UpdaterProxyPort are 0 and any positive whole number." << endl
			<<	"ProxyType               = direct" << endl
			<<	"ProxyHostname           = none" << endl
			<<	"ProxyPort               = 0" << endl << endl

			<<	"[BOSS.RunOptions]" << endl
			<<	"# This section sets the options used when BOSS.exe is run directly." << endl
			<<	"# For toggles, 0 = not set, 1 = set." << endl
			<<	"# For RevertLevel and CommandLineVerbosity, values specify levels." << endl
			<<	"# Valid values for Game are 'auto', 'Oblivion', 'Nehrim', 'Fallout3', and 'FalloutNV'." << endl
			<<	"# Valid values for BOSSlogFormat are 'html' and 'text'." << endl
			<<	"UpdateMasterlist        = 1" << endl
			<<	"OnlyUpdateMasterlist    = 0" << endl
			<<	"DisableMasterlistUpdate = 0" << endl
			<<	"Game                    = auto" << endl
			<<	"SilentRun               = 0" << endl
			<<	"NoVersionParse          = 0" << endl
			<<	"RevertLevel             = 0" << endl
			<<	"CommandLineVerbosity    = 0" << endl
			<<	"Debug                   = 0" << endl
			<<	"DisplayCRCs             = 0" << endl
			<<	"BOSSlogFormat           = html" << endl
			<<	"DoTrialRun              = 0" << endl << endl

			<<	"[GUI.Settings]" << endl
			<<	"# These settings have no counterparts in the GUI itself and must be set here." << endl
			<<	"# DoStartupUpdateCheck is a toggle, 0 = not set, 1 = set." << endl
			<<	"DoStartupUpdateCheck    = 1" << endl << endl

			<<	"[GUI.LastOptions]" << endl
			<<	"# This section records the last configuration of options selected through the GUI." << endl
			<<	"# These settings are updated automatically when the GUI quits." << endl
			<<	"RunType                 = 1" << endl
			<<	"SilentRun               = 0" << endl
			<<	"Debug                   = 0" << endl
			<<	"LogCLOutput             = 0" << endl
			<<	"BOSSlogFormat           = html" << endl
			<<	"CLVerbosity             = 0" << endl
			<<	"UpdateMasterlist        = 1" << endl
			<<	"SortNoVersionParse      = 0" << endl
			<<	"SortDisplayCRCs         = 0" << endl
			<<	"DoTrialRun              = 0" << endl
			<<	"Game                    = auto" << endl
			<<	"RevertLevel             = 0" << endl
			<<	"RevertNoVersionParse    = 0" << endl
			<<	"RevertDisplayCRCs       = 0" << endl << endl

			<<	"[BOSSlog.Filters]" << endl
			<<	"# Settings below specify the default state of BOSSlog filters." << endl
			<<	"# 0 = unchecked by default, 1 = checked by default." << endl
			<<	"UseDarkColourScheme    = 0" << endl
			<<	"HideVersionNumbers     = 0" << endl
			<<	"HideGhostedLabel       = 0" << endl
			<<	"HideChecksums          = 0" << endl
			<<	"HideMessagelessMods    = 0" << endl
			<<	"HideGhostedMods        = 0" << endl
			<<	"HideRuleWarnings       = 0" << endl
			<<	"HideAllModMessages     = 0" << endl
			<<	"HideNotes              = 0" << endl
			<<	"HideBashTagSuggestions = 0" << endl
			<<	"HideRequirements       = 0" << endl
			<<	"HideIncompatibilities  = 0" << endl << endl

			<<	"[BOSSlog.Styles]" << endl
			<<	"# Below are the CSS styles responsible for how the BOSSlog looks." << endl
			<<	"# A style with nothing specified between the {} brackets uses the coded default." << endl
			<<	"# See the BOSSlog.html source code for the defaults, providing you haven't overwritten them here." << endl
			<<	"\"body\"                                     = {}" << endl
			<<	"\".filters\"                                 = {}" << endl
			<<	"\".filters > li\"                            = {}" << endl
			<<	"\"body > div:first-child\"                   = {}" << endl
			<<	"\"body > div:first-child + div\"             = {}" << endl
			<<	"\"body > div\"                               = {}" << endl
			<<	"\"body > div > span:first-child\"            = {}" << endl
			<<	"\"body > div > span:first-child > span\"     = {}" << endl
			<<	"\"div > ul\"                                 = {}" << endl
			<<	"\"body > div:last-child\"                    = {}" << endl
			<<	"\"body > div:last-child > span:first-child\" = {}" << endl
			<<	"\"div > ul > li\"                            = {}" << endl
			<<	"\"ul\"                                       = {}" << endl
			<<	"\"ul li\"                                    = {}" << endl
			<<	"\"li ul\"                                    = {}" << endl
			<<	"\"input[type='checkbox']\"                   = {}" << endl
			<<	"\"blockquote\"                               = {}" << endl
			<<	"\"#unrecognised > li\"                       = {}" << endl
			<<	"\"#summary > div\"                           = {}" << endl
			<<	"\"#summary > div > div\"                     = {}" << endl
			<<	"\".error\"                                   = {}" << endl
			<<	"\".warn\"                                    = {}" << endl
			<<	"\".success\"                                 = {}" << endl
			<<	"\".version\"                                 = {}" << endl
			<<	"\".ghosted\"                                 = {}" << endl
			<<	"\".crc\"                                     = {}" << endl
			<<	"\".tagPrefix\"                               = {}" << endl
			<<	"\".dirty\"                                   = {}" << endl
			<<	"\".message\"                                 = {}" << endl
			<<	"\".mod\"                                     = {}" << endl
			<<	"\".tag\"                                     = {}" << endl
			<<	"\".note\"                                    = {}" << endl
			<<	"\".req\"                                     = {}" << endl
			<<	"\".inc\"                                     = {}";
	}
}