/*	BOSS

	A "one-click" program for users that quickly optimises and avoids
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge,
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

    Copyright (C) 2009-2011    BOSS Development Team.

	This file is part of BOSS.

    BOSS is free software: you can redistribute
	it and/or modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

    BOSS is distributed in the hope that it will
	be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with BOSS.  If not, see
	<http://www.gnu.org/licenses/>.

	$Revision: 1783 $, $Date: 2010-10-31 23:05:28 +0000 (Sun, 31 Oct 2010) $
*/

#define NOMINMAX // we don't want the dummy min/max macros since they overlap with the std:: algorithms

#include "BOSS-Common.h"

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem/detail/utf8_codecvt_facet.hpp>
#include <boost/exception/get_error_info.hpp>
#include <boost/unordered_set.hpp>
#include <boost/locale.hpp>

#include <clocale>
#include <cstdlib>
#include <ctime>
#include <string>
#include <iostream>
#include <algorithm>

#include <boost/regex.hpp>


using namespace boss;
using namespace std;
namespace po = boost::program_options;
using boost::locale::translate;

#if _WIN32 || _WIN64
	const string launcher_cmd = "start";
#else
	const string launcher_cmd = "xdg-open";
#endif

int Launch(string filename) {
	if (filename.find(' ') != string::npos) {
#if _WIN32 || _WIN64
		filename = "\"\" \"" + filename + '"';  //Need "" "filename" because the first quoted string is the CLI window title and the second is the thing started...
#else
		boost::replace_all(filename, " ", "\\ ");
#endif
	}
	const string cmd = launcher_cmd + " " + filename;
	return ::system(cmd.c_str());
}


void ShowVersion() {
	cout << "BOSS" << endl;
	cout << translate("Version ") << BOSS_VERSION_MAJOR << "." << BOSS_VERSION_MINOR << "." << BOSS_VERSION_PATCH << endl;
}

void ShowUsage(po::options_description opts) {

	static string progName =
#if _WIN32 || _WIN64
		"BOSS";
#else
		"boss";
#endif

	ShowVersion();
	cout << endl << translate("Description:") << endl
	 << translate("  BOSS is a utility that sorts the mod load order of TESIV: Oblivion, Nehrim,"
				  "  Fallout 3, Fallout: New Vegas and TESV: Skyrim according to a frequently updated"
				  "  masterlist to minimise incompatibilities between mods.") << endl << endl
	 << opts << endl
	 << translate("Examples:") << endl
	 << "  " << progName << " -u" << endl
	 << translate("    updates the masterlist, sorts your mods, and shows the log") << endl << endl
	 << "  " << progName << " -sr" << endl
	 << translate("    reverts your load order 1 level and skips showing the log") << endl << endl
	 << "  " << progName << " -r 2" << endl
	 << translate("    reverts your load order 2 levels and shows the log") << endl << endl;
}

void Fail() {
	cout << "Press ENTER to quit...";
	cin.clear();
	cin.ignore(1, '\n');
	cin.get();
	exit(1);
}

class CLIMlistUpdater : public MasterlistUpdater {
protected:
	int progress(Updater * updater, double dlFraction, double dlTotal) {
		printf((translate("Downloading: %s; %3.0f%% of %3.0f KB").str() + "\r").c_str(), updater->TargetFile().c_str(), dlFraction, dlTotal);  //The +20 is there because for some reason there's always a 20kb difference between reported size and Windows' size.
		fflush(stdout);
		return 0;
	}
};

class CLIBOSSUpdater : public BOSSUpdater {
protected:
	int progress(Updater * updater, double dlFraction, double dlTotal) {
		printf((translate("Downloading: %s; %3.0f%% of %3.0f KB").str() + "\r").c_str(), updater->TargetFile().c_str(), dlFraction, dlTotal);  //The +20 is there because for some reason there's always a 20kb difference between reported size and Windows' size.
		fflush(stdout);
		return 0;
	}
};

int main(int argc, char *argv[]) {
	Settings ini;
	Game game;
	string gameStr;							// allow for autodetection override
	string bosslogFormat;
	fs::path sortfile;						//modlist/masterlist to sort plugins using.


	///////////////////////////////
	// Set up initial conditions
	///////////////////////////////

	LOG_INFO("BOSS starting...");

	LOG_INFO("Parsing Ini...");
	//Parse ini file if found. Can't just use BOOST's program options ini parser because of the CSS syntax and spaces.
	if (fs::exists(ini_path)) {
		try {
			ini.Load(ini_path);
		} catch (boss_error &e) {
			LOG_ERROR("Error: %s", e.getString().c_str());
			//Error will be added to log once format has been set.
		}
	} else {
		try {
			ini.Save(ini_path, AUTODETECT);
		} catch (boss_error &e) {
			ini.ErrorBuffer(ParsingError("Error: " + e.getString()));
		}
	}

	//Specify location of language dictionaries
	boost::locale::generator gen;
	gen.add_messages_path(fs::path(boss_path / "l10n").string());
	gen.add_messages_domain("messages");

	//Set the locale to get encoding and language conversions working correctly.
	string localeId = "";
	if (gl_language == ENGLISH)
		localeId = "en.UTF-8";
	else if (gl_language == SPANISH)
		localeId = "es.UTF-8";
	else if (gl_language == GERMAN)
		localeId = "de.UTF-8";
	else if (gl_language == RUSSIAN)
		localeId = "ru.UTF-8";
	else if (gl_language == SIMPCHINESE)
		localeId = "zh.UTF-8";

	try {
		locale::global(gen(localeId));
		cout.imbue(locale());
	} catch(exception &e) {
		LOG_ERROR("could not implement translation: %s", e.what());
		cout << e.what() << endl;
	}
	locale global_loc = locale();
	locale loc(global_loc, new boost::filesystem::detail::utf8_codecvt_facet());
	boost::filesystem::path::imbue(loc);

	//////////////////////////////
	// Handle Command Line Args
	//////////////////////////////

	// declare the supported options
	po::options_description opts("Options");
	opts.add_options()
		("help,h",				translate("produces this help message").str().c_str())
		("version,V",			translate("prints the version banner").str().c_str())
		("update,u", po::value(&gl_update)->zero_tokens(),
								translate("automatically update the local copy of the"
								" masterlist to the latest version"
								" available on the web before sorting").str().c_str())
		("no-update,U",			translate("inhibit the automatic masterlist updater").str().c_str())
		("only-update,o", po::value(&gl_update_only)->zero_tokens(),
								translate("automatically update the local copy of the"
								" masterlist to the latest version"
								" available on the web but don't sort right"
								" now").str().c_str())
		("silent,s", po::value(&gl_silent)->zero_tokens(),
								translate("don't launch a browser to show the HTML log"
								" at program completion").str().c_str())
		("revert,r", po::value(&gl_revert)->implicit_value(1, ""),
								translate("revert to a previous load order.  this"
								" parameter optionally accepts values of 1 or"
								" 2, indicating how many undo steps to apply."
								"  if no option value is specified, it"
								" defaults to 1").str().c_str())
		("verbose,v", po::value(&gl_debug_verbosity)->implicit_value(1, ""),
								translate("specify verbosity level (0-3) of the debugging output.  0 is the"
								" default, showing only WARN and ERROR messges."
								" 1 (INFO and above) is implied if this option"
								" is specified without an argument.  higher"
								" values increase the verbosity further").str().c_str())
		("game,g", po::value(&gameStr),
								translate("override game autodetection.  valid values"
								" are: 'Oblivion', 'Nehrim', 'Fallout3',"
								" 'FalloutNV', and 'Skyrim'").str().c_str())
		("debug-with-source,d", po::value(&gl_debug_with_source)->zero_tokens(),
								translate("add source file references to debug statements").str().c_str())
		("crc-display,c", po::value(&gl_show_CRCs)->zero_tokens(),
								translate("show mod file CRCs, so that a file's CRC can be"
								" added to the masterlist in a conditional").str().c_str())
		("format,f", po::value(&bosslogFormat),
								translate("select output format. valid values"
								" are: 'html', 'text'").str().c_str())
		("trial-run,t", po::value(&gl_trial_run)->zero_tokens(),
								translate("run BOSS without actually making any changes to load order").str().c_str())
		("proxy-host,H", po::value(&gl_proxy_host),
								translate("sets the proxy hostname for the masterlist updater").str().c_str())
		("proxy-port,P", po::value(&gl_proxy_port),
								translate("sets the proxy port number for the masterlist updater").str().c_str())
		("log-debug,l", po::value(&gl_log_debug_output)->zero_tokens(),
								translate("logs the debug output to the BOSSDebugLog.txt file instead"
								" of the command line.").str().c_str());

	// parse command line arguments
	po::variables_map vm;
	try{
		po::store(po::command_line_parser(argc, argv).options(opts).run(), vm);
		po::notify(vm);
	}catch (po::multiple_occurrences &){
		LOG_ERROR("cannot specify options multiple times; please use the '--help' option to see usage instructions");
		Fail();
	}catch (exception & e){
		LOG_ERROR("%s; please use the '--help' option to see usage instructions", e.what());
		Fail();
	}

	// set alternative output stream for logger and whether to track log statement origins
	if (gl_log_debug_output)
		g_logger.setStream(debug_log_path.string().c_str());
	g_logger.setOriginTracking(gl_debug_with_source);
	if (gl_debug_verbosity < 0) {
		LOG_ERROR("invalid option for 'verbose' parameter: %d", gl_debug_verbosity);
		Fail();
	}
	g_logger.setVerbosity(static_cast<LogVerbosity>(LV_WARN + gl_debug_verbosity));  // it's ok if this number is too high.  setVerbosity will handle it

	if (vm.count("help")) {
		ShowUsage(opts);
		exit(0);
	}
	if (vm.count("version")) {
		ShowVersion();
		exit(0);
	}
	if (vm.count("no-update")) {
		gl_update = false;
	}
	if ((vm.count("update")) && (vm.count("no-update"))) {
		LOG_ERROR("invalid options: --update,-u and --no-update,-U cannot both be given.");
		Fail();
	}
	if (vm.count("revert") && (gl_revert < 1 || gl_revert > 2)) {
		LOG_ERROR("invalid option for 'revert' parameter: %d", gl_revert);
		Fail();
	}
	if (vm.count("game")) {
		// sanity check and parse argument
		if      (boost::iequals("Oblivion",   gameStr))
			gl_game = OBLIVION;
		else if (boost::iequals("Fallout3",   gameStr))
			gl_game = FALLOUT3;
		else if (boost::iequals("Nehrim",     gameStr))
			gl_game = NEHRIM;
		else if (boost::iequals("FalloutNV", gameStr))
			gl_game = FALLOUTNV;
		else if (boost::iequals("Skyrim", gameStr))
			gl_game = SKYRIM;
		else if (boost::iequals("Morrowind", gameStr))
			gl_game = MORROWIND;
		else {
			LOG_ERROR("invalid option for 'game' parameter: '%s'", gameStr.c_str());
			Fail();
		}
		LOG_DEBUG("game ini setting overridden with: '%s' (%d)", gameStr.c_str(), gl_game);
	}
	if (vm.count("format")) {
		// sanity check and parse argument
		string bosslogFormat = vm["format"].as<string>();
		if (bosslogFormat == "html")
			gl_log_format = HTML;
		else if (bosslogFormat == "text")
			gl_log_format = PLAINTEXT;
		else {
			LOG_ERROR("invalid option for 'format' parameter: '%s'", bosslogFormat.c_str());
			Fail();
		}
		LOG_DEBUG("BOSSlog format set to: '%s'", bosslogFormat.c_str());
	}


	/////////////////////////
	// BOSS Updater Stuff
	/////////////////////////

	if (gl_do_startup_update_check) {
		string updateText, updateVersion;
		CLIBOSSUpdater BossUpdater;
		try {
			if (BossUpdater.IsInternetReachable()) {
				cout << translate("Checking for BOSS updates...") << endl;
				LOG_DEBUG("Checking for BOSS updates...");
				try {
					updateVersion = BossUpdater.IsUpdateAvailable();
					if (updateVersion.empty()) {
						cout << translate("You are already using the latest version of BOSS.") << endl;
						LOG_DEBUG("You are already using the latest version of BOSS.");
					} else {
						//First detect type of current install: manual or installer.
						if (!RegKeyExists("HKEY_LOCAL_MACHINE", "Software\\BOSS", "Installed Path")) {  //Manual.
							//Point user to download locations.
							cout << translate("Update available! New version: %s") << endl
								 << translate("The update may be downloaded from any of the locations listed in the BOSS Log.") << endl;
							LOG_DEBUG("Update available! New version: %s", updateVersion);
							LOG_DEBUG("The update may be downloaded from any of the locations listed in the BOSS Log.");
						} else {  //Installer
							cout << translate("Update available! New version: %s") << endl;
							LOG_DEBUG("Update available! New version: %s", updateVersion);
							string notes;
							try {
								notes = BossUpdater.FetchReleaseNotes(updateVersion);
							} catch (boss_error &e) {
								LOG_ERROR("Failed to get release notes. Details: '%s'", e.getString().c_str());
							}
							if (!notes.empty())
								cout << translate("Release notes:") << endl << endl << notes << endl << endl;
							cout << translate("Do you want to download and install the update? (y/n)") << endl;

							//Does the user want to update?
							char answer;
							cin >> answer;
							if (answer == 'n') {
								cout << translate("No update has been downloaded or installed.") << endl;
								LOG_DEBUG("No update has been downloaded or installed.");
							} else if (answer == 'y') {
								try {
									string file = "BOSS Installer.exe";
									BossUpdater.GetUpdate(fs::path(file), updateVersion);

									cout << endl << translate("New installer successfully downloaded!") << endl
										 << translate("BOSS will now launch the downloaded installer and exit. Complete the installer to complete the update.") << endl << endl;
									if (fs::exists(file))
										Launch(file);
									Fail();
								} catch (boss_error &e) {
									try {
										BossUpdater.CleanUp();
									} catch (boss_error &ee) {
										if (e.getCode() != BOSS_ERROR_CURL_USER_CANCEL)
											LOG_ERROR("Update failed. Details: '%s'", e.getString().c_str());
										LOG_ERROR("Update clean up failed. Details: '%s'", ee.getString().c_str());
										Fail();
									}
									if (e.getCode() == BOSS_ERROR_CURL_USER_CANCEL) {
										cout << translate("Update cancelled.") << endl;
										LOG_DEBUG("Update cancelled.");
									} else {
										LOG_ERROR("Update failed. Details: '%s'", e.getString().c_str());
										Fail();
									}
								}
							} else {
								LOG_ERROR("invalid option given: '%s'", answer);
								Fail();
							}
						}
					}
				} catch (boss_error &e) {
					LOG_ERROR("BOSS Update check failed. Details: '%s'", e.getString().c_str());
				}
			} else {
				cout << translate("BOSS Update check failed. No Internet connection detected.") << endl;
				LOG_DEBUG("BOSS Update check failed. No Internet connection detected.");
			}
		} catch (boss_error &e) {
			LOG_ERROR("Update check failed. Details: '%s'", e.getString().c_str());
		}
	}

	/////////////////////////////////////////
	// Check for critical error conditions
	/////////////////////////////////////////

	//Game checks.
	LOG_DEBUG("Detecting game...");
	try {
		gl_last_game = AUTODETECT;  //Clear this setting in case the GUI was run.
		vector<uint32_t> detected, undetected;
		uint32_t detectedGame = DetectGame(detected, undetected);
		if (detectedGame == AUTODETECT) {
			//Now check what games were found.
			if (detected.empty())
				throw boss_error(BOSS_ERROR_NO_GAME_DETECTED);
			else if (detected.size() == 1)
				detectedGame = detected.front();
			else {
				size_t ans;
				//Ask user to choose game.
				cout << endl << translate("Please pick which game to run BOSS for:") << endl;
				for (size_t i=0; i < detected.size(); i++)
					cout << i << " : " << Game(detected[i], "", true).Name() << endl;

				cin >> ans;
				if (ans < 0 || ans >= detected.size()) {
					cout << translate("Invalid selection.") << endl;
					throw boss_error(BOSS_ERROR_NO_GAME_DETECTED);
				}
				detectedGame = detected[ans];
			}
		}
		game = Game(detectedGame);
		game.CreateBOSSGameFolder();
		LOG_INFO("Game detected: %s", game.Name().c_str());
	} catch (boss_error &e) {
		LOG_ERROR("Critical Error: %s", e.getString().c_str());
		game.bosslog.SetFormat(gl_log_format);
		game.bosslog.criticalError << LIST_ITEM_CLASS_ERROR << translate("Critical Error: ") << e.getString() << LINE_BREAK
			<< translate("Check the Troubleshooting section of the ReadMe for more information and possible solutions.") << LINE_BREAK
			<< translate("Utility will end now.");
		try {
			game.bosslog.Save(game.Log(gl_log_format), true);
		} catch (boss_error &e) {
			LOG_ERROR("Critical Error: %s", e.getString().c_str());
		}
		if ( !gl_silent )
			Launch(game.Log(gl_log_format).string());	//Displays the BOSSlog.txt.
		exit (1); //fail in screaming heap.
	}
	game.bosslog.SetFormat(gl_log_format);
	game.bosslog.parsingErrors.push_back(ini.ErrorBuffer());


	/////////////////////////////////////////////////////////
	// Update masterlist
	/////////////////////////////////////////////////////////

	if (gl_revert < 1 && (gl_update || gl_update_only)) {
		//First check for internet connection, then update masterlist if connection present.
		CLIMlistUpdater MlistUpdater;
		try {
			if (MlistUpdater.IsInternetReachable()) {
				cout << endl << translate("Updating to the latest masterlist from the online repository...") << endl;
				LOG_DEBUG("Updating masterlist...");
				try {
					string localDate, remoteDate, message;
					uint32_t localRevision, remoteRevision;
					MlistUpdater.Update(game.Id(), game.Masterlist(), localRevision, localDate, remoteRevision, remoteDate);
					if (localRevision == remoteRevision) {
						message = (boost::format(translate("Your masterlist is already at the latest revision (r%1%; %2%). No update necessary.")) % localRevision % localDate).str();
						LOG_DEBUG("masterlist update unnecessary.");
					} else {
						message =  (boost::format(translate("Your masterlist has been updated to revision %1% (%2%).")) % remoteRevision % remoteDate).str();
						LOG_DEBUG("masterlist updated successfully.");
					}
					game.bosslog.updaterOutput << LIST_ITEM_CLASS_SUCCESS << message;
					cout << endl << message << endl;
				} catch (boss_error &e) {
					game.bosslog.updaterOutput << LIST_ITEM_CLASS_ERROR << translate("Error: masterlist update failed.") << LINE_BREAK
						<< (boost::format(translate("Details: %1%")) % e.getString()).str() << LINE_BREAK
						<< translate("Check the Troubleshooting section of the ReadMe for more information and possible solutions.");
					LOG_ERROR("Error: masterlist update failed. Details: %s", e.getString().c_str());
				}
			} else {
				game.bosslog.updaterOutput << LIST_ITEM_CLASS_WARN << translate("No internet connection detected. Masterlist auto-updater could not check for updates.");
			}
		} catch (boss_error &e) {
			game.bosslog.updaterOutput << LIST_ITEM_CLASS_ERROR << translate("Error: masterlist update failed.") << LINE_BREAK
				<< (boost::format(translate("Details: %1%")) % e.getString()).str() << LINE_BREAK
				<< translate("Check the Troubleshooting section of the ReadMe for more information and possible solutions.");
			LOG_ERROR("Error: masterlist update failed. Details: %s", e.getString().c_str());
		}
	}

	//If true, exit BOSS now. Flush earlyBOSSlogBuffer to the bosslog and exit.
	if (gl_update_only) {
		try {
			game.bosslog.Save(game.Log(gl_log_format), true);
		} catch (boss_error &e) {
			LOG_ERROR("Critical Error: %s", e.getString().c_str());
		}
		if ( !gl_silent )
			Launch(game.Log(gl_log_format).string());	//Displays the BOSSlog.
		return (0);
	}


	///////////////////////////////////
	// Resume Error Condition Checks
	///////////////////////////////////

	cout << endl << translate("BOSS working...") << endl;

	//Build and save modlist.
	try {
		game.modlist.Load(game, game.DataFolder());
		if (gl_revert < 1)
			game.modlist.Save(game.Modlist(), game.OldModlist());
	} catch (boss_error &e) {
		LOG_ERROR("Failed to load/save modlist, error was: %s", e.getString().c_str());
		game.bosslog.criticalError << LIST_ITEM_CLASS_ERROR << (boost::format(translate("Critical Error: %1%")) % e.getString()).str() << LINE_BREAK
			<< translate("Check the Troubleshooting section of the ReadMe for more information and possible solutions.") << LINE_BREAK
			<< translate("Utility will end now.");
		try {
			game.bosslog.Save(game.Log(gl_log_format), true);
		} catch (boss_error &e) {
			LOG_ERROR("Critical Error: %s", e.getString().c_str());
		}
		if ( !gl_silent )
			Launch(game.Log(gl_log_format).string());	//Displays the BOSSlog.txt.
		exit (1); //fail in screaming heap.
	}


	/////////////////////////////////
	// Parse Master- and Userlists
	/////////////////////////////////
	//masterlist parse errors are critical, ini and userlist parse errors are not.

	//Set masterlist path to be used.
	if (gl_revert == 1)
		sortfile = game.Modlist();
	else if (gl_revert == 2)
		sortfile = game.OldModlist();
	else
		sortfile = game.Masterlist();
	LOG_INFO("Using sorting file: %s", sortfile.string().c_str());

	//Parse masterlist/modlist backup into data structure.
	try {
		LOG_INFO("Starting to parse sorting file: %s", sortfile.string().c_str());
		game.masterlist.Load(game, sortfile);
		LOG_INFO("Starting to parse conditionals from sorting file: %s", sortfile.string().c_str());
		game.masterlist.EvalConditions(game);
		game.masterlist.EvalRegex(game);
		game.bosslog.globalMessages = game.masterlist.GlobalMessageBuffer();
		game.bosslog.parsingErrors.push_back(game.masterlist.ErrorBuffer());
	} catch (boss_error &e) {
		LOG_ERROR("Critical Error: %s", e.getString().c_str());
        if (e.getCode() == BOSS_ERROR_FILE_PARSE_FAIL)
			game.bosslog.criticalError << game.masterlist.ErrorBuffer();
		else if (e.getCode() == BOSS_ERROR_CONDITION_EVAL_FAIL)
			game.bosslog.criticalError << LIST_ITEM_CLASS_ERROR << e.getString();
		else
			game.bosslog.criticalError << LIST_ITEM_CLASS_ERROR << (boost::format(translate("Critical Error: %1%")) % e.getString()).str() << LINE_BREAK
				<< translate("Check the Troubleshooting section of the ReadMe for more information and possible solutions.") << LINE_BREAK
				<< translate("Utility will end now.");
		try {
			game.bosslog.Save(game.Log(gl_log_format), true);
		} catch (boss_error &e) {
			LOG_ERROR("Critical Error: %s", e.getString().c_str());
		}
		if ( !gl_silent )
                Launch(game.Log(gl_log_format).string());  //Displays the BOSSlog.txt.
        exit (1); //fail in screaming heap.
	}

	LOG_INFO("Starting to parse userlist.");
	try {
		game.userlist.Load(game, game.Userlist());
		vector<ParsingError> errs = game.userlist.ErrorBuffer();
		game.bosslog.parsingErrors.insert(game.bosslog.parsingErrors.end(), errs.begin(), errs.end());
	} catch (boss_error &e) {
		vector<ParsingError> errs = game.userlist.ErrorBuffer();
		game.bosslog.parsingErrors.insert(game.bosslog.parsingErrors.end(), errs.begin(), errs.end());
		game.userlist.Clear();  //If userlist has parsing errors, empty it so no rules are applied.
		LOG_ERROR("Error: %s", e.getString().c_str());
	}

	//////////////////////////////////
	// Perform sorting functionality
	//////////////////////////////////

	try {
		game.ApplyMasterlist();
		LOG_INFO("masterlist now filled with ordered mods and modlist filled with unknowns.");
		game.ApplyUserlist();
		LOG_INFO("userlist sorting process finished.");
		game.ScanSEPlugins();
		game.SortPlugins();
		game.bosslog.Save(game.Log(gl_log_format), true);
	} catch (boss_error &e) {
		LOG_ERROR("Critical Error: %s", e.getString().c_str());
		game.bosslog.criticalError << LIST_ITEM_CLASS_ERROR << (boost::format(translate("Critical Error: %1%")) % e.getString()).str() << LINE_BREAK
			<< translate("Check the Troubleshooting section of the ReadMe for more information and possible solutions.") << LINE_BREAK
			<< translate("Utility will end now.");
		try {
			game.bosslog.Save(game.Log(gl_log_format), true);
		} catch (boss_error &e) {
			LOG_ERROR("Critical Error: %s", e.getString().c_str());
		}
		if ( !gl_silent )
                Launch(game.Log(gl_log_format).string());  //Displays the BOSSlog.txt.
        exit (1); //fail in screaming heap.
	}

	LOG_INFO("Launching boss log in browser.");
	if ( !gl_silent )
		Launch(game.Log(gl_log_format).string());	//Displays the BOSSlog.txt.
	LOG_INFO("BOSS finished.");
	return (0);
}