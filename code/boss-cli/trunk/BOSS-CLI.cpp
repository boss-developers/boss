/*	Better Oblivion Sorting Software

	Quick and Dirty Load Order Utility for Oblivion, Nehrim, Fallout 3 and Fallout: New Vegas
	(Making C++ look like the scripting language it isn't.)

	Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
	http://creativecommons.org/licenses/by-nc-nd/3.0/
*/

#define NOMINMAX // we don't want the dummy min/max macros since they overlap with the std:: algorithms


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


void ShowVersion() {
	cout << "BOSS: Better Oblivion Sorting Software" << endl;
	cout << "Version " << g_version << " (" << g_releaseDate << ")" << endl;
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
	vector<item> Modlist, Masterlist;		//Modlist and masterlist data structures.
	vector<rule> Userlist;					//Userlist data structure.
	summaryCounters counters;				//Summary counters.
	bosslogContents contents;				//BOSSlog contents.
	string gameStr;							// allow for autodetection override
	fs::path bosslog_path;					//Path to BOSSlog being used.
	fs::path sortfile;						//Modlist/masterlist to sort plugins using.

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
		("format,f", po::value(&log_format),
								"select output format. valid values"
								" are: 'html', 'text'")
		("trial-run,t", po::value(&trial_run)->zero_tokens(),
								"run BOSS without actually making any changes to load order")
		("proxy-type,T", po::value(&proxy_type),
								"sets the proxy type for the masterlist updater")
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
	if (fs::exists(ini_path))
		parseIni(ini_path);
	else {
		if (!GenerateIni())
			iniErrorBuffer.push_back("<p class='error'>Error: BOSS.ini generation failed. Ensure your BOSS folder is not read-only.");
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
		if (log_format != "html" && log_format != "text") {
			LOG_ERROR("invalid option for 'format' parameter: '%s'", log_format.c_str());
			Fail();
		}
	
		LOG_DEBUG("BOSSlog format set to: '%s'", log_format.c_str());
	}

	/////////////////////////
	// BOSS Updater Stuff
	/////////////////////////

	string updateText, updateVersion;
	bool connection = false;
	try {
		connection = CheckConnection();
	} catch (boss_error e) {
		const string detail = *boost::get_error_info<err_detail>(e);
		LOG_ERROR("Update check failed. Details: '%s'", detail.c_str());
		Fail();
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
								DownloadInstallBOSSUpdate(ui, INSTALLER, updateVersion);
								cout << endl << "New installer successfully downloaded!" << endl
									<< "When you click 'OK', BOSS will launch the downloaded installer and exit. Complete the installer to complete the update." << endl << endl;

								//Now run downloaded installer then exit.
								//Although there should only be one installer file, to be safe iterate through the files vector.
								for (size_t i=0;i<updatedFiles.size();i++) {
									if (updatedFiles[i].name.empty())  //Just in case.
										continue;
									Launch(updatedFiles[i].name);
								}
								exit (0);
							} catch (boss_error e) {
								CleanUp();
								const string detail = *boost::get_error_info<err_detail>(e);
								if (detail == "Cancelled by user.") {
									cout << "Update cancelled." << endl;
									LOG_DEBUG("Update cancelled.");
								} else {
									LOG_ERROR("Update failed. Details: '%s'", detail.c_str());
									Fail();
								}
							} catch (fs::filesystem_error e) {
								CleanUp();
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
								DownloadInstallBOSSUpdate(ui, MANUAL, updateVersion);
								cout << endl << "Files successfully updated!" << endl
									<< "When you click 'OK' BOSS will exit. Once it has closed, you must manually delete your current \"BOSS GUI.exe\" and rename the downloaded \"BOSS GUI.exe.new\" to \"BOSS GUI.exe\" to complete the update." << endl << endl;

								//Now run downloaded installer then exit.
								//Although there should only be one installer file, to be safe iterate through the files vector.
								for (size_t i=0;i<updatedFiles.size();i++) {
									if (updatedFiles[i].name.empty())  //Just in case.
										continue;
									Launch(updatedFiles[i].name);
								}
								exit (0);
							} catch (boss_error e) {
								CleanUp();
								const string detail = *boost::get_error_info<err_detail>(e);
								if (detail == "Cancelled by user.") {
									cout << "Update cancelled." << endl;
									LOG_DEBUG("Update cancelled.");
								} else {
									LOG_ERROR("Update failed. Details: '%s'", detail.c_str());
									Fail();
								}
							} catch (fs::filesystem_error e) {
								CleanUp();
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
			const string detail = *boost::get_error_info<err_detail>(e);
			LOG_ERROR("BOSS Update check failed. Details: '%s'", detail.c_str());
		}
	} else {
		LOG_DEBUG("BOSS Update check failed. No Internet connection detected.");
	}

	


	/////////////////////////
	// File IO Setup
	/////////////////////////

	//Set BOSSlog path to be used.
	if (log_format == "html")
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

	//BOSSlog check.
	LOG_DEBUG("opening '%s'", bosslog_path.string().c_str());
	bosslog.open(bosslog_path.c_str());  //Might as well keep it open, just don't write anything unless an error till the end.
	if (bosslog.fail()) {
		LOG_ERROR("file '%s' could not be accessed for writing. Check the"
				  " Troubleshooting section of the ReadMe for more"
				  " information and possible solutions.", bosslog_path.string().c_str());
		Fail();
	}

	//Game checks.
	if (0 == game) {
		LOG_DEBUG("Detecting game...");
		try {
			GetGame();
		} catch (boss_error e) {
			const string detail = *boost::get_error_info<err_detail>(e);
			LOG_ERROR("Critical Error: %s", detail);
			OutputHeader();
			Output("<p class='error'>Critical Error: " + detail + "<br />");
			Output("Check the Troubleshooting section of the ReadMe for more information and possible solutions.<br />");
			Output("Utility will end now.");
			OutputFooter();
			bosslog.close();
			LOG_ERROR("Installation error found: check BOSSLOG.");
			if ( !silent ) 
				Launch(bosslog_path.string());	//Displays the BOSSlog.txt.
			exit (1); //fail in screaming heap.
		}
	}
	LOG_INFO("Game detected: %d", game);

	/////////////////////////////////////////////////////////
	// Error Condition Check Interlude - Update Masterlist
	/////////////////////////////////////////////////////////
	
	if (revert<1 && (update || update_only)) {
		//First check for internet connection, then update masterlist if connection present.
		bool connection = false;
		try {
			connection = CheckConnection();
		} catch (boss_error e) {
			const string detail = *boost::get_error_info<err_detail>(e);
			contents.updaterErrors += "<li class='warn'>Error: Masterlist update failed.<br />";
			contents.updaterErrors += "Details: " + EscapeHTMLSpecial(detail) + "<br />";
			contents.updaterErrors += "Check the Troubleshooting section of the ReadMe for more information and possible solutions.";
			LOG_ERROR("Error: Masterlist update failed. Details: %s", detail);
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
					contents.summary += "<p>Your masterlist is already at the latest revision (r" + IntToString(localRevision) + "; " + EscapeHTMLSpecial(localDate) + "). No update necessary.";
					cout << endl << "Your masterlist is already at the latest revision (" + IntToString(localRevision) + "; " + EscapeHTMLSpecial(localDate) + "). No update necessary." << endl;
					LOG_DEBUG("Masterlist update unnecessary.");
				} else {
					contents.summary += "<p>Your masterlist has been updated to revision " + IntToString(remoteRevision) + " (" + EscapeHTMLSpecial(remoteDate) + ").";
					cout << endl << "Your masterlist has been updated to revision " << remoteRevision << endl;
					LOG_DEBUG("Masterlist updated successfully.");
				}
			} catch (boss_error e) {
				const string detail = *boost::get_error_info<err_detail>(e);
				contents.updaterErrors += "<li class='warn'>Error: Masterlist update failed.<br />";
				contents.updaterErrors += "Details: " + EscapeHTMLSpecial(detail) + "<br />";
				contents.updaterErrors += "Check the Troubleshooting section of the ReadMe for more information and possible solutions.";
				LOG_ERROR("Error: Masterlist update failed. Details: %s", detail);
			}
		} else
			contents.summary += "<p>No internet connection detected. Masterlist auto-updater aborted.";
	}

	//If true, exit BOSS now. Flush earlyBOSSlogBuffer to the bosslog and exit.
	if (update_only == true) {
		OutputHeader();
		if (contents.updaterErrors.empty()) {
			Output("<h3 onclick='toggleSectionDisplay(this)'><span>&#x2212;</span>Summary</h3><ul>");
			Output(contents.summary);
			Output("</ul>");
		} else {
			Output("<h3 onclick='toggleSectionDisplay(this)'><span>&#x2212;</span>General Messages</h3><ul>");
			Output(contents.updaterErrors);
			Output("</ul>");
		}
		Output("<h3 id='end'>BOSS Execution Complete</h3>");
		OutputFooter();
		bosslog.close();
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
		const string detail = *boost::get_error_info<err_detail>(e);
		OutputHeader();
		Output("<p class='error'>Critical Error: " + detail + "<br />");
		Output("Check the Troubleshooting section of the ReadMe for more information and possible solutions.<br />");
		Output("Utility will end now.");
		OutputFooter();
		bosslog.close();
		LOG_ERROR("Failed to set modification time of game master file, error was: %s", detail);
		if ( !silent ) 
			Launch(bosslog_path.string());	//Displays the BOSSlog.txt.
		exit (1); //fail in screaming heap.
	}

	//Build and save modlist.
	BuildModlist(Modlist);
	if (revert<1) {
		try {
			SaveModlist(Modlist, curr_modlist_path);
		} catch (boss_error e) {
			const string detail = *boost::get_error_info<err_detail>(e);
			OutputHeader();
			Output("<p class='error'>Critical Error: Modlist backup failed!<br />");
			Output("Details: " + EscapeHTMLSpecial(detail) + ".<br />");
			Output("Check the Troubleshooting section of the ReadMe for more information and possible solutions.<br />");
			Output("Utility will end now.");
			OutputFooter();
			bosslog.close();
			if ( !silent ) 
				Launch(bosslog_path.string());	//Displays the BOSSlog.txt.
			exit (1); //fail in screaming heap.
		}
	}


	/////////////////////////////////
	// Parse Master- and Userlists
	/////////////////////////////////
	//Masterlist parse errors are critical, ini and userlist parse errors are not.
	
	//Parse masterlist/modlist backup into data structure.
	LOG_INFO("Starting to parse sorting file: %s", sortfile.string().c_str());
	try {
		parseMasterlist(sortfile,Masterlist);
	} catch (boss_error e) {
		const string detail = *boost::get_error_info<err_detail>(e);
		OutputHeader();
		Output("<p class='error'>Critical Error: " +EscapeHTMLSpecial(detail) +"<br />");
		Output("Check the Troubleshooting section of the ReadMe for more information and possible solutions.<br />");
		Output("Utility will end now.");
		OutputFooter();
        bosslog.close();
        LOG_ERROR("Couldn't open sorting file: %s", sortfile.filename().string().c_str());
        if ( !silent ) 
                Launch(bosslog_path.string());  //Displays the BOSSlog.txt.
        exit (1); //fail in screaming heap.
	}

	//Check if parsing failed. If so, exit with errors.
	if (!masterlistErrorBuffer.empty()) {
		OutputHeader();
		size_t size = masterlistErrorBuffer.size();
		for (size_t i=0; i<size; i++)  //Print parser error messages.
			Output(masterlistErrorBuffer[i]);
		OutputFooter();
		bosslog.close();
		if ( !silent ) 
                Launch(bosslog_path.string());  //Displays the BOSSlog.txt.
        exit (1); //fail in screaming heap.
	}

	LOG_INFO("Starting to parse userlist.");
	try {
		bool parsed = parseUserlist(userlist_path,Userlist);
		if (!parsed)
			Userlist.clear();  //If userlist has parsing errors, empty it so no rules are applied.
	} catch (boss_error e) {
		const string detail = *boost::get_error_info<err_detail>(e);
		userlistErrorBuffer.push_back("<p class='error'>Error: "+detail+" Userlist parsing aborted. No rules will be applied.");
		LOG_ERROR("Error: %s", detail);
	}


	/////////////////////////////////////////////////
	// Compare Masterlist against Modlist, Userlist
	/////////////////////////////////////////////////

	lastRecognisedPos = BuildWorkingModlist(Modlist,Masterlist,Userlist);
	LOG_INFO("Modlist now filled with ordered mods and unknowns.");


	//////////////////////////
	// Apply Userlist Rules
	//////////////////////////

	//Apply userlist rules to modlist.
	if (revert<1 && fs::exists(userlist_path)) {
		ApplyUserRules(Modlist, Userlist, contents.userlistMessages, lastRecognisedPos);
		LOG_INFO("Userlist sorting process finished.");
	}

	//////////////////////////////////////////////////////
	// Print version & checksum info for OBSE & plugins
	//////////////////////////////////////////////////////

	if (show_CRCs)
		scriptExtender = GetSEPluginInfo(contents.seInfo);

	////////////////////////////////
	// Re-date Files & Output Info
	////////////////////////////////

	//Re-date .esp/.esm files according to order in modlist and output messages
	SortRecognisedMods(Modlist, lastRecognisedPos, contents.recognisedPlugins, esmtime, counters);


	//Find and show found mods not recognised. These are the mods that are found at and after index lastRecognisedPos in the mods vector.
	//Order their dates to be i days after the master esm to ensure they load last.
	ListUnrecognisedMods(Modlist, lastRecognisedPos, contents.unrecognisedPlugins, esmtime, counters);

	/////////////////////////////
	// Print Output to BOSSlog
	/////////////////////////////

	PrintBOSSlog(contents, counters, scriptExtender);

	bosslog.close();
	LOG_INFO("Launching boss log in browser.");
	if ( !silent ) 
		Launch(bosslog_path.string());	//Displays the BOSSlog.txt.
	LOG_INFO("BOSS finished.");
	return (0);
}