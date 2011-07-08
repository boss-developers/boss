/*	Better Oblivion Sorting Software

	Quick and Dirty Load Order Utility for Oblivion, Nehrim, Fallout 3 and Fallout: New Vegas
	(Making C++ look like the scripting language it isn't.)

	Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
	http://creativecommons.org/licenses/by-nc-nd/3.0/
*/

#define NOMINMAX // we don't want the dummy min/max macros since they overlap with the std:: algorithms


#include "BOSS.h"

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem/detail/utf8_codecvt_facet.hpp>
#include "boost/exception/get_error_info.hpp"
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
	cout << "  Fallout 3, and Fallout: New Vegas according to a frequently updated" << endl;
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

	size_t x=0;							//position of last recognised mod.
	string textbuf;						//a text string.
	time_t esmtime = 0, modfiletime = 0;	//File modification times.
	vector<item> Modlist, Masterlist;	//Modlist and masterlist data structures.
	vector<rule> Userlist;				//Userlist data structure.
	//Summary counters
	int recModNo = 0, unrecModNo = 0, ghostModNo = 0, messageNo = 0, warningNo = 0, errorNo = 0;
	string gameStr;                  // allow for autodetection override
	string oldBOSSlogRecognised = "";
	string masterlistUpdateContent = "";
	string userlistMessagesContent = "";
	string seInfoContent = "";
	string recogModContent = "";
	string unrecogModContent = "";
	fs::path bosslog_path;				//Path to BOSSlog being used.
	fs::path sortfile;					//Modlist/masterlist to sort plugins using.

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
		("verbose,v", po::value(&verbosity)->implicit_value(1, ""),
								"specify verbosity level (0-3).  0 is the"
								" default, showing only WARN and ERROR messges."
								" 1 (INFO and above) is implied if this option"
								" is specified without an argument.  higher"
								" values increase the verbosity further")
		("game,g", po::value(&gameStr),
								"override game autodetection.  valid values"
								" are: 'Oblivion', 'Nehrim', 'Fallout3', and"
								" 'FalloutNV'")
		("debug,d", po::value(&debug)->zero_tokens(),
								"add source file references to logging statements")
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
								"sets the proxy port number for the masterlist updater");
	
	///////////////////////////////
	// Set up initial conditions
	///////////////////////////////

	//Parse ini file if found. Can't just use BOOST's program options ini parser because of the CSS syntax and spaces.
	if (fs::exists("BOSS.ini"))
		parseIni("BOSS.ini");
	else {
		if (!GenerateIni())
			iniErrorBuffer.push_back("<p class='error'>Error: BOSS.ini generation failed. Ensure your BOSS folder is not read-only.</p>\n\n");
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
	
	// set whether to track log statement origins
	g_logger.setOriginTracking(debug);

	LOG_INFO("BOSS starting...");

	if (vm.count("verbose")) {
		if (0 > verbosity) {
			LOG_ERROR("invalid option for 'verbose' parameter: %d", verbosity);
			Fail();
		}

		// it's ok if this number is too high.  setVerbosity will handle it
		g_logger.setVerbosity(static_cast<LogVerbosity>(LV_WARN + verbosity));
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

	//Back up old recognised mod list for diff later.
	if (fs::exists(bosslog_text_path) || fs::exists(bosslog_html_path)) {
		size_t pos1 = string::npos, pos2 = string::npos;
		if (fs::exists(bosslog_text_path) && fs::exists(bosslog_html_path)) {
			//Need to work out which was last written.
			time_t htmllogtime = 0, textlogtime = 0;
			try {
				htmllogtime = fs::last_write_time(bosslog_html_path);
				textlogtime = fs::last_write_time(bosslog_text_path);
			}catch (fs::filesystem_error e){
				LOG_WARN("%s; This may result in the BOSSlog diff being incorrect.", e.what());
			}
			if (textlogtime > htmllogtime) {
				fileToBuffer(bosslog_text_path,oldBOSSlogRecognised);
				pos1 = oldBOSSlogRecognised.find("Recognised And Re-ordered Plugins\n")+34;
				if (pos1 != string::npos)
					pos2 = oldBOSSlogRecognised.find("\n\n\n======================================", pos1);
			} else {
				fileToBuffer(bosslog_html_path,oldBOSSlogRecognised);
				pos1 = oldBOSSlogRecognised.find("<ul id='recognised'>\n")+21;
				if (pos1 != string::npos)
					pos2 = oldBOSSlogRecognised.find("</ul>\n</div>\n", pos1);
			}
		} else if (fs::exists(bosslog_html_path)) {
			fileToBuffer(bosslog_html_path,oldBOSSlogRecognised);
			pos1 = oldBOSSlogRecognised.find("<ul id='recognised'>\n")+21;
			if (pos1 != string::npos)
				pos2 = oldBOSSlogRecognised.find("</ul>\n</div>\n", pos1);
		} else {
			fileToBuffer(bosslog_text_path,oldBOSSlogRecognised);
			pos1 = oldBOSSlogRecognised.find("Recognised And Re-ordered Plugins\n")+34;
			if (pos1 != string::npos)
				pos2 = oldBOSSlogRecognised.find("\n\n\n======================================", pos1);
		}
		if (pos2 != string::npos)
			oldBOSSlogRecognised = oldBOSSlogRecognised.substr(pos1, pos2-pos1);
	}


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
		if (fs::exists(data_path / "Oblivion.esm")) {
			game = 1;
			if (fs::exists(data_path / "Nehrim.esm")) {
				OutputHeader();
				Output("<p class='error'>Critical Error: Oblivion.esm and Nehrim.esm have both been found!<br />\n");
				Output("Please ensure that you have installed Nehrim correctly. In a correct install of Nehrim, there is no Oblivion.esm.<br />\n");
				Output("Utility will end now.</p>\n\n");
				OutputFooter();
				bosslog.close();
				LOG_ERROR("Installation error found: check BOSSLOG.");
				if ( !silent ) 
					Launch(bosslog_path.string());	//Displays the BOSSlog.txt.
				exit (1); //fail in screaming heap.
			}
		} else if (fs::exists(data_path / "Nehrim.esm")) game = 3;
		else if (fs::exists(data_path / "FalloutNV.esm")) game = 4;
		else if (fs::exists(data_path / "Fallout3.esm")) game = 2;
		else {
			LOG_ERROR("None of the supported games were detected...");
			OutputHeader();
			Output("<p class='error'>Critical Error: Master .ESM file not found!<br />\n");
			Output("Check the Troubleshooting section of the ReadMe for more information and possible solutions.<br />\n");
			Output("Utility will end now.</p>\n\n");
			OutputFooter();
			bosslog.close();
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
		} catch (boss_error & e) {
			string const * detail = boost::get_error_info<err_detail>(e);
			masterlistUpdateContent += "<li class='warn'>Error: Masterlist update failed.<br />\n";
			masterlistUpdateContent += "Details: " + *detail + "<br />\n";
			masterlistUpdateContent += "Check the Troubleshooting section of the ReadMe for more information and possible solutions.</li>\n";
		}
		if (connection) {
			cout << endl << "Updating to the latest masterlist from the Google Code repository..." << endl;
			LOG_DEBUG("Updating masterlist...");
			try {
				unsigned int revision = UpdateMasterlist();
				if (revision == 0) {
					unsigned int localRevision = GetLocalMasterlistRevision();
					masterlistUpdateContent += "<li>masterlist.txt is already at the latest version (" + IntToString(localRevision) + "). No update necessary.</li>";
					cout << "masterlist.txt is already at the latest version (" + IntToString(localRevision) + "). No update necessary." << endl;
				} else {
					masterlistUpdateContent += "<li>masterlist.txt updated to revision " + IntToString(revision) + ".</li>";
					cout << "masterlist.txt updated to revision " << revision << endl;
				}
			} catch (boss_error & e) {
				string const * detail = boost::get_error_info<err_detail>(e);
				masterlistUpdateContent += "<li class='warn'>Error: Masterlist update failed.<br />\n";
				masterlistUpdateContent += "Details: " + *detail + "<br />\n";
				masterlistUpdateContent += "Check the Troubleshooting section of the ReadMe for more information and possible solutions.</li>\n";
			}
			LOG_DEBUG("Masterlist updated successfully.");
		} else
			masterlistUpdateContent += "<li>No internet connection detected. Masterlist auto-updater aborted.</li>";
	}

	//If true, exit BOSS now. Flush earlyBOSSlogBuffer to the bosslog and exit.
	if (update_only == true) {
		OutputHeader();
		Output("<div><span onclick='toggleSectionDisplay(this)'><span>&#x2212;</span>Masterlist Update</span><ul>\n");
		Output(masterlistUpdateContent);
		Output("</ul>\n</div>\n");
		Output("<div><span>BOSS Execution Complete</span></div>\n");
		OutputFooter();
		bosslog.close();
		if ( !silent ) 
			Launch(bosslog_path.string());	//Displays the BOSSlog.txt.
		return (0);
	}


	///////////////////////////////////
	// Resume Error Condition Checks
	///////////////////////////////////

	cout << endl << "Better Oblivion Sorting Software working..." << endl;

	//Get the master esm's modification date. 
	try {
		if (game == 1) esmtime = fs::last_write_time(data_path / "Oblivion.esm");
		else if (game == 2) esmtime = fs::last_write_time(data_path / "Fallout3.esm");
		else if (game == 3) esmtime = fs::last_write_time(data_path / "Nehrim.esm");
		else if (game == 4) esmtime = fs::last_write_time(data_path / "FalloutNV.esm");
	} catch(fs::filesystem_error e) {
		OutputHeader();
		Output("<p class='error'>Critical Error: Master .ESM file cannot be read!<br />\n");
		Output("Check the Troubleshooting section of the ReadMe for more information and possible solutions.<br />\n");
		Output("Utility will end now.</p>\n\n");
		OutputFooter();
		bosslog.close();
		LOG_ERROR("Failed to set modification time of game master file, error was: %s", e.what());
		if ( !silent ) 
			Launch(bosslog_path.string());	//Displays the BOSSlog.txt.
		exit (1); //fail in screaming heap.
	}

	//Build and save modlist.
	BuildModlist(Modlist);
	if (revert<1) {
		try {
			SaveModlist(Modlist, curr_modlist_path);
		} catch (boss_error &e) {
			string const * detail = boost::get_error_info<err_detail>(e);
			OutputHeader();
			Output("<p class='error'>Critical Error: Modlist backup failed!<br />\n");
			Output("Details: " + *detail + ".<br />\n");
			Output("Check the Troubleshooting section of the ReadMe for more information and possible solutions.<br />\n");
			Output("Utility will end now.</p>\n\n");
			OutputFooter();
			bosslog.close();
			if ( !silent ) 
				Launch(bosslog_path.string());	//Displays the BOSSlog.txt.
			exit (1); //fail in screaming heap.
		}
	}

	//Check if masterlist exists.
	if (!fs::exists(sortfile)) {         
		OutputHeader();
		Output("<p class='error'>Critical Error: \"" +sortfile.string() +"\" cannot be read!<br />\n");
		Output("Check the Troubleshooting section of the ReadMe for more information and possible solutions.<br />\n");
		Output("Utility will end now.</p>\n\n");
		OutputFooter();
        bosslog.close();
        LOG_ERROR("Couldn't open sorting file: %s", sortfile.filename().string().c_str());
        if ( !silent ) 
                Launch(bosslog_path.string());  //Displays the BOSSlog.txt.
        exit (1); //fail in screaming heap.
    }

	//Check if masterlist is valid UTF-8.
	if (!ValidateUTF8File(sortfile)) {
		OutputHeader();
		Output("<p class='error'>Critical Error: \""+sortfile.filename().string()+"\" is not encoded in valid UTF-8. Please save the file using the UTF-8 encoding.<br />\n");
		Output("Utility will end now.</p>\n\n");
		OutputFooter();
		bosslog.close();
		LOG_ERROR("File '%s' was not encoded in valid UTF-8.", sortfile.filename().string().c_str());
		if ( !silent ) 
                Launch(bosslog_path.string());  //Displays the BOSSlog.txt.
        exit (1); //fail in screaming heap.
	}


	/////////////////////////////////
	// Parse Master- and Userlists
	/////////////////////////////////
	//Masterlist parse errors are critical, ini and userlist parse errors are not.
	
	//Parse masterlist/modlist backup into data structure.
	LOG_INFO("Starting to parse sorting file: %s", sortfile.string().c_str());
	parseMasterlist(sortfile,Masterlist);

	//Check if parsing failed. If so, exit with errors.
	if (masterlistErrorBuffer.size() != 0) {
		OutputHeader();
		for (size_t i=0; i<masterlistErrorBuffer.size(); i++)  //Print parser error messages.
			Output(masterlistErrorBuffer[i]);
		OutputFooter();
		bosslog.close();
		if ( !silent ) 
                Launch(bosslog_path.string());  //Displays the BOSSlog.txt.
        exit (1); //fail in screaming heap.
	}

	//Parse userlist.
	if (revert<1 && fs::exists(userlist_path)) {
		//Validate file first.
		if (!ValidateUTF8File(userlist_path)) {
			userlistErrorBuffer.push_back("<p class='error'>Error: \""+userlist_path.filename().string()+"\" is not encoded in valid UTF-8. Please save the file using the UTF-8 encoding. Userlist parsing aborted. No rules will be applied.</p>\n\n");
			LOG_ERROR("File '%s' was not encoded in valid UTF-8.", userlist_path.filename().string().c_str());
		} else {
			LOG_INFO("Starting to parse userlist.");
			bool parsed = parseUserlist(userlist_path,Userlist);
			if (!parsed)
				Userlist.clear();  //If userlist has parsing errors, empty it so no rules are applied.
		}
	} else if (!fs::exists(userlist_path)) {
		ofstream userlist_file(userlist_path.c_str(),ios_base::binary);
		if (!userlist_file.fail()) {
			userlist_file << '\xEF' << '\xBB' << '\xBF';  //Write UTF-8 BOM to ensure the file is recognised as having the UTF-8 encoding.
		} else
			userlistErrorBuffer.push_back("<p class='error'>Error: userlist.txt generation failed. Ensure your BOSS folder is not read-only.</p>\n\n");
		userlist_file.close();
	}


	/////////////////////////////////////////////////
	// Compare Masterlist against Modlist, Userlist
	/////////////////////////////////////////////////

	//Add all modlist and userlist mods to a hashset to optimise comparison against masterlist.
	boost::unordered_set<string> hashset;  //Holds mods for checking against masterlist
	boost::unordered_set<string>::iterator setPos;
	/* Hashset must be a set of unique mods.
	Ghosted mods take priority over non-ghosted mods, as they are specifically what is installed. 
	*/

	LOG_INFO("Populating hashset with modlist.");
	for (size_t i=0; i<Modlist.size(); i++) {
		if (Modlist[i].type == MOD)
			hashset.insert(Tidy(Modlist[i].name.string()));
	}
	LOG_INFO("Populating hashset with userlist.");
	for (size_t i=0; i<Userlist.size(); i++) {
		for (size_t j=0; j<Userlist[i].lines.size(); j++) {
			if (IsPlugin(Userlist[i].lines[j].object)) {
				setPos = hashset.find(Tidy(Userlist[i].lines[j].object));
				if (setPos == hashset.end()) {  //Mod not already in hashset.
					setPos = hashset.find(Tidy(Userlist[i].lines[j].object + ".ghost"));
					if (setPos == hashset.end()) {  //Ghosted mod not already in hashset. 
						//Unique plugin, so add to hashset.
						hashset.insert(Tidy(Userlist[i].lines[j].object));
					}
				}
			}
		}
	}

	//Now compare masterlist against hashset.
	vector<item>::iterator iter = Masterlist.begin();
	vector<item> holdingVec;
	boost::unordered_set<string>::iterator addedPos;
	boost::unordered_set<string> addedMods;
	size_t pos;
	LOG_INFO("Comparing hashset against masterlist.");
	while (iter != Masterlist.end()) {
		item Item = *iter;
		if (Item.type == MOD) {
			//Check to see if the mod is in the hashset. If it is, or its ghosted version is, also check if 
			//the mod is already in the holding vector. If not, add it.
			setPos = hashset.find(Tidy(Item.name.string()));
			addedPos = addedMods.find(Tidy(Item.name.string()));
			if (setPos != hashset.end()) {										//Mod found in hashset. 
				if (addedPos == addedMods.end()) {								//The mod is not already in the holding vector.
					holdingVec.push_back(Item);									//Record it in the holding vector.
					pos = GetModPos(Modlist,Item.name.string());				//Also remove it from the Modlist.
					if (pos != (size_t)-1)
						Modlist.erase(Modlist.begin()+pos);
					addedMods.insert(Tidy(Item.name.string()));
				}
			} else {
				//Mod not found. Look for ghosted mod.
				Item.name = fs::path(Item.name.string() + ".ghost");		//Add ghost extension to mod name.
				setPos = hashset.find(Tidy(Item.name.string()));
				addedPos = addedMods.find(Tidy(Item.name.string()));
				if (setPos != hashset.end()) {									//Mod found in hashset.
					if (addedPos == addedMods.end()) {							//The mod is not already in the holding vector.
						holdingVec.push_back(Item);								//Record it in the holding vector.
						pos = GetModPos(Modlist,Item.name.string());			//Also remove it from the Modlist.
						if (pos != (size_t)-1)
							Modlist.erase(Modlist.begin()+pos);
						addedMods.insert(Tidy(Item.name.string()));
					}
				}
			}
		} else if (Item.type == REGEX) {
			//Form a regex.
			boost::regex reg(Tidy(Item.name.string())+"(.ghost)?",boost::regex::extended);  //Ghost extension is added so ghosted mods will also be found.
			//Now start looking.
			setPos = hashset.begin();
			do {
				setPos = FindRegexMatch(hashset, reg, setPos);
				if (setPos == hashset.end())  //Exit if the mod hasn't been found.
					break;
				string mod = *setPos;
				//Look for mod in modlist, and userlist. Replace with case-preserved mod name.
				pos = GetModPos(Modlist,mod);
				if (pos != (size_t)-1)
					mod = Modlist[pos].name.string();
				else {
					for (size_t i=0; i<Userlist.size(); i++) {
						for (size_t j=0; j<Userlist[i].lines.size(); j++) {
							if (Tidy(Userlist[i].lines[j].object) == mod)
								mod = Userlist[i].lines[j].object;
						}
					}
				}
				//Check that the mod hasn't already been added to the holding vector.
				addedPos = addedMods.find(Tidy(mod));
				if (addedPos == addedMods.end()) {							//The mod is not already in the holding vector.
					//Now do the adding/removing.
					//Create new temporary item to hold current found mod.
					item tempItem = Item;
					tempItem.type = MOD;
					tempItem.name = fs::path(mod);
					holdingVec.push_back(tempItem);							//Record it in the holding vector.
					pos = GetModPos(Modlist,mod);							//Also remove it from the Modlist.
					if (pos != (size_t)-1)
						Modlist.erase(Modlist.begin()+pos);
					addedMods.insert(Tidy(mod));
				}
				++setPos;
			} while (setPos != hashset.end());
		} else //Group lines must stay recorded.
			holdingVec.push_back(Item);
		++iter;
	}
	Masterlist = holdingVec;  //Masterlist now only contains the items needed to sort the user's mods.
	x = Masterlist.size()-1;  //Record position of last sorted mod.

	//Add modlist's mods to masterlist, then set the modlist to the masterlist, since that's a more sensible name to work with.
	Masterlist.insert(Masterlist.end(),Modlist.begin(),Modlist.end());
	Modlist = Masterlist;
	LOG_INFO("Modlist now filled with ordered mods and unknowns.");


	//////////////////////////
	// Apply Userlist Rules
	//////////////////////////

	//Apply userlist rules to modlist.
	if (revert<1 && fs::exists(userlist_path)) {
		//Apply rules, one rule at a time, one line at a time.
		LOG_INFO("Starting userlist sort process... Total %" PRIuS " user rules statements to process.", Userlist.size());
		for (size_t i=0; i<Userlist.size(); i++) {
			LOG_DEBUG(" -- Processing rule #%" PRIuS ".", i+1);
			for (size_t j=0; j<Userlist[i].lines.size(); j++) {
				//A mod sorting rule.
				if ((Userlist[i].lines[j].key == BEFORE || Userlist[i].lines[j].key == AFTER) && IsPlugin(Userlist[i].lines[j].object)) {
					size_t index1,index2;
					item mod;
					index1 = GetModPos(Modlist,Userlist[i].ruleObject);  //Find the rule mod in the modlist.
					mod = Modlist[index1];  //Record the rule mod in a new variable.
					//Do checks/increments.
					if (Userlist[i].ruleKey == ADD && index1 > x) 
						x++;
					//If it adds a mod already sorted, skip the rule.
					else if (Userlist[i].ruleKey == ADD  && index1 <= x) {
						userlistMessagesContent += "<li class='warn'>\""+Userlist[i].ruleObject+"\" is already in the masterlist. Rule skipped.</li>\n";
						LOG_WARN(" * \"%s\" is already in the masterlist.", Userlist[i].ruleObject.c_str());
						break;
					} else if (Userlist[i].ruleKey == OVERRIDE && index1 > x) {
						userlistMessagesContent += "<li class='error'>\""+Userlist[i].ruleObject+"\" is not in the masterlist, cannot override. Rule skipped.</li>\n";
						LOG_WARN(" * \"%s\" is not in the masterlist, cannot override.", Userlist[i].ruleObject.c_str());
						break;
					}
					//Remove the rule mod from its current position.
					Modlist.erase(Modlist.begin()+index1);
					//Find the sort mod in the modlist.
					index2 = GetModPos(Modlist,Userlist[i].lines[j].object);
					//Handle case of mods that don't exist at all.
					if (index2 == (size_t)-1) {
						if (Userlist[i].ruleKey == ADD)
							x--;
						Modlist.insert(Modlist.begin()+index1,mod);
						userlistMessagesContent += "<li class='warn'>\""+Userlist[i].lines[j].object+"\" is not installed, and is not in the masterlist. Rule skipped.</li>\n";
						LOG_WARN(" * \"%s\" is not installed or in the masterlist.", Userlist[i].lines[j].object.c_str());
						break;
					}
					//Handle the case of a rule sorting a mod into a position in unsorted mod territory.
					if (index2 > x) {
						if (Userlist[i].ruleKey == ADD)
							x--;
						Modlist.insert(Modlist.begin()+index1,mod);
						userlistMessagesContent += "<li class='error'>\""+Userlist[i].lines[j].object+"\" is not in the masterlist and has not been sorted by a rule. Rule skipped.</li>\n";
						LOG_WARN(" * \"%s\" is not in the masterlist and has not been sorted by a rule.", Userlist[i].lines[j].object.c_str());
						break;
					}
					//Insert the mod into its new position.
					if (Userlist[i].lines[j].key == AFTER) 
						index2 += 1;
					Modlist.insert(Modlist.begin()+index2,mod);
					userlistMessagesContent += "<li class='success'>\""+Userlist[i].ruleObject+"\" has been sorted "+ KeyToString(Userlist[i].lines[j].key) + " \"" + Userlist[i].lines[j].object + "\".</li>\n";
				//A group sorting line.
				} else if ((Userlist[i].lines[j].key == BEFORE || Userlist[i].lines[j].key == AFTER) && !IsPlugin(Userlist[i].lines[j].object)) {
					vector<item> group;
					size_t index1, index2;
					//Look for group to sort. Find start and end positions.
					index1 = GetModPos(Modlist, Userlist[i].ruleObject);
					index2 = GetGroupEndPos(Modlist, Userlist[i].ruleObject);
					//Check to see group actually exists.
					if (index1 == (size_t)-1 || index2 == (size_t)-1) {
						userlistMessagesContent += "<li class='error'>The group \""+Userlist[i].ruleObject+"\" is not in the masterlist or is malformatted. Rule skipped.</p>\n";
						LOG_WARN(" * \"%s\" is not in the masterlist, or is malformatted.", Userlist[i].ruleObject.c_str());
						break;
					}
					//Copy the start, end and everything in between to a new variable.
					group.assign(Modlist.begin()+index1,Modlist.begin()+index2+1);
					//Now erase group from modlist.
					Modlist.erase(Modlist.begin()+index1,Modlist.begin()+index2+1);
					//Find the group to sort relative to and insert it before or after it as appropriate.
					if (Userlist[i].lines[j].key == BEFORE)
						index2 = GetModPos(Modlist, Userlist[i].lines[j].object);  //Find the start.
					else
						index2 = GetGroupEndPos(Modlist, Userlist[i].lines[j].object);  //Find the end, and add one, as inserting works before the given element.
					//Check that the sort group actually exists.
					if (index2 == (size_t)-1) {
						Modlist.insert(Modlist.begin()+index1,group.begin(),group.end());  //Insert the group back in its old position.
						userlistMessagesContent += "<li class='error'>The group \""+Userlist[i].lines[j].object+"\" is not in the masterlist or is malformatted. Rule skipped.</li>\n";
						LOG_WARN(" * \"%s\" is not in the masterlist, or is malformatted.", Userlist[i].lines[j].object.c_str());
						break;
					}
					if (Userlist[i].lines[j].key == AFTER)
						index2++;
					//Now insert the group.
					Modlist.insert(Modlist.begin()+index2,group.begin(),group.end());
					//Print success message.
					userlistMessagesContent += "<li class='success'>The group \""+Userlist[i].ruleObject+"\" has been sorted "+ KeyToString(Userlist[i].lines[j].key) + " the group \""+Userlist[i].lines[j].object+"\".</li>\n";
				//An insertion line.
				} else if (Userlist[i].lines[j].key == TOP || Userlist[i].lines[j].key == BOTTOM) {
					size_t index1,index2;
					item mod;
					index1 = GetModPos(Modlist,Userlist[i].ruleObject);  //Find the rule mod in the modlist.
					mod = Modlist[index1];  //Record the rule mod in a new variable.
					//Do checks/increments.
					if (Userlist[i].ruleKey == ADD && index1 > x) 
						x++;
					//If it adds a mod already sorted, skip the rule.
					else if (Userlist[i].ruleKey == ADD  && index1 <= x) {
						userlistMessagesContent += "<li class='warn'>\""+Userlist[i].ruleObject+"\" is already in the masterlist. Rule skipped.</li>\n";
						LOG_WARN(" * \"%s\" is already in the masterlist.", Userlist[i].ruleObject.c_str());
						break;
					} else if (Userlist[i].ruleKey == OVERRIDE && index1 > x) {
						userlistMessagesContent += "<li class='error'>\""+Userlist[i].ruleObject+"\" is not in the masterlist, cannot override. Rule skipped.</li>\n";
						LOG_WARN(" * \"%s\" is not in the masterlist, cannot override.", Userlist[i].ruleObject.c_str());
						break;
					}
					//Remove the rule mod from its current position.
					Modlist.erase(Modlist.begin()+index1);
					//Find the position of the group to sort it to.
					//Find the group to sort relative to and insert it before or after it as appropriate.
					if (Userlist[i].lines[j].key == TOP)
						index2 = GetModPos(Modlist, Userlist[i].lines[j].object);  //Find the start.
					else
						index2 = GetGroupEndPos(Modlist, Userlist[i].lines[j].object);  //Find the end.
					//Check that the sort group actually exists.
					if (index2 == (size_t)-1) {
						Modlist.insert(Modlist.begin()+index1,mod);  //Insert the mod back in its old position.
						userlistMessagesContent += "<li class='error'>The group \""+Userlist[i].lines[j].object+"\" is not in the masterlist or is malformatted. Rule skipped.</li>\n";
						LOG_WARN(" * \"%s\" is not in the masterlist, or is malformatted.", Userlist[i].lines[j].object.c_str());
						break;
					}
					//Now insert the mod into the group.
					Modlist.insert(Modlist.begin()+index2,mod);
					//Print success message.
					userlistMessagesContent += "<li class='success'>\""+Userlist[i].ruleObject+"\" inserted at the "+ KeyToString(Userlist[i].lines[j].key) + " of group \"" + Userlist[i].lines[j].object + "\".</li>\n";
				//A message line.
				} else if (Userlist[i].lines[j].key == APPEND || Userlist[i].lines[j].key == REPLACE) {
					size_t index, pos;
					string key,data;
					message newMessage;
					//Find the mod which will have its messages edited.
					index = GetModPos(Modlist,Userlist[i].ruleObject);
					//The provided message string could be using either the old format or the new format. Need to support both.
					//The easiest way is to check the first character. If it's a symbol character, it's the old format, otherwise the new.
					char sym = Userlist[i].lines[j].object[0];
					if (sym == '?' || sym == '%' || sym == ':' || sym == '"') {  //Old format.
						//First character is the keyword, the rest is the data.
						newMessage.key = StringToKey(Userlist[i].lines[j].object.substr(0,1));
						newMessage.data = trim_copy(Userlist[i].lines[j].object.substr(1));
					} else {  //New format.
						pos = Userlist[i].lines[j].object.find(":"); //Look for separator colon.
						newMessage.key = StringToKey(Tidy(Userlist[i].lines[j].object.substr(0,pos)));
						newMessage.data = trim_copy(Userlist[i].lines[j].object.substr(pos+1));
					}
					//If the rule is to replace messages, clear existing messages.
					if (Userlist[i].lines[j].key == REPLACE)
						Modlist[index].messages.clear();
					//Append message to message list of mod.
					Modlist[index].messages.push_back(newMessage);

					//Output confirmation.
					userlistMessagesContent += "<li class='success'>\"<span class='message'>" + Userlist[i].lines[j].object + "</span>\"";
					if (Userlist[i].lines[j].key == APPEND)
						userlistMessagesContent += " appended to ";
					else
						userlistMessagesContent += " replaced ";
					userlistMessagesContent += "messages attached to \"" + Userlist[i].ruleObject + "\".</li>\n";
				}
			}
		}
		if (Userlist.empty()) 
			userlistMessagesContent = "No valid rules were found in your userlist.txt.";
		LOG_INFO("Userlist sorting process finished.");
	}

	//////////////////////////////////////////////////////
	// Print version & checksum info for OBSE & plugins
	//////////////////////////////////////////////////////

	string SE = "";
	if (show_CRCs) {
		string SELoc, SEPluginLoc;
		if (game == 1 || game == 3) {  //Oblivion/Nehrim
			SE = "OBSE";
			SELoc = "../obse_1_2_416.dll";
			SEPluginLoc = "OBSE/Plugins";
		} else if (game == 2) {  //Fallout 3
			SE = "FOSE";
			SELoc = "../fose_loader.exe";
			SEPluginLoc = "FOSE/Plugins";
		} else {  //Fallout: New Vegas
			SE = "NVSE";
			SELoc = "../nvse_loader.exe";
			SEPluginLoc = "NVSE/Plugins";
		}

		if (!fs::exists(SELoc)) {
			LOG_DEBUG("Script extender DLL not detected");
		} else {
			string CRC = IntToHexString(GetCrc32(SELoc));
			string ver = GetExeDllVersion(SELoc);

			seInfoContent += "<li><span class='mod'>" + SE + "</span>";
			if (ver.length() != 0)
				seInfoContent += "<span class='version'>Version: " + ver + "</span>";
			seInfoContent += "<span class='crc'>Checksum: " + CRC + "</span></li>\n";

			if (!fs::is_directory(data_path / SEPluginLoc)) {
				LOG_DEBUG("Script extender plugins directory not detected");
			} else {
				for (fs::directory_iterator itr(data_path / SEPluginLoc); itr!=fs::directory_iterator(); ++itr) {
					const fs::path filename = itr->path().filename();
					const string ext = Tidy(itr->path().extension().string());
					if (fs::is_regular_file(itr->status()) && ext==".dll") {
						string CRC = IntToHexString(GetCrc32(itr->path()));
						string ver = GetExeDllVersion(itr->path());

						seInfoContent += "<li><span class='mod'>" + filename.string() + "</span>";
						if (ver.length() != 0)
							seInfoContent += "<span class='version'>Version: " + ver + "</span>";
						seInfoContent += "<span class='crc'>Checksum: " + CRC + "</span></li>\n";
					}
				}
			}
		}
	}

	////////////////////////////////
	// Re-date Files & Output Info
	////////////////////////////////

	//Re-date .esp/.esm files according to order in modlist and output messages
	LOG_INFO("Applying calculated ordering to user files...");
	for (size_t i=0; i<=x; i++) {
		//Only act on mods that exist.
		if (Modlist[i].type == MOD && (Exists(data_path / Modlist[i].name))) {
			recogModContent += "<li><span class='mod'>" + TrimDotGhost(Modlist[i].name.string()) + "</span>";
			if (!skip_version_parse) {
				string version = GetModHeader(Modlist[i].name);
				if (!version.empty())
					recogModContent += "<span class='version'>Version "+version+"</span>";
			}
			if (IsGhosted(data_path / Modlist[i].name)) {
				recogModContent += "<span class='ghosted'>Ghosted</span>";
				ghostModNo++;
			}
			if (show_CRCs)
				recogModContent += "<span class='crc'>Checksum: " + IntToHexString(GetCrc32(data_path / Modlist[i].name)) + "</span>";
				
			//Now change the file's date, if it is not the game's master file.
			if (!IsMasterFile(Modlist[i].name.string()) && !trial_run) {
				//Calculate the new file time.
				modfiletime = esmtime + recModNo*60;  //time_t is an integer number of seconds, so adding 60 on increases it by a minute. Using recModNo instead of i to avoid increases for group entries.
				//Re-date file. Provide exception handling in case their permissions are wrong.
				LOG_DEBUG(" -- Setting last modified time for file: \"%s\"", Modlist[i].name.string().c_str());
				try { 
					fs::last_write_time(data_path / Modlist[i].name,modfiletime);
				} catch(fs::filesystem_error e) {
					recogModContent += " - <span class='error'>Error: Could not change the date of \"" + Modlist[i].name.string() + "\", check the Troubleshooting section of the ReadMe for more information and possible solutions.</span>";
				}
			}
			//Finally, print the mod's messages.
			if (Modlist[i].messages.size()>0) {
				recogModContent += "\n<ul>\n";
				for (size_t j=0; j<Modlist[i].messages.size(); j++) {
					ShowMessage(recogModContent, Modlist[i].messages[j]);  //Print messages to buffer.
					messageNo++;
					if (Modlist[i].messages[j].key == WARN)
						warningNo++;
					else if (Modlist[i].messages[j].key == ERR)
						errorNo++;
				}
				recogModContent += "</ul>\n</li>\n\n";
			} else
				recogModContent += "</li>\n\n";
			recModNo++;
		}
	}
	LOG_INFO("User file ordering applied successfully.");
	

	//Find and show found mods not recognised. These are the mods that are found at and after index x in the mods vector.
	//Order their dates to be i days after the master esm to ensure they load last.
	LOG_INFO("Reporting unrecognized mods...");
	for (size_t i=x+1; i<Modlist.size(); i++) {
		//Only act on mods that exist.
		if (Modlist[i].type == MOD && (Exists(data_path / Modlist[i].name))) {
			unrecogModContent += "<li><span class='mod'>" + TrimDotGhost(Modlist[i].name.string()) + "</span>";
			if (!skip_version_parse) {
				string version = GetModHeader(Modlist[i].name);
				if (!version.empty())
					unrecogModContent += "<span class='version'>Version "+version+"</span>";
			}
			if (IsGhosted(data_path / Modlist[i].name)) {
				unrecogModContent += "<span class='ghosted'>Ghosted</span>";
				ghostModNo++;
			}
			if (show_CRCs)
				unrecogModContent += "<span class='crc'>Checksum: " + IntToHexString(GetCrc32(data_path / Modlist[i].name)) + "</span>";
			
			if (!trial_run) {
				modfiletime = esmtime + 86400 + (recModNo + unrecModNo)*60;  //time_t is an integer number of seconds, so adding 60 on increases it by a minute and adding 86,400 on increases it by a day. Using unrecModNo instead of i to avoid increases for group entries.
				//Re-date file. Provide exception handling in case their permissions are wrong.
				LOG_DEBUG(" -- Setting last modified time for file: \"%s\"", Modlist[i].name.string().c_str());
				try {
					fs::last_write_time(data_path / Modlist[i].name,modfiletime);
				} catch(fs::filesystem_error e) {
					unrecogModContent += " - <span class='error'>Error: Could not change the date of \"" + Modlist[i].name.string() + "\", check the Troubleshooting section of the ReadMe for more information and possible solutions.</span>";
				}
			}
			unrecogModContent += "</li>\n";
			unrecModNo++;
		}
	}
	if (x+1 == Modlist.size())
		unrecogModContent += "<i>No unrecognised plugins.</i>";
	
	LOG_INFO("Unrecognized mods reported.");

	/////////////////////////////
	// Print Output to BOSSlog
	/////////////////////////////

	OutputHeader();  //Output BOSSlog header.

	/////////////////////////////
	// Print BOSSLog Filters
	/////////////////////////////
	
	if (log_format == "html") {
		Output("<ul class='filters'>\n");
		if (UseDarkColourScheme)
			Output("<li><input type='checkbox' checked='checked' id='b1' onclick='swapColorScheme(this)' /><label for='b1'>Use Dark Colour Scheme</label></li>\n");
		else
			Output("<li><input type='checkbox' id='b1' onclick='swapColorScheme(this)' /><label for='b1'>Use Dark Colour Scheme</label></li>\n");

		if (HideRuleWarnings)
			Output("<li><input type='checkbox' checked='checked' id='b12' onclick='toggleUserlistWarnings(this)' /><label for='b12'>Hide Rule Warnings</label></li>\n");
		else
			Output("<li><input type='checkbox' id='b12' onclick='toggleUserlistWarnings(this)' /><label for='b12'>Hide Rule Warnings</label></li>\n");
		
		if (HideVersionNumbers)
			Output("<li><input type='checkbox' checked='checked' id='b2' onclick='toggleDisplayCSS(this,\".version\",\"inline\")' /><label for='b2'>Hide Version Numbers</label></li>\n");
		else
			Output("<li><input type='checkbox' id='b2' onclick='toggleDisplayCSS(this,\".version\",\"inline\")' /><label for='b2'>Hide Version Numbers</label></li>\n");

		if (HideGhostedLabel)
			Output("<li><input type='checkbox' checked='checked' id='b3' onclick='toggleDisplayCSS(this,\".ghosted\",\"inline\")' /><label for='b3'>Hide 'Ghosted' Label</label></li>\n");
		else
			Output("<li><input type='checkbox' id='b3' onclick='toggleDisplayCSS(this,\".ghosted\",\"inline\")' /><label for='b3'>Hide 'Ghosted' Label</label></li>\n");

		if (HideChecksums)
			Output("<li><input type='checkbox' checked='checked' id='b4' onclick='toggleDisplayCSS(this,\".crc\",\"inline\")' /><label for='b4'>Hide Checksums</label></li>\n");
		else
			Output("<li><input type='checkbox' id='b4' onclick='toggleDisplayCSS(this,\".crc\",\"inline\")' /><label for='b4'>Hide Checksums</label></li>\n");

		if (HideMessagelessMods)
			Output("<li><input type='checkbox' checked='checked' id='noMessageModFilter' onclick='toggleMods()' /><label for='noMessageModFilter'>Hide Messageless Mods</label></li>\n");
		else
			Output("<li><input type='checkbox' id='noMessageModFilter' onclick='toggleMods()' /><label for='noMessageModFilter'>Hide Messageless Mods</label></li>\n");

		if (HideGhostedMods)
			Output("<li><input type='checkbox' checked='checked' id='ghostModFilter' onclick='toggleMods()' /><label for='ghostModFilter'>Hide Ghosted Mods</label></li>\n");
		else
			Output("<li><input type='checkbox' id='ghostModFilter' onclick='toggleMods()' /><label for='ghostModFilter'>Hide Ghosted Mods</label></li>\n");

		if (HideAllModMessages)
			Output("<li><input type='checkbox' checked='checked' id='b7' onclick='toggleDisplayCSS(this,\"li ul\",\"block\")' /><label for='b7'>Hide All Mod Messages</label></li>\n");
		else
			Output("<li><input type='checkbox' id='b7' onclick='toggleDisplayCSS(this,\"li ul\",\"block\")' /><label for='b7'>Hide All Mod Messages</label></li>\n");

		if (HideNotes)
			Output("<li><input type='checkbox' checked='checked' id='b8' onclick='toggleDisplayCSS(this,\".note\",\"table\")' /><label for='b8'>Hide Notes</label></li>\n");
		else
			Output("<li><input type='checkbox' id='b8' onclick='toggleDisplayCSS(this,\".note\",\"table\")' /><label for='b8'>Hide Notes</label></li>\n");

		if (HideBashTagSuggestions)
			Output("<li><input type='checkbox' checked='checked' id='b9' onclick='toggleDisplayCSS(this,\".tag\",\"table\")' /><label for='b9'>Hide Bash Tag Suggestions</label></li>\n");
		else
			Output("<li><input type='checkbox' id='b9' onclick='toggleDisplayCSS(this,\".tag\",\"table\")' /><label for='b9'>Hide Bash Tag Suggestions</label></li>\n");

		if (HideRequirements)
			Output("<li><input type='checkbox' checked='checked' id='b10' onclick='toggleDisplayCSS(this,\".req\",\"table\")' /><label for='b10'>Hide Requirements</label></li>\n");
		else
			Output("<li><input type='checkbox' id='b10' onclick='toggleDisplayCSS(this,\".req\",\"table\")' /><label for='b10'>Hide Requirements</label></li>\n");

		if (HideIncompatibilities)
			Output("<li><input type='checkbox' checked='checked' id='b11' onclick='toggleDisplayCSS(this,\".inc\",\"table\")' /><label for='b11'>Hide Incompatibilities</label></li>\n");
		else
			Output("<li><input type='checkbox' id='b11' onclick='toggleDisplayCSS(this,\".inc\",\"table\")' /><label for='b11'>Hide Incompatibilities</label></li>\n");

		Output("</ul>\n");
	}

	/////////////////////////////
	// Display Ini Parser Errors
	/////////////////////////////

	if (!iniErrorBuffer.empty()) {
		for (size_t i=0; i<iniErrorBuffer.size(); i++)  //First print parser/syntax error messages.
			Output(iniErrorBuffer[i]);
	}

	/////////////////////////////
	// Display Masterlist Update
	/////////////////////////////

	if (!masterlistUpdateContent.empty()) {
		Output("<div><span onclick='toggleSectionDisplay(this)'><span>&#x2212;</span>Masterlist Update</span><ul>\n");
		Output(masterlistUpdateContent);
		Output("</ul>\n</div>\n");
	}


	/////////////////////////////
	// Display Global Messages
	/////////////////////////////

	if (!globalMessageBuffer.empty()) {
		Output("<div><span onclick='toggleSectionDisplay(this)'><span>&#x2212;</span>General Messages</span><ul>\n");
		for (size_t i=0; i<globalMessageBuffer.size(); i++) {
			ShowMessage(textbuf, globalMessageBuffer[i]);  //Print messages.
		}
		Output(textbuf);
		Output("</ul>\n</div>\n");
	}


	/////////////////////////////
	// Print Summary
	/////////////////////////////

	Output("<div><span onclick='toggleSectionDisplay(this)'><span>&#x2212;</span>Summary</span><div id='summary'><p>\n");

	if (oldBOSSlogRecognised == recogModContent)
		Output("<b>No change in recognised mod list since last run.</b><br />\n<br />\n");
	
	Output("Mods sorted by your userlist are counted as recognised, not unrecognised, plugins.</p>\n");
	Output("<div>\n");
	Output("<div>Number of recognised plugins:</div>\n");
	Output("<div>" + IntToString(recModNo) + "</div>\n");
	Output("<div>Number of warning messages:</div>\n");
	Output("<div>" + IntToString(warningNo) + "</div>\n");
	Output("</div>\n");
	Output("<div>\n");
	Output("<div>Number of unrecognised plugins:</div>\n");
	Output("<div>" + IntToString(unrecModNo) + "</div>\n");
	Output("<div>Number of error messages:</div>\n");
	Output("<div>" + IntToString(errorNo) + "</div>\n");
	Output("</div>\n");
	Output("<div>\n");
	Output("<div>Number of ghosted plugins:</div>\n");
	Output("<div>" + IntToString(ghostModNo) + "</div>\n");
	Output("<div>Total number of messages:</div>\n");
	Output("<div>" + IntToString(messageNo) + "</div>\n");
	Output("</div>\n");
	Output("<div>\n");
	Output("<div>Total number of plugins:</div>\n");
	Output("<div>" + IntToString(recModNo+unrecModNo) + "</div>\n");
	Output("</div>\n");
	Output("</div>\n</div>\n");

		
	/////////////////////////////
	// Display Userlist Messages
	/////////////////////////////

	if (!userlistMessagesContent.empty() || !userlistErrorBuffer.empty()) {
		Output("<div><span onclick='toggleSectionDisplay(this)'><span>&#x2212;</span>Userlist Messages</span><ul id='userlistMessages'>\n");
		for (size_t i=0; i<userlistErrorBuffer.size(); i++)  //First print parser/syntax error messages.
			Output(userlistErrorBuffer[i]);
		Output(userlistMessagesContent);  //Now print the rest of the userlist messages.
		Output("</ul>\n</div>\n");
	}


	/////////////////////////////////
	// Display Script Extender Info
	/////////////////////////////////

	if (!SE.empty()) {
		Output("<div><span onclick='toggleSectionDisplay(this)'><span>&#x2212;</span>" + SE + " And " + SE + " Plugin Checksums</span><ul id='seplugins'>\n");
		Output(seInfoContent);
		Output("</ul>\n</div>\n");
	}


	/////////////////////////////////
	// Display Recognised Mods
	/////////////////////////////////

	if (revert<1) Output("<div><span onclick='toggleSectionDisplay(this)'><span>&#x2212;</span>Recognised And Re-ordered Plugins</span><ul id='recognised'>\n");
	else if (revert==1) Output("<div><span><span onclick='toggleSectionDisplay(this)'>&#x2212;</span>Restored Load Order (Using modlist.txt)</span><ul id='recognised'>\n");
	else if (revert==2) Output("<div><span><span onclick='toggleSectionDisplay(this)'>&#x2212;</span>Restored Load Order (Using modlist.old)</span><ul id='recognised'>\n");
	Output(recogModContent);
	Output("</ul>\n</div>\n");


	/////////////////////////////////
	// Display Unrecognised Mods
	/////////////////////////////////

	Output("<div><span onclick='toggleSectionDisplay(this)'><span>&#x2212;</span>Unrecognised Plugins</span><div>\n<p>Reorder these by hand using your favourite mod ordering utility.</p>\n<ul id='unrecognised'>\n");
	Output(unrecogModContent);
	Output("</ul></div>\n</div>\n");


	////////////////
	// Finish
	////////////////

	Output("<div><span>Execution Complete</span></div>\n");
	OutputFooter();
	bosslog.close();
	LOG_INFO("Launching boss log in browser.");
	if ( !silent ) 
		Launch(bosslog_path.string());	//Displays the BOSSlog.txt.
	LOG_INFO("BOSS finished.");
	return (0);
}