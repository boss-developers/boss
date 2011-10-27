/*	Better Oblivion Sorting Software

	A "one-click" program for users that quickly optimises and avoids 
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge, 
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

    Copyright (C) 2011  Random/Random007/jpearce, WrinklyNinja & the BOSS 
	development team. Copyright license:
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 1783 $, $Date: 2010-10-31 23:05:28 +0000 (Sun, 31 Oct 2010) $
*/

#define NOMINMAX // we don't want the dummy min/max macros since they overlap with the std:: algorithms

//We want to ensure that the GUI-specific code in BOSS-Common isn't included.
#ifdef BOSSGUI
#undef BOSSGUI
#endif


#include "BOSS-Common.h"

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem/detail/utf8_codecvt_facet.hpp>
#include <boost/exception/get_error_info.hpp>
#include <boost/unordered_set.hpp>

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
using boost::algorithm::trim_copy;

#if _WIN32 || _WIN64
	const string launcher_cmd = "start";
#else
	const string launcher_cmd = "xdg-open";
#endif

int Launch(const string& filename) {
	const string cmd = launcher_cmd + " " + filename;
	return ::system(cmd.c_str());
}


void ShowVersion() {
	cout << "BOSS: Better Oblivion Sorting Software" << endl;
	cout << "Version " << BOSS_VERSION_MAJOR << "." << BOSS_VERSION_MINOR
		 << "." << BOSS_VERSION_PATCH << " (" << boss_release_date << ")" << endl;
}

void ShowUsage(po::options_description opts) {

	static string progName =
#if _WIN32 || _WIN64
		"BOSS";
#else
		"boss";
#endif

	ShowVersion();
	cout << endl << "Description:" << endl;
	cout << "  BOSS is a utility that sorts the mod load order of TESIV: Oblivion, Nehrim," << endl;
	cout << "  Fallout 3, Fallout: New Vegas and TESV: Skyrim according to a frequently updated" << endl;
	cout << "  masterlist to minimise incompatibilities between mods." << endl << endl;
	cout << opts << endl;
	cout << "Examples:" << endl;
	cout << "  " << progName << " -u" << endl;
	cout << "    updates the masterlist, sorts your mods, and shows the log" << endl << endl;
	cout << "  " << progName << " -sr" << endl;
	cout << "    reverts your load order 1 level and skips showing the log" << endl << endl;
	cout << "  " << progName << " -r 2" << endl;
	cout << "    reverts your load order 2 levels and shows the log" << endl << endl;
}

void Fail() {
#if _WIN32 || _WIN64
	cout << "Press ENTER to quit...";
	cin.ignore(1, '\n');
#endif

	exit(1);
}

void InhibitUpdate(bool val) {
	if (val)
		update = false;
}

int main(int argc, char *argv[]) {

	size_t lastRecognisedPos = 0;			//position of last recognised mod.
	string scriptExtender;					//What script extender is present.
	time_t esmtime = 0;						//File modification times.
	ItemList modlist, masterlist;		//modlist and masterlist data structures.
	RuleList userlist;					//userlist data structure.
	Ini ini;
	bosslogContents contents;				//BOSSlog contents.
	string gameStr;							// allow for autodetection override
	string bosslogFormat;
	fs::path bosslog_path;					//Path to BOSSlog being used.
	fs::path sortfile;						//modlist/masterlist to sort plugins using.
	Outputter output;

	//Set the locale to get encoding conversions working correctly.
	setlocale(LC_CTYPE, "");
	locale global_loc = locale();
	locale loc(global_loc, new boost::filesystem::detail::utf8_codecvt_facet());
	boost::filesystem::path::imbue(loc);
	
	// declare the supported options
	po::options_description opts("Options");
	opts.add_options()
		("help,h",				"produces this help message")
		("version,V",			"prints the version banner")
		("update,u", po::value(&update)->zero_tokens(),
								"automatically update the local copy of the"
								" masterlist to the latest version"
								" available on the web before sorting")
		("no-update,U", po::value<bool>()->zero_tokens()->notifier(&InhibitUpdate),
								"inhibit the automatic masterlist updater")
		("only-update,o", po::value(&update_only)->zero_tokens(),
								"automatically update the local copy of the"
								" masterlist to the latest version"
								" available on the web but don't sort right"
								" now")
		("silent,s", po::value(&silent)->zero_tokens(),
								"don't launch a browser to show the HTML log"
								" at program completion")
		("no-version-parse,n", po::value(&skip_version_parse)->zero_tokens(),
								"don't extract mod version numbers for"
								" printing in the HTML log")
		("revert,r", po::value(&revert)->implicit_value(1, ""),
								"revert to a previous load order.  this"
								" parameter optionally accepts values of 1 or"
								" 2, indicating how many undo steps to apply."
								"  if no option value is specified, it"
								" defaults to 1")
		("verbose,v", po::value(&debug_verbosity)->implicit_value(1, ""),
								"specify verbosity level (0-3) of the debugging output.  0 is the"
								" default, showing only WARN and ERROR messges."
								" 1 (INFO and above) is implied if this option"
								" is specified without an argument.  higher"
								" values increase the verbosity further")
		("game,g", po::value(&gameStr),
								"override game autodetection.  valid values"
								" are: 'Oblivion', 'Nehrim', 'Fallout3',"
								" 'FalloutNV', and 'Skyrim'")
		("debug-with-source,d", po::value(&debug_with_source)->zero_tokens(),
								"add source file references to debug statements")
		("crc-display,c", po::value(&show_CRCs)->zero_tokens(),
								"show mod file CRCs, so that a file's CRC can be"
								" added to the masterlist in a conditional")
		("format,f", po::value(&bosslogFormat),
								"select output format. valid values"
								" are: 'html', 'text'")
		("trial-run,t", po::value(&trial_run)->zero_tokens(),
								"run BOSS without actually making any changes to load order")
		("proxy-host,H", po::value(&proxy_host),
								"sets the proxy hostname for the masterlist updater")
		("proxy-port,P", po::value(&proxy_port),
								"sets the proxy port number for the masterlist updater")
		("log-debug,l", po::value(&log_debug_output)->zero_tokens(),
								"logs the debug output to the BOSSDebugLog.txt file instead"
								" of the command line.");
	
	///////////////////////////////
	// Set up initial conditions
	///////////////////////////////

	LOG_INFO("BOSS starting...");

	LOG_INFO("Parsing Ini...");
	//Parse ini file if found. Can't just use BOOST's program options ini parser because of the CSS syntax and spaces.
	if (fs::exists(ini_path)) {
		try {
			ini.Load(ini_path);
		} catch (boss_error e) {
			LOG_ERROR("Ini parsing failed. Details: %s", e.getString().c_str());
			//Error will be added to log once format has been set.
		}
	} else {
		try {
			ini.Save(ini_path);
		} catch (boss_error e) {
			contents.iniParsingError = "<p class='error'>Error: BOSS.ini generation failed. Details: " + e.getString() + ". Ensure your BOSS folder is not read-only.";
		}
	}

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
	if (log_debug_output)
		g_logger.setStream(debug_log_path.string().c_str());
	g_logger.setOriginTracking(debug_with_source);

	if (vm.count("verbose")) {
		if (0 > debug_verbosity) {
			LOG_ERROR("invalid option for 'verbose' parameter: %d", debug_verbosity);
			Fail();
		}

		// it's ok if this number is too high.  setVerbosity will handle it
		g_logger.setVerbosity(static_cast<LogVerbosity>(LV_WARN + debug_verbosity));
	}
	if ((vm.count("update")) && (vm.count("no-update"))) {
		LOG_ERROR("invalid options: --update,-u and --no-update,-U cannot both be given.");
		Fail();
	}
	if (vm.count("help")) {
		ShowUsage(opts);
		exit(0);
	}
	if (vm.count("version")) {
		ShowVersion();
		exit(0);
	}
	if (vm.count("revert")) {
		// sanity check argument
		if (revert < 1 || revert > 2) {
			LOG_ERROR("invalid option for 'revert' parameter: %d", revert);
			Fail();
		}
	}
	if (vm.count("game")) {
		// sanity check and parse argument
		if      (boost::iequals("Oblivion",   gameStr)) { game = 1; }
		else if (boost::iequals("Fallout3",   gameStr)) { game = 2; }
		else if (boost::iequals("Nehrim",     gameStr)) { game = 3; }
		else if (boost::iequals("FalloutNV", gameStr)) { game = 4; }
		else if (boost::iequals("Skyrim", gameStr)) { game = 5; }
		else {
			LOG_ERROR("invalid option for 'game' parameter: '%s'", gameStr.c_str());
			Fail();
		}
	
		LOG_DEBUG("game autodectection overridden with: '%s' (%d)", gameStr.c_str(), game);
	}

	if (vm.count("format")) {
		// sanity check and parse argument
		if (bosslogFormat == "html")
			log_format = HTML;
		else if (bosslogFormat == "text")
			log_format = PLAINTEXT;
		else {
			LOG_ERROR("invalid option for 'format' parameter: '%s'", bosslogFormat.c_str());
			Fail();
		}
		LOG_DEBUG("BOSSlog format set to: '%s'", bosslogFormat.c_str());
	}
	output.SetFormat(log_format);
	if (!ini.errorBuffer.Empty())
		contents.iniParsingError = ini.errorBuffer.FormatFor(log_format);

	/////////////////////////
	// BOSS Updater Stuff
	/////////////////////////

	string updateText, updateVersion;
	bool connection = false;
	try {
		connection = CheckConnection();
	} catch (boss_error e) {
		LOG_ERROR("Update check failed. Details: '%s'", e.getString().c_str());
	}
	if (connection) {
		cout << "Checking for BOSS updates..." << endl;
		LOG_DEBUG("Checking for BOSS updates...");
		try {
			updateVersion = IsBOSSUpdateAvailable();
			if (updateVersion.empty()) {
				cout << "You are already using the latest version of BOSS." << endl;
				LOG_DEBUG("You are already using the latest version of BOSS.");
			} else {
				cout << "Update available! New version: " << updateVersion << endl << "Do you want to download and install the update? (y/N)"<< endl;
				//Does the user want to update?
				string answer;
				cin >> answer;
				if (answer == "n" || answer == "N") {
					cout << "No update has been downloaded or installed." << endl;
					LOG_DEBUG("No update has been downloaded or installed.");
				} else if (answer == "y" || answer == "Y") {
					//First detect type of current install: manual or installer.
					if (fs::exists("BOSS ReadMe.lnk")) {  //Installer
						cout << endl << "Your current install has been determined as having been installed via the BOSS installer." << endl
							<< "The BOSS Updater will download the installer for the new version to this BOSS folder." << endl
							<< "It will then launch the installer before exiting. Complete the installer to complete the update." << endl
							<< "Do you wish to continue? (y/N)" << endl;

						cin >> answer;
						if (answer == "n" || answer == "N") {
						cout << "BOSS Updater cancelled." << endl;
						LOG_DEBUG("BOSS Updater cancelled.");
						} else if (answer == "y" || answer == "Y") {
							try {
								uiStruct ui;
								vector<string> fails = DownloadInstallBOSSUpdate(ui, INSTALLER, updateVersion);

								cout << endl << "Release notes for v" << updateVersion+":" << endl << endl << FetchReleaseNotes(updateVersion) << endl;

								cout << endl << "New installer successfully downloaded!" << endl;
								if (!fails.empty()) {
									cout << "There were errors renaming the downloaded files. After BOSS quits, remove the \".new\" extension from the following file(s), deleting any existing files with the same names, then run the downloaded installer to complete the update:" << endl << endl;
									size_t size=fails.size();
									for (size_t i=0;i<size;i++)
										cout << fails[i] << ".new" << endl;
								} else {
									cout << "BOSS will now launch the downloaded installer and exit. Complete the installer to complete the update." << endl << endl;

									//Now run downloaded installer then exit.
									//Although there should only be one installer file, to be safe iterate through the files vector.
									for (size_t i=0;i<updatedFiles.size();i++) {
										if (updatedFiles[i].name.empty())  //Just in case.
											continue;
										else if (fs::exists(updatedFiles[i].name))
											Launch(updatedFiles[i].name);
									}
								}
								Fail();
							} catch (boss_error e) {
								try {
									CleanUp();
								} catch (boss_error ee) {
									LOG_ERROR("Update clean up failed. Details: '%s'", ee.getString().c_str());
									Fail();
								}
								if (e.getCode() == BOSS_ERROR_CURL_USER_CANCEL) {
									cout << "Update cancelled." << endl;
									LOG_DEBUG("Update cancelled.");
								} else {
									LOG_ERROR("Update failed. Details: '%s'", e.getString().c_str());
									Fail();
								}
							} catch (fs::filesystem_error e) {
								try {
									CleanUp();
								} catch (boss_error ee) {
									LOG_ERROR("Update clean up failed. Details: '%s'", ee.getString().c_str());
									Fail();
								}
								string detail = e.what();
								LOG_ERROR("Update failed. Details: '%s'", detail.c_str());
								Fail();
							}
						} else {
							LOG_ERROR("invalid option given: '%s'", answer.c_str());
							Fail();
						}
					} else {  //Manual.
						cout << endl << "Your current install has been determined as having been installed manually." << endl
							<< "The BOSS Updater will download the updated files and replace your existing files with them." << endl
							<< "Your current BOSS.ini will be renamed to BOSS.ini.old. It may still be opened in your chosen text editor, allowing you to migrate your settings." << endl
							<< " Your current userlist.txt will not be replaced." << endl
							<< "Do you wish to continue? (y/N)" << endl;

						cin >> answer;
						if (answer == "n" || answer == "N") {
						cout << "BOSS Updater cancelled." << endl;
						LOG_DEBUG("BOSS Updater cancelled.");
						} else if (answer == "y" || answer == "Y") {
							try {
								uiStruct ui;
								vector<string> fails = DownloadInstallBOSSUpdate(ui, MANUAL, updateVersion);

								cout << endl << "Release notes for v" << updateVersion+":" << endl << endl << FetchReleaseNotes(updateVersion) << endl;
								
								if (!fails.empty()) {
									cout << endl << "Files successfully downloaded!" << endl
										<< "However, the following files could not be automatically installed. After BOSS quits, remove the \".new\" extension from the following file(s), deleting any existing files with the same names to complete the update:" << endl << endl;
									size_t size=fails.size();
									for (size_t i=0;i<size;i++) {
										cout << fails[i] << ".new" << endl;
									}
								} else
									cout << endl << "Files successfully updated!" << endl
										<< "BOSS will now exit." << endl << endl;

								Fail();
							} catch (boss_error e) {
								try {
									CleanUp();
								} catch (boss_error ee) {
									LOG_ERROR("Update clean up failed. Details: '%s'", ee.getString().c_str());
									Fail();
								}
								if (e.getCode() == BOSS_ERROR_CURL_USER_CANCEL) {
									cout << "Update cancelled." << endl;
									LOG_DEBUG("Update cancelled.");
								} else {
									LOG_ERROR("Update failed. Details: '%s'", e.getString().c_str());
									Fail();
								}
							} catch (fs::filesystem_error e) {
								try {
									CleanUp();
								} catch (boss_error ee) {
									LOG_ERROR("Update clean up failed. Details: '%s'", ee.getString().c_str());
									Fail();
								}
								string detail = e.what();
								LOG_ERROR("Update failed. Details: '%s'", detail.c_str());
								Fail();
							}
						} else {
							LOG_ERROR("invalid option given: '%s'", answer.c_str());
							Fail();
						}		
					}
				} else {
					LOG_ERROR("invalid option given: '%s'", answer.c_str());
					Fail();
				}
			}
		} catch (boss_error e) {
			LOG_ERROR("BOSS Update check failed. Details: '%s'", e.getString().c_str());
		}
	} else {
		LOG_DEBUG("BOSS Update check failed. No Internet connection detected.");
	}


	/////////////////////////
	// File IO Setup
	/////////////////////////

	//Set BOSSlog path to be used.
	if (log_format == HTML)
		bosslog_path = bosslog_html_path;
	else
		bosslog_path = bosslog_text_path;

	//Set masterlist path to be used.
	if (revert==1)
		sortfile = curr_modlist_path;	
	else if (revert==2) 
		sortfile = prev_modlist_path;
	else 
		sortfile = masterlist_path;
	LOG_INFO("Using sorting file: %s", sortfile.string().c_str());


	////////////////////////////////////////////////
	// Record last BOSSlog's recognised mod list
	////////////////////////////////////////////////

	//Back up old recognised mod list for diff later. Only works for HTML bosslog due to formatting conversion.
	if (fs::exists(bosslog_html_path))
		contents.oldRecognisedPlugins = GetOldRecognisedList(bosslog_html_path);


	/////////////////////////////////////////
	// Check for critical error conditions
	/////////////////////////////////////////

	//Game checks.
	if (0 == game) {
		LOG_DEBUG("Detecting game...");
		try {
			GetGame();
		} catch (boss_error e) {
			LOG_ERROR("Critical Error: %s", e.getString().c_str());
			output.Clear();
			output.PrintHeader();
			output << LIST_OPEN << LIST_ITEM_CLASS_ERROR << "Critical Error: " << e.getString() << LINE_BREAK
				<< "Check the Troubleshooting section of the ReadMe for more information and possible solutions." << LINE_BREAK
				<< "Utility will end now." << LIST_CLOSE;
			output.PrintFooter();
			try {
				output.Save(bosslog_path, true);
			} catch (boss_error e) {
				LOG_ERROR("Critical Error: %s", e.getString().c_str());
			}
			LOG_ERROR("Installation error found: check BOSSLOG.");
			if ( !silent ) 
				Launch(bosslog_path.string());	//Displays the BOSSlog.txt.
			exit (1); //fail in screaming heap.
		}
	}
	LOG_INFO("Game detected: %d", game);

	/////////////////////////////////////////////////////////
	// Error Condition Check Interlude - Update masterlist
	/////////////////////////////////////////////////////////
	
	if (revert<1 && (update || update_only)) {
		//First check for internet connection, then update masterlist if connection present.
		bool connection = false;
		try {
			connection = CheckConnection();
		} catch (boss_error e) {
			output << LIST_ITEM_CLASS_WARN << "Error: masterlist update failed." << LINE_BREAK
				<< "Details: " << e.getString() << LINE_BREAK
				<< "Check the Troubleshooting section of the ReadMe for more information and possible solutions.";
			contents.updaterErrors = output.AsString();
			LOG_ERROR("Error: masterlist update failed. Details: %s", e.getString().c_str());
		}
		if (connection) {
			cout << endl << "Updating to the latest masterlist from the Google Code repository..." << endl;
			LOG_DEBUG("Updating masterlist...");
			try {
				string localDate, remoteDate;
				unsigned int localRevision, remoteRevision;
				uiStruct ui;
				UpdateMasterlist(ui, localRevision, localDate, remoteRevision, remoteDate);
				if (localRevision == remoteRevision) {
					output << PARAGRAPH << "Your masterlist is already at the latest revision (r" << localRevision << "; " << localDate << "). No update necessary.";
					cout << endl << "Your masterlist is already at the latest revision (" << localRevision << "; " << localDate << "). No update necessary." << endl;
					LOG_DEBUG("masterlist update unnecessary.");
				} else {
					output << PARAGRAPH << "Your masterlist has been updated to revision " << remoteRevision << " (" << remoteDate << ").";
					cout << endl << "Your masterlist has been updated to revision " << remoteRevision << endl;
					LOG_DEBUG("masterlist updated successfully.");
				}
				contents.summary = output.AsString();
			} catch (boss_error e) {
				output << LIST_ITEM_CLASS_WARN << "Error: masterlist update failed." << LINE_BREAK
					<< "Details: " << e.getString() << LINE_BREAK
					<< "Check the Troubleshooting section of the ReadMe for more information and possible solutions.";
				contents.updaterErrors = output.AsString();
				LOG_ERROR("Error: masterlist update failed. Details: %s", e.getString().c_str());
			}
		} else {
			output << PARAGRAPH << "No internet connection detected. masterlist auto-updater aborted.";
			contents.summary = output.AsString();
		}
	}

	//If true, exit BOSS now. Flush earlyBOSSlogBuffer to the bosslog and exit.
	if (update_only == true) {
		output.Clear();
		output.PrintHeader();
		if (contents.updaterErrors.empty())
			output << HEADING_OPEN << "Summary" << HEADING_CLOSE << contents.summary;
		else
			output << HEADING_OPEN << "General Messages" << HEADING_CLOSE << LIST_OPEN
				<< contents.updaterErrors << LIST_CLOSE;
		output << HEADING_ID_END_OPEN << "Execution Complete" << HEADING_CLOSE;
		output.PrintFooter();
		try {
			output.Save(bosslog_path, true);
		} catch (boss_error e) {
			LOG_ERROR("Critical Error: %s", e.getString().c_str());
		}
		if ( !silent ) 
			Launch(bosslog_path.string());	//Displays the BOSSlog.
		return (0);
	}


	///////////////////////////////////
	// Resume Error Condition Checks
	///////////////////////////////////

	cout << endl << "Better Oblivion Sorting Software working..." << endl;

	//Get the master esm's modification date. 
	try {
		esmtime = GetMasterTime();
	} catch(boss_error e) {
		output.Clear();
		output.PrintHeader();
		output << LIST_OPEN << LIST_ITEM_CLASS_ERROR << "Critical Error: " << e.getString() << LINE_BREAK
			<< "Check the Troubleshooting section of the ReadMe for more information and possible solutions." << LINE_BREAK
			<< "Utility will end now." << LIST_CLOSE;
		output.PrintFooter();
		try {
			output.Save(bosslog_path, true);
		} catch (boss_error e) {
			LOG_ERROR("Critical Error: %s", e.getString().c_str());
		}
		LOG_ERROR("Failed to set modification time of game master file, error was: %s", e.getString().c_str());
		if ( !silent ) 
			Launch(bosslog_path.string());	//Displays the BOSSlog.txt.
		exit (1); //fail in screaming heap.
	}

	//Build and save modlist.
	try {
		modlist.Load(data_path);
		if (revert<1)
			modlist.Save(curr_modlist_path);
	} catch (boss_error e) {
		output.Clear();
		output.PrintHeader();
		output << LIST_OPEN << LIST_ITEM_CLASS_ERROR << "Critical Error: " << e.getString() << LINE_BREAK
			<< "Check the Troubleshooting section of the ReadMe for more information and possible solutions." << LINE_BREAK
			<< "Utility will end now." << LIST_CLOSE;
		output.PrintFooter();
		try {
			output.Save(bosslog_path, true);
		} catch (boss_error e) {
			LOG_ERROR("Critical Error: %s", e.getString().c_str());
		}
		LOG_ERROR("Failed to load/save modlist, error was: %s", e.getString().c_str());
		if ( !silent ) 
			Launch(bosslog_path.string());	//Displays the BOSSlog.txt.
		exit (1); //fail in screaming heap.
	}


	/////////////////////////////////
	// Parse Master- and Userlists
	/////////////////////////////////
	//masterlist parse errors are critical, ini and userlist parse errors are not.
	
	//Parse masterlist/modlist backup into data structure.
	LOG_INFO("Starting to parse sorting file: %s", sortfile.string().c_str());
	try {
		masterlist.Load(sortfile);
		contents.globalMessages = masterlist.globalMessageBuffer;
	} catch (boss_error e) {
		contents.criticalError = masterlist.errorBuffer.FormatFor(log_format);
		output.Clear();
		output.PrintHeader();
		if (e.getCode() == BOSS_ERROR_FILE_PARSE_FAIL)
			output << HEADING_OPEN << "General Messages" << HEADING_CLOSE << LIST_OPEN
				<< contents.criticalError << LIST_CLOSE;
		else
			output << LIST_OPEN << LIST_ITEM_CLASS_ERROR << "Critical Error: " << e.getString() << LINE_BREAK
				<< "Check the Troubleshooting section of the ReadMe for more information and possible solutions." << LINE_BREAK
				<< "Utility will end now." << LIST_CLOSE;
		output.PrintFooter();
		try {
			output.Save(bosslog_path, true);
		} catch (boss_error e) {
			LOG_ERROR("Critical Error: %s", e.getString().c_str());
		}
		LOG_ERROR("Couldn't open sorting file: %s", sortfile.filename().string().c_str());
        if ( !silent ) 
                Launch(bosslog_path.string());  //Displays the BOSSlog.txt.
        exit (1); //fail in screaming heap.
	}

	LOG_INFO("Starting to parse userlist.");
	try {
		userlist.Load(userlist_path);
		if (!userlist.parsingErrorBuffer.Empty())
			contents.userlistParsingError = userlist.parsingErrorBuffer.FormatFor(log_format);
		for (vector<ParsingError>::iterator iter; iter != userlist.syntaxErrorBuffer.end(); ++iter)
			contents.userlistSyntaxErrors.push_back(iter->FormatFor(log_format));
	} catch (boss_error e) {
		userlist.rules.clear();  //If userlist has parsing errors, empty it so no rules are applied.
		output.Clear();
		output << LIST_ITEM_CLASS_ERROR << "Error: " << e.getString() << " userlist parsing aborted. No rules will be applied.";
		contents.userlistParsingError = output.AsString();
		LOG_ERROR("Error: %s", e.getString().c_str());
	}

	//////////////////////////////////
	// Perform sorting functionality
	//////////////////////////////////

	PerformSortingFunctionality(bosslog_path, modlist, masterlist, userlist, esmtime, contents);

	LOG_INFO("Launching boss log in browser.");
	if ( !silent ) 
		Launch(bosslog_path.string());	//Displays the BOSSlog.txt.
	LOG_INFO("BOSS finished.");
	return (0);
}