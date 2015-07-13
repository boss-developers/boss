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

#define NOMINMAX  // We don't want the dummy min/max macros since they overlap with the std:: algorithms

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include <exception>
#include <iostream>
#include <locale>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/exception/get_error_info.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/detail/utf8_codecvt_facet.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <boost/program_options.hpp>

#include <git2.h>

#include "common/error.h"
#include "common/game.h"
#include "common/globals.h"
#include "common/settings.h"
#include "output/output.h"
#include "support/logger.h"
#include "updating/updater.h"

using namespace boss;  // MCP Note: Temporary solution, need to come up with a better one.
namespace fs = boost::filesystem;
namespace po = boost::program_options;
namespace loc = boost::locale;  // MCP Note: Will this interfere with std::locale loc?

#if _WIN32 || _WIN64
	const std::string launcher_cmd = "start";
#else
	const std::string launcher_cmd = "xdg-open";
#endif

int Launch(std::string filename) {
	if (filename.find(' ') != std::string::npos) {
#if _WIN32 || _WIN64
		filename = "\"\" \"" + filename + '"';  // Need "" "filename" because the first quoted string is the CLI window title and the second is the thing started...
#else
		boost::replace_all(filename, " ", "\\ ");
#endif
	}
	const std::string cmd = launcher_cmd + " " + filename;
	return std::system(cmd.c_str());
}


void ShowVersion() {
	std::cout << "BOSS" << std::endl;
	std::cout << loc::translate("Version ") << BOSS_VERSION_MAJOR << "." << BOSS_VERSION_MINOR << "." << BOSS_VERSION_PATCH << std::endl;
}

void ShowUsage(po::options_description opts) {
	static std::string progName =
#if _WIN32 || _WIN64
	    "BOSS";
#else
	    "boss";
#endif

	ShowVersion();
	std::cout << std::endl << loc::translate("Description:") << std::endl
	          << loc::translate("  BOSS is a utility that sorts the mod load order of TESIV: Oblivion, Nehrim,"
	                            "  Fallout 3, Fallout: New Vegas and TESV: Skyrim according to a frequently updated"
	                            "  masterlist to minimise incompatibilities between mods.") << std::endl << std::endl
	          << opts << std::endl
	          << loc::translate("Examples:") << std::endl
	          << "  " << progName << " -u" << std::endl
	          << loc::translate("    updates the masterlist, sorts your mods, and shows the log") << std::endl << std::endl
	          << "  " << progName << " -sr" << std::endl
	          << loc::translate("    reverts your load order 1 level and skips showing the log") << std::endl << std::endl
	          << "  " << progName << " -r 2" << std::endl
	          << loc::translate("    reverts your load order 2 levels and shows the log") << std::endl << std::endl;
}

void Fail() {
	std::cout << "Press ENTER to quit...";
	std::cin.clear();
	std::cin.ignore(1, '\n');
	std::cin.get();
	std::exit(1);
}

int progress(const git_transfer_progress *stats, void *payload) {
	std::printf((loc::translate("Downloading masterlist: %i of %i objects (%i KB)").str() + "\r").c_str(),
	             stats->received_objects,
	             stats->total_objects,
	             stats->received_bytes/1024);
	std::fflush(stdout);
	return 0;
}

int main(int argc, char *argv[]) {
	Settings ini;
	Game game;
	std::string gameStr;  // Allow for autodetection override
	std::string bosslogFormat;
	fs::path sortfile;  // Modlist/masterlist to sort plugins using.


	///////////////////////////////
	// Set up initial conditions
	///////////////////////////////

	LOG_INFO("BOSS starting...");

	LOG_INFO("Parsing Ini...");
	// Parse ini file if found. Can't just use BOOST's program options ini parser because of the CSS syntax and spaces.
	if (fs::exists(ini_path)) {
		try {
			ini.Load(ini_path);
		} catch (boss_error &e) {
			LOG_ERROR("Error: %s", e.getString().c_str());
			// Error will be added to log once format has been set.
		}
	} else {
		try {
			ini.Save(ini_path, AUTODETECT);
		} catch (boss_error &e) {
			ini.ErrorBuffer(ParsingError("Error: " + e.getString()));
		}
	}

	// Specify location of language dictionaries
	loc::generator gen;
	gen.add_messages_path(l10n_path.string());
	gen.add_messages_domain("messages");

	// Set the locale to get encoding and language conversions working correctly.
	// TODO(MCP): Replace this with a switch statement?
	std::string localeId = "";
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
		std::locale::global(gen(localeId));  // MCP Note: Is this std::locale or boost::locale?
		std::cout.imbue(locale());
	} catch(std::exception &e) {  // MCP Note: std::exception or boost::exception?
		LOG_ERROR("could not implement translation: %s", e.what());
		std::cout << e.what() << std::endl;
	}
	std::locale global_loc = std::locale();  // MCP Note: std::locale or boost::locale?
	std::locale loc(global_loc, new fs::detail::utf8_codecvt_facet());
	fs::path::imbue(loc);


	//////////////////////////////
	// Handle Command Line Args
	//////////////////////////////

	// Declare the supported options
	po::options_description opts("Options");
	// TODO(MCP): Come up with a good alignment scheme for these lines.
	opts.add_options()
		("help,h", loc::translate("produces this help message").str().c_str())
		("version,V", loc::translate("prints the version banner").str().c_str())
		("update,u", po::value(&gl_update)->zero_tokens(),
		loc::translate("automatically update the local copy of the"
		" masterlist to the latest version"
		" available on the web before sorting").str().c_str())
		("no-update,U", loc::translate("inhibit the automatic masterlist updater").str().c_str())
		("only-update,o", po::value(&gl_update_only)->zero_tokens(),
		loc::translate("automatically update the local copy of the"
		" masterlist to the latest version"
		" available on the web but don't sort right"
		" now").str().c_str())
		("silent,s", po::value(&gl_silent)->zero_tokens(),
		loc::translate("don't launch a browser to show the HTML log"
		" at program completion").str().c_str())
		("revert,r", po::value(&gl_revert)->implicit_value(1, ""),
		loc::translate("revert to a previous load order.  this"
		" parameter optionally accepts values of 1 or"
		" 2, indicating how many undo steps to apply."
		"  if no option value is specified, it"
		" defaults to 1").str().c_str())
		("verbose,v", po::value(&gl_debug_verbosity)->implicit_value(1, ""),
		loc::translate("specify verbosity level (0-3) of the debugging output.  0 is the"
		" default, showing only WARN and ERROR messges."
		" 1 (INFO and above) is implied if this option"
		" is specified without an argument.  higher"
		" values increase the verbosity further").str().c_str())
		("game,g", po::value(&gameStr),
		loc::translate("override game autodetection.  valid values"
		" are: 'Oblivion', 'Nehrim', 'Fallout3',"
		" 'FalloutNV', and 'Skyrim'").str().c_str())
		("crc-display,c", po::value(&gl_show_CRCs)->zero_tokens(),
		loc::translate("show mod file CRCs, so that a file's CRC can be"
		" added to the masterlist in a conditional").str().c_str())
		("format,f", po::value(&bosslogFormat),
		loc::translate("select output format. valid values"
		" are: 'html', 'text'").str().c_str())
		("trial-run,t", po::value(&gl_trial_run)->zero_tokens(),
		loc::translate("run BOSS without actually making any changes to load order").str().c_str());

	// Parse command line arguments
	po::variables_map vm;
	try {
		po::store(po::command_line_parser(argc, argv).options(opts).run(), vm);
		po::notify(vm);
	} catch (po::multiple_occurrences &) {
		LOG_ERROR("cannot specify options multiple times; please use the '--help' option to see usage instructions");
		Fail();
	} catch (std::exception & e) {
		LOG_ERROR("%s; please use the '--help' option to see usage instructions",
		          e.what());
		Fail();
	}

	// Set alternative output stream for logger and whether to track log statement origins
	// MCP Note: Could this be changed to an if-else if statement as opposed to two if-statements? Would be more efficient due to the lack of a check.
	if (gl_debug_verbosity > 0)
		g_logger.setStream(debug_log_path.string().c_str());
	if (gl_debug_verbosity < 0) {
		LOG_ERROR("invalid option for 'verbose' parameter: %d",
		          gl_debug_verbosity);
		Fail();
	}
	g_logger.setVerbosity(static_cast<LogVerbosity>(LV_WARN + gl_debug_verbosity));  // It's ok if this number is too high, setVerbosity will handle it

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
		// Sanity check and parse argument
		if (boost::iequals("Oblivion", gameStr))
			gl_game = OBLIVION;
		else if (boost::iequals("Fallout3", gameStr))
			gl_game = FALLOUT3;
		else if (boost::iequals("Nehrim", gameStr))
			gl_game = NEHRIM;
		else if (boost::iequals("FalloutNV", gameStr))
			gl_game = FALLOUTNV;
		else if (boost::iequals("Skyrim", gameStr))
			gl_game = SKYRIM;
		else {
			LOG_ERROR("invalid option for 'game' parameter: '%s'",
			          gameStr.c_str());
			Fail();
		}
		LOG_DEBUG("game ini setting overridden with: '%s' (%d)",
		          gameStr.c_str(), gl_game);
	}
	if (vm.count("format")) {
		// Sanity check and parse argument
		std::string bosslogFormat = vm["format"].as<std::string>();
		if (bosslogFormat == "html")
			gl_log_format = HTML;
		else if (bosslogFormat == "text")
			gl_log_format = PLAINTEXT;
		else {
			LOG_ERROR("invalid option for 'format' parameter: '%s'",
			          bosslogFormat.c_str());
			Fail();
		}
		LOG_DEBUG("BOSSlog format set to: '%s'", bosslogFormat.c_str());
	}


	/////////////////////////////////////////
	// Check for critical error conditions
	/////////////////////////////////////////

	// Game checks.
	LOG_DEBUG("Detecting game...");
	try {
		gl_last_game = AUTODETECT;  // Clear this setting in case the GUI was run.
		std::vector<std::uint32_t> detected, undetected;
		std::uint32_t detectedGame = DetectGame(detected, undetected);
		if (detectedGame == AUTODETECT) {
			// Now check what games were found.
			if (detected.empty()) {
				throw boss_error(BOSS_ERROR_NO_GAME_DETECTED);
			} else if (detected.size() == 1) {
				detectedGame = detected.front();
			} else {
				std::size_t ans;
				// Ask user to choose game.
				std::cout << std::endl << loc::translate("Please pick which game to run BOSS for:") << std::endl;
				for (std::size_t i = 0; i < detected.size(); i++)
					std::cout << i << " : " << Game(detected[i], "", true).Name() << std::endl;

				std::cin >> ans;
				if (ans < 0 || ans >= detected.size()) {
					std::cout << loc::translate("Invalid selection.") << std::endl;
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
		game.bosslog.criticalError << LIST_ITEM_CLASS_ERROR << loc::translate("Critical Error: ") << e.getString() << LINE_BREAK
		                           << loc::translate("Check the Troubleshooting section of the ReadMe for more information and possible solutions.") << LINE_BREAK
		                           << loc::translate("Utility will end now.");
		try {
			game.bosslog.Save(game.Log(gl_log_format), true);
		} catch (boss_error &e) {
			LOG_ERROR("Critical Error: %s", e.getString().c_str());
		}
		if (!gl_silent)
			Launch(game.Log(gl_log_format).string());  // Displays the BOSSlog.txt.
		exit(1);  // Fail in screaming heap.
	}
	game.bosslog.SetFormat(gl_log_format);
	game.bosslog.parsingErrors.push_back(ini.ErrorBuffer());


	/////////////////////////////////////////////////////////
	// Update masterlist
	/////////////////////////////////////////////////////////

	if (gl_revert < 1 && (gl_update || gl_update_only)) {
		std::cout << std::endl << loc::translate("Updating to the latest masterlist from the online repository...") << std::endl;
		LOG_DEBUG("Updating masterlist...");
		try {
			std::string revision = UpdateMasterlist(game, progress, NULL);
			std::string message = (boost::format(loc::translate("Masterlist updated; at revision: %1%.")) % revision).str();
			game.bosslog.updaterOutput << LIST_ITEM_CLASS_SUCCESS << message;
			std::cout << std::endl << message << std::endl;
		} catch (boss_error &e) {
			game.bosslog.updaterOutput << LIST_ITEM_CLASS_ERROR << loc::translate("Error: masterlist update failed.") << LINE_BREAK
			                           << (boost::format(loc::translate("Details: %1%")) % e.getString()).str() << LINE_BREAK;
			LOG_ERROR("Error: masterlist update failed. Details: %s",
			          e.getString().c_str());
		}
	} else {
		std::string revision = GetMasterlistVersion(game);
		std::string message = (boost::format(loc::translate("Masterlist updating disabled; at revision: %1%.")) % revision).str();
		game.bosslog.updaterOutput << LIST_ITEM_CLASS_SUCCESS << message;
	}

	// If true, exit BOSS now. Flush earlyBOSSlogBuffer to the bosslog and exit.
	if (gl_update_only) {
		try {
			game.bosslog.Save(game.Log(gl_log_format), true);
		} catch (boss_error &e) {
			LOG_ERROR("Critical Error: %s", e.getString().c_str());
		}
		if (!gl_silent)
			Launch(game.Log(gl_log_format).string());  // Displays the BOSSlog.
		return 0;
	}


	///////////////////////////////////
	// Resume Error Condition Checks
	///////////////////////////////////

	std::cout << std::endl << loc::translate("BOSS working...") << std::endl;

	// Build and save modlist.
	try {
		game.modlist.Load(game, game.DataFolder());
		if (gl_revert < 1)
			game.modlist.Save(game.Modlist(), game.OldModlist());
	} catch (boss_error &e) {
		LOG_ERROR("Failed to load/save modlist, error was: %s",
		          e.getString().c_str());
		game.bosslog.criticalError << LIST_ITEM_CLASS_ERROR << (boost::format(loc::translate("Critical Error: %1%")) % e.getString()).str() << LINE_BREAK
		                           << loc::translate("Check the Troubleshooting section of the ReadMe for more information and possible solutions.") << LINE_BREAK
		                           << loc::translate("Utility will end now.");
		try {
			game.bosslog.Save(game.Log(gl_log_format), true);
		} catch (boss_error &e) {
			LOG_ERROR("Critical Error: %s", e.getString().c_str());
		}
		if (!gl_silent)
			Launch(game.Log(gl_log_format).string());  // Displays the BOSSlog.txt.
		exit(1);  // Fail in screaming heap.
	}


	/////////////////////////////////
	// Parse Master- and Userlists
	/////////////////////////////////
	// Masterlist parse errors are critical, ini and userlist parse errors are not.

	// Set masterlist path to be used.
	if (gl_revert == 1)
		sortfile = game.Modlist();
	else if (gl_revert == 2)
		sortfile = game.OldModlist();
	else
		sortfile = game.Masterlist();
	LOG_INFO("Using sorting file: %s", sortfile.string().c_str());

	// Parse masterlist/modlist backup into data structure.
	try {
		LOG_INFO("Starting to parse sorting file: %s",
		         sortfile.string().c_str());
		game.masterlist.Load(game, sortfile);
		LOG_INFO("Starting to parse conditionals from sorting file: %s",
		         sortfile.string().c_str());
		game.masterlist.EvalConditions(game);
		game.masterlist.EvalRegex(game);
		game.bosslog.globalMessages = game.masterlist.GlobalMessageBuffer();
		game.bosslog.parsingErrors.push_back(game.masterlist.ErrorBuffer());
	} catch (boss_error &e) {
		LOG_ERROR("Critical Error: %s", e.getString().c_str());
		if (e.getCode() == BOSS_ERROR_FILE_PARSE_FAIL) {
			game.bosslog.criticalError << game.masterlist.ErrorBuffer();
		} else if (e.getCode() == BOSS_ERROR_CONDITION_EVAL_FAIL) {
			game.bosslog.criticalError << LIST_ITEM_CLASS_ERROR << e.getString();
		} else {
			game.bosslog.criticalError << LIST_ITEM_CLASS_ERROR << (boost::format(loc::translate("Critical Error: %1%")) % e.getString()).str() << LINE_BREAK
			                           << loc::translate("Check the Troubleshooting section of the ReadMe for more information and possible solutions.") << LINE_BREAK
			                           << loc::translate("Utility will end now.");
		}
		try {
			game.bosslog.Save(game.Log(gl_log_format), true);
		} catch (boss_error &e) {
			LOG_ERROR("Critical Error: %s", e.getString().c_str());
		}
		if (!gl_silent)
			Launch(game.Log(gl_log_format).string());  // Displays the BOSSlog.txt.
		exit(1);  // Fail in screaming heap.
	}

	LOG_INFO("Starting to parse userlist.");
	try {
		game.userlist.Load(game, game.Userlist());
		std::vector<ParsingError> errs = game.userlist.ErrorBuffer();
		game.bosslog.parsingErrors.insert(game.bosslog.parsingErrors.end(),
		                                  errs.begin(), errs.end());
	} catch (boss_error &e) {
		std::vector<ParsingError> errs = game.userlist.ErrorBuffer();
		game.bosslog.parsingErrors.insert(game.bosslog.parsingErrors.end(),
		                                  errs.begin(), errs.end());
		game.userlist.Clear();  // If userlist has parsing errors, empty it so no rules are applied.
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
		game.bosslog.criticalError << LIST_ITEM_CLASS_ERROR << (boost::format(loc::translate("Critical Error: %1%")) % e.getString()).str() << LINE_BREAK
		                           << loc::translate("Check the Troubleshooting section of the ReadMe for more information and possible solutions.") << LINE_BREAK
		                           << loc::translate("Utility will end now.");
		try {
			game.bosslog.Save(game.Log(gl_log_format), true);
		} catch (boss_error &e) {
			LOG_ERROR("Critical Error: %s", e.getString().c_str());
		}
		if (!gl_silent)
			Launch(game.Log(gl_log_format).string());  // Displays the BOSSlog.txt.
		exit(1);  // Fail in screaming heap.
	}

	LOG_INFO("Launching boss log in browser.");
	if (!gl_silent)
		Launch(game.Log(gl_log_format).string());  // Displays the BOSSlog.txt.
	LOG_INFO("BOSS finished.");
	return 0;
}
