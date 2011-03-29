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


using namespace boss;
using namespace std;
namespace po = boost::program_options;
using boost::algorithm::trim_copy;


const string g_version     = "1.7 Dev";
const string g_releaseDate = "March 17, 2011";


void ShowVersion() {
	cout << "BOSS: Better Oblivion Sorting Software" << endl;
	cout << "Version " << g_version << " (" << g_releaseDate << ")" << endl;
}

void ShowUsage(po::options_description opts) {

	static string progName =
#if _WIN32 || _WIN64
		"BOSS";
#else
		"BOSS";
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

int main(int argc, char *argv[]) {

	size_t x=0;							//position of last recognised mod.
	string textbuf;						//a text string.
	time_t esmtime = 0, modfiletime;	//File modification times.
	int game = 0;						//What game's mods are we sorting? 1 = Oblivion, 2 = Fallout 3, 3 = Nehrim, 4 = Fallout: New Vegas.
	vector<item> Modlist, Masterlist;	//Modlist and masterlist data structures.
	vector<rule> Userlist;				//Userlist data structure.

	// set option defaults
	bool update             = false; // update masterlist?
	bool updateonly         = false; // only update the masterlist and don't sort currently.
	bool silent             = false; // silent mode?
	bool skip_version_parse = false; // enable parsing of mod's headers to look for version strings
	int revert              = 0;     // what level to revert to
	int verbosity           = 0;     // log levels above INFO to output
	string gameStr;                  // allow for autodetection override
	bool debug              = false; // whether to include origin information in logging statements
	bool showCRCs			= false; // whether or not to show mod CRCs.
	string format			= "html";  // what format the output should be in.

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
		("only-update,o", po::value(&updateonly)->zero_tokens(),
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
		("crc-display,c", po::value(&showCRCs)->zero_tokens(),
								"show mod file CRCs, so that a file's CRC can be"
								" added to the masterlist in a conditional")
		("format,f", po::value(&format),
								"select output format. valid values"
								" are: 'html', 'text'");
	

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
		if (format != "html" && format != "text") {
			LOG_ERROR("invalid option for 'format' parameter: '%s'", format.c_str());
			Fail();
		}
	
		LOG_DEBUG("BOSSlog format set to: '%s'", format.c_str());
	}

	//BOSSLog bosslog;
	fs::path bosslog_path;				//Path to BOSSlog being used.
	if (format == "html")
		bosslog_path = bosslog_html_path;
	else
		bosslog_path = bosslog_text_path;
	LOG_DEBUG("opening '%s'", bosslog_path.string().c_str());
	bosslog.open(bosslog_path.c_str());
	if (bosslog.fail()) {
		LOG_ERROR("file '%s' could not be accessed for writing. Check the"
				  " Troubleshooting section of the ReadMe for more"
				  " information and possible solutions.", bosslog_path.string().c_str());
		Fail();
	}

	
	OutputHeader(bosslog,format);  //Output HTML start and <head>
	//Output start of <body>
	Output(bosslog,format, "<div>Better Oblivion Sorting Software Log</div>\n");
	Output(bosslog,format, "<div>&copy; Random007 &amp; the BOSS development team, 2009-2010. Some rights reserved.<br />\n");
	Output(bosslog,format, "<a href=\"http://creativecommons.org/licenses/by-nc-nd/3.0/\">CC Attribution-Noncommercial-No Derivative Works 3.0</a><br />\n");
	Output(bosslog,format, "v"+g_version+" ("+g_releaseDate+")</div>\n<br />\n<br />\n");

	if (0 == game) {
		LOG_DEBUG("Detecting game...");
		if (fs::exists(data_path / "Oblivion.esm")) {
			game = 1;
			if (fs::exists(data_path / "Nehrim.esm")) {
				Output(bosslog,format, "<p class='error'>Critical Error: Oblivion.esm and Nehrim.esm have both been found!<br />\n");
				Output(bosslog,format, "Please ensure that you have installed Nehrim correctly. In a correct install of Nehrim, there is no Oblivion.esm.<br />\n");
				Output(bosslog,format, "Utility will end now.</p>\n\n</body>\n</html>");
				bosslog.close();
				LOG_ERROR("Installation error found: check BOSSLOG.");
				if ( !silent ) 
					Launch(bosslog_path.string());	//Displays the BOSSlog.txt.
				exit (1); //fail in screaming heap.
			}
		} else if (fs::exists(data_path / "Fallout3.esm")) game = 2;
		else if (fs::exists(data_path / "Nehrim.esm")) game = 3;
		else if (fs::exists(data_path / "FalloutNV.esm")) game = 4;
		else {
			LOG_ERROR("None of the supported games were detected...");
			Output(bosslog,format, "<p class='error'>Critical Error: Master .ESM file not found!<br />\n");
			Output(bosslog,format, "Check the Troubleshooting section of the ReadMe for more information and possible solutions.<br />\n");
			Output(bosslog,format, "Utility will end now.</p>\n\n</body>\n</html>");
			bosslog.close();
			if ( !silent ) 
				Launch(bosslog_path.string());	//Displays the BOSSlog.txt.
			exit (1); //fail in screaming heap.
		}
	}

	LOG_INFO("Game detected: %d", game);

	if (revert<1 && (update || updateonly)) {
		Output(bosslog,format, "<div><span>Masterlist Update</span>");
		cout << endl << "Updating to the latest masterlist from the Google Code repository..." << endl;
		LOG_DEBUG("Updating masterlist...");
		try {
			unsigned int revision = UpdateMasterlist(game);  //Need to sort out the output of this - ATM it's very messy.
			if (revision == 0) {
				Output(bosslog,format, "<p>masterlist.txt is already at the latest version. Update skipped.</p>\n\n");
				cout << "masterlist.txt is already at the latest version. Update skipped." << endl;
			} else {
				Output(bosslog,format, "<p>masterlist.txt updated to revision " + IntToString(revision) + ".</p>\n\n");
				cout << "masterlist.txt updated to revision " << revision << endl;
			}
		} catch (boss_error & e) {
			string const * detail = boost::get_error_info<err_detail>(e);
			Output(bosslog,format, "<p class='warn'>Error: Masterlist update failed.<br />\n");
			Output(bosslog,format, "Details: " + *detail + "<br />\n");
			Output(bosslog,format, "Check the Troubleshooting section of the ReadMe for more information and possible solutions.</p>\n\n");
		}
		LOG_DEBUG("Masterlist updated successfully.");
		Output(bosslog,format, "</div>\n<br />\n<br />\n");
	}

	if (updateonly == true) {
		return (0);
	}

	cout << endl << "Better Oblivion Sorting Software working..." << endl;

	//Get the master esm's modification date. 
	try {
		if (game == 1) esmtime = fs::last_write_time(data_path / "Oblivion.esm");
		else if (game == 2) esmtime = fs::last_write_time(data_path / "Fallout3.esm");
		else if (game == 3) esmtime = fs::last_write_time(data_path / "Nehrim.esm");
		else if (game == 4) esmtime = fs::last_write_time(data_path / "FalloutNV.esm");
	} catch(fs::filesystem_error e) {
		Output(bosslog,format, "<p class='error'>Critical Error: Master .ESM file cannot be read!<br />\n");
		Output(bosslog,format, "Check the Troubleshooting section of the ReadMe for more information and possible solutions.<br />\n");
		Output(bosslog,format, "Utility will end now.</p>\n\n</body>\n</html>");
		bosslog.close();
		LOG_ERROR("Failed to set modification time of game master file, error was: %s", e.what());
		if ( !silent ) 
			Launch(bosslog_path.string());	//Displays the BOSSlog.txt.
		exit (1); //fail in screaming heap.
	}

	//////////////////////////////////////////////////////
	// Print version & checksum info for OBSE & plugins
	//////////////////////////////////////////////////////

	if (showCRCs) {
		string SE, SELoc, SEPluginLoc;
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

		Output(bosslog, format, "<div><span>" + SE + " &amp; " + SE + " Plugin Versions/Checksums</span><p>");

		if (fs::exists(SELoc)) {
			string CRC = IntToHexString(GetCrc32(SELoc));
			string ver = GetExeDllVersion(SELoc);
			string text = "<b>" + SE;
			if (ver.length() != 0)
				text += " [Version: " + ver + "]";
			text += "</b> - <i>Checksum: " + CRC + "</i><br />\n<br />\n";
			Output(bosslog, format, text);
		}

		if (fs::is_directory(data_path / SEPluginLoc)) {
			for (fs::directory_iterator itr(data_path / SEPluginLoc); itr!=fs::directory_iterator(); ++itr) {
				const fs::path filename = itr->path().filename();
				const string ext = Tidy(itr->path().extension().string());
				if (fs::is_regular_file(itr->status()) && ext==".dll") {
					string CRC = IntToHexString(GetCrc32(itr->path()));
					string ver = GetExeDllVersion(itr->path());
					string text = "<b>" + filename.string();
					if (ver.length() != 0)
						text += " [Version: " + ver + "]";
					text += "</b> - <i>Checksum: " + CRC + "</i><br />\n<br />\n";
					Output(bosslog, format, text);
				}
			}
		}

		Output(bosslog, format, "</p>\n\n</div>\n<br />\n<br />\n");
	}

	//////////////////////////////////////////////
	// Parse & Build Mod-,Master- and Userlists
	//////////////////////////////////////////////

	//Build and save modlist.
	BuildModlist(Modlist);
	if (revert<1) {
		try {
			SaveModlist(Modlist, curr_modlist_path);
		} catch (boss_error &e) {
			string const * detail = boost::get_error_info<err_detail>(e);
			Output(bosslog,format, "<p class='error'>Critical Error: " + *detail + ".<br />\n");
			Output(bosslog,format, "Check the Troubleshooting section of the ReadMe for more information and possible solutions.<br />\n");
			Output(bosslog,format, "Utility will end now.</p>\n\n</body>\n</html>");
			bosslog.close();
			if ( !silent ) 
				Launch(bosslog_path.string());	//Displays the BOSSlog.txt.
			exit (1); //fail in screaming heap.
		}
	}

	//Parse masterlist/modlist backup
	fs::path sortfile;					//Modlist/masterlist to sort plugins using.
	if (revert==1)
		sortfile = curr_modlist_path;	
	else if (revert==2) 
		sortfile = prev_modlist_path;
	else 
		sortfile = masterlist_path;
	LOG_INFO("Using sorting file: %s", sortfile.string().c_str());
	//Check if it actually exists, because the parser doesn't fail if there is no file...
	if (!fs::exists(sortfile)) {                                                     
		Output(bosslog,format, "<p class='error'>Critical Error: \"" +sortfile.string() +"\" cannot be read!<br />\n");
		Output(bosslog,format, "Check the Troubleshooting section of the ReadMe for more information and possible solutions.<br />\n");
		Output(bosslog,format, "Utility will end now.</p>\n\n</body>\n</html>");
        bosslog.close();
        LOG_ERROR("Couldn't open sorting file: %s", sortfile.filename().string().c_str());
        if ( !silent ) 
                Launch(bosslog_path.string());  //Displays the BOSSlog.txt.
        exit (1); //fail in screaming heap.
    }
	//Now validate file.
	if (!ValidateUTF8File(sortfile)) {
		Output(bosslog,format, "<p class='error'>Critical Error: \""+sortfile.filename().string()+"\" is not encoded in valid UTF-8. Please save the file using the UTF-8 encoding.<br />\n");
		Output(bosslog, format, "Utility will end now.</p>\n\n</body>\n</html>");
		bosslog.close();
		LOG_ERROR("File '%s' was not encoded in valid UTF-8.", sortfile.filename().string().c_str());
		if ( !silent ) 
                Launch(bosslog_path.string());  //Displays the BOSSlog.txt.
        exit (1); //fail in screaming heap.
	}
	//Parse masterlist/modlist backup into data structure.
	bool parsed = parseMasterlist(sortfile,Masterlist);
	//Check if parsing failed - the parsed bool always returns true for some reason, so check size of errorMessageBuffer.
	if (errorMessageBuffer.size() != 0) {
		for (size_t i=0; i<errorMessageBuffer.size(); i++)  //Print parser error messages.
			Output(bosslog,format,errorMessageBuffer[i]);
		bosslog.close();
		if ( !silent ) 
                Launch(bosslog_path.string());  //Displays the BOSSlog.txt.
        exit (1); //fail in screaming heap.
	}

	//Parse userlist.
	if (revert<1 && fs::exists(userlist_path)) {
		//Validate file first.
		if (!ValidateUTF8File(userlist_path)) {
			errorMessageBuffer.push_back("<p class='error'>Critical Error: \""+userlist_path.filename().string()+"\" is not encoded in valid UTF-8. Please save the file using the UTF-8 encoding. Userlist parsing aborted. No rules will be applied.</p>\n\n");
			LOG_ERROR("File '%s' was not encoded in valid UTF-8.", userlist_path.filename().string().c_str());
		} else {
			bool parsed = parseUserlist(userlist_path,Userlist);
			if (!parsed)
				Userlist.clear();
		}
	}

	/////////////////////////////////////////////////
	// Compare Masterlist against Modlist, Userlist
	/////////////////////////////////////////////////

	//Add all modlist and userlist mods to a hashset to optimise comparison against masterlist.
	boost::unordered_set<string> hashset;  //Holds mods for checking against masterlist
	for (size_t i=0; i<Modlist.size(); i++) {
		if (Modlist[i].type == MOD)
			hashset.insert(Tidy(Modlist[i].name.string()));
	}
	for (size_t i=0; i<Userlist.size(); i++) {
		for (size_t j=0; j<Userlist[i].lines.size(); j++) {
			if (IsPlugin(Userlist[i].lines[j].object))
				hashset.insert(Tidy(Userlist[i].lines[j].object));
		}
	}

	//Now compare masterlist against hashset.
	vector<item>::iterator iter = Masterlist.begin();
	vector<item> holdingVec;
	boost::unordered_set<string>::iterator setPos;
	size_t pos;
	while (iter != Masterlist.end()) {
		item Item = *iter;
		if (Item.type == MOD) {
			setPos = hashset.find(Tidy(Item.name.string()));
			if (setPos != hashset.end()) {  //Mod found in hashset. Record it in the holding vector.
				holdingVec.push_back(Item);
				pos = GetModPos(Modlist,Item.name.string());  //Also remove it from the Modlist.
				if (pos != (size_t)-1)
					Modlist.erase(Modlist.begin()+pos);
			} else {  //Mod not found. Look for ghosted mod.
				setPos = hashset.find(Tidy(Item.name.string() + ".ghost"));
				if (setPos != hashset.end()) {  //Mod found in hashset. Record it in the holding vector, with .ghost extension.
					Item.name = fs::path(Item.name.string() + ".ghost");  //Add ghost extension to mod name.
					holdingVec.push_back(Item);
					pos = GetModPos(Modlist,Item.name.string());  //Also remove it from the Modlist.
					if (pos != (size_t)-1)
						Modlist.erase(Modlist.begin()+pos);
				}
			}
		} else //Group lines must stay recorded.
			holdingVec.push_back(Item);
		++iter;
	}
	Masterlist = holdingVec;  //Masterlist now only contains the items needed to sort the user's mods.
	x = Masterlist.size()-1;  //Record position of last sorted mod.

	//Add modlist's mods to masterlist, then set the modlist to the masterlist, since that's a more sensible name to work with.
	Masterlist.insert(Masterlist.end(),Modlist.begin(),Modlist.end());
	Modlist = Masterlist;

	//////////////////////////
	// Apply Userlist Rules
	//////////////////////////

	//Apply userlist rules to modlist.
	if (revert<1 && fs::exists(userlist_path)) {
		Output(bosslog, format, "<div><span>Userlist Messages</span><p>");

		for (size_t i=0; i<errorMessageBuffer.size(); i++)  //First print parser/syntax error messages.
			Output(bosslog,format,errorMessageBuffer[i]);

		//Now apply rules, one rule at a time, one line at a time.
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
						Output(bosslog, format, "<p class='warn'>\""+Userlist[i].ruleObject+"\" is already in the masterlist. Rule skipped.</p>\n\n");
						LOG_WARN(" * \"%s\" is already in the masterlist.", Userlist[i].ruleObject.c_str());
						break;
					} else if (Userlist[i].ruleKey == OVERRIDE && index1 > x) {
						Output(bosslog, format, "<p class='error'>\""+Userlist[i].ruleObject+"\" is not in the masterlist, cannot override. Rule skipped.</p>\n\n");
						LOG_WARN(" * \"%s\" is not in the masterlist, cannot override.", Userlist[i].ruleObject.c_str());
						break;
					}
					//Remove the rule mod from its current position.
					Modlist.erase(Modlist.begin()+index1);
					//Find the sort mod in the modlist.
					index2 = GetModPos(Modlist,Userlist[i].lines[j].object);
					//Handle case of mods that don't exist at all.
					if (index2 == (size_t)-1) {
						Modlist.insert(Modlist.begin()+index1,mod);
						Output(bosslog, format, "<p class='warn'>\""+Userlist[i].lines[j].object+"\" is not installed, and is not in the masterlist. Rule skipped.</p>\n\n");
						LOG_WARN(" * \"%s\" is not installed or in the masterlist.", Userlist[i].lines[j].object.c_str());
						break;
					}
					//Handle the case of a rule sorting a mod into a position in unsorted mod territory.
					if (index2 > x) {
						if (Userlist[i].ruleKey == ADD)
							x--;
						Modlist.insert(Modlist.begin()+index1,mod);
						Output(bosslog, format, "<p class='error'>\""+Userlist[i].lines[j].object+"\" is not in the masterlist and has not been sorted by a rule. Rule skipped.</p>\n\n");
						LOG_WARN(" * \"%s\" is not in the masterlist and has not been sorted by a rule.", Userlist[i].lines[j].object.c_str());
						break;
					}
					//Insert the mod into its new position.
					if (Userlist[i].lines[j].key == AFTER) 
						index2 += 1;
					Modlist.insert(Modlist.begin()+index2,mod);
					Output(bosslog, format, "<p class='success'>\""+Userlist[i].ruleObject+"\" has been sorted "+ KeyToString(Userlist[i].lines[j].key) + " \"" + Userlist[i].lines[j].object + "\".</p>\n\n");
				//A group sorting line.
				} else if ((Userlist[i].lines[j].key == BEFORE || Userlist[i].lines[j].key == AFTER) && !IsPlugin(Userlist[i].lines[j].object)) {
					vector<item> group;
					size_t index1, index2;
					//Look for group to sort. Find start and end positions.
					index1 = GetModPos(Modlist, Userlist[i].ruleObject);
					index2 = GetGroupEndPos(Modlist, Userlist[i].ruleObject);
					//Check to see group actually exists.
					if (index1 == (size_t)-1 || index2 == (size_t)-1) {
						Output(bosslog, format, "<p class='error'>The group \""+Userlist[i].ruleObject+"\" is not in the masterlist or is malformatted. Rule skipped.</p>\n\n");
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
						index2 = GetGroupEndPos(Modlist, Userlist[i].lines[j].object)+1;  //Find the end, and add one, as inserting works before the given element.
					//Check that the sort group actually exists.
					if (index2 == (size_t)-1) {
						Modlist.insert(Modlist.begin()+index1,group.begin(),group.end());  //Insert the group back in its old position.
						Output(bosslog, format, "<p class='error'>The group \""+Userlist[i].lines[j].object+"\" is not in the masterlist or is malformatted. Rule skipped.</p>\n\n");
						LOG_WARN(" * \"%s\" is not in the masterlist, or is malformatted.", Userlist[i].lines[j].object.c_str());
						break;
					}
					//Now insert the group.
					Modlist.insert(Modlist.begin()+index2,group.begin(),group.end());
					//Print success message.
					Output(bosslog, format, "<p class='success'>The group \""+Userlist[i].ruleObject+"\" has been sorted "+ KeyToString(Userlist[i].lines[j].key) + " the group \""+Userlist[i].lines[j].object+"\".</p>\n\n");
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
						Output(bosslog, format, "<p class='warn'>\""+Userlist[i].ruleObject+"\" is already in the masterlist. Rule skipped.</p>\n\n");
						LOG_WARN(" * \"%s\" is already in the masterlist.", Userlist[i].ruleObject.c_str());
						break;
					} else if (Userlist[i].ruleKey == OVERRIDE && index1 > x) {
						Output(bosslog, format, "<p class='error'>\""+Userlist[i].ruleObject+"\" is not in the masterlist, cannot override. Rule skipped.</p>\n\n");
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
						Output(bosslog, format, "<p class='error'>The group \""+Userlist[i].lines[j].object+"\" is not in the masterlist or is malformatted. Rule skipped.</p>\n\n");
						LOG_WARN(" * \"%s\" is not in the masterlist, or is malformatted.", Userlist[i].lines[j].object.c_str());
						break;
					}
					//Now insert the mod into the group.
					Modlist.insert(Modlist.begin()+index2,mod);
					//Print success message.
					Output(bosslog, format, "<p class='success'>\""+Userlist[i].ruleObject+"\" inserted at the "+ KeyToString(Userlist[i].lines[j].key) + " of group \"" + Userlist[i].lines[j].object + "\".</p>\n\n");
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
					Output(bosslog, format, "<p class='success'>\"" + Userlist[i].lines[j].object + "\"");
					if (Userlist[i].lines[j].key == APPEND)
						Output(bosslog, format, " appended to ");
					else
						Output(bosslog, format, " replaced ");
					Output(bosslog, format, "messages attached to \"" + Userlist[i].ruleObject + "\".</p>\n\n");
				}
			}
		}
		if (Userlist.size() == 0) 
			Output(bosslog, format, "No valid rules were found in your userlist.txt.");
		Output(bosslog, format, "</p>\n\n</div>\n<br />\n<br />\n");
		LOG_INFO("Userlist sorting process finished.");
	}

	/////////////////////////////
	// Display Global Messages
	/////////////////////////////

	if (globalMessageBuffer.size() > 0) {
		Output(bosslog, format, "<div><span>General Messages</span><p>\n<ul>\n");
		for (size_t i=0; i<globalMessageBuffer.size(); i++)
			ShowMessage(bosslog, format, globalMessageBuffer[i]);  //Print messages.
		Output(bosslog, format, "</ul>\n</p>\n</div>\n<br />\n<br />\n");
	}

	////////////////////////////////
	// Re-date Files & Output Info
	////////////////////////////////

	//Re-date .esp/.esm files according to order in modlist and output messages
	if (revert<1) Output(bosslog, format, "<div><span>Recognised And Re-ordered Mod Files</span><p>");
	else if (revert==1) Output(bosslog, format, "<div><span>Restored Load Order (Using modlist.txt)</span><p>");
	else if (revert==2) Output(bosslog, format, "<div><span>Restored Load Order (Using modlist.old)</span><p>");

	LOG_INFO("Applying calculated ordering to user files...");
	for (size_t i=0; i<=x; i++) {
		//Only act on mods that exist.
		if (Modlist[i].type == MOD && (Exists(data_path / Modlist[i].name))) {
			string text = "<b>" + TrimDotGhost(Modlist[i].name.string());
			if (!skip_version_parse) {
				string version = GetModHeader(Modlist[i].name);
				if (!version.empty())
					text += " <span class='version'>[Version "+version+"]</span>";
			}
			text += "</b>";
			if (IsGhosted(data_path / Modlist[i].name)) 
				text += " <span class='ghosted'> - Ghosted</span>";
			if (showCRCs)
				text += "<i> - Checksum: " + IntToHexString(GetCrc32(data_path / Modlist[i].name)) + "</i>";
			Output(bosslog, format, text); 
				
			//Now change the file's date, if it is not the game's master file.
			if (!IsMasterFile(Modlist[i].name.string())) {
				//Calculate the new file time.
				modfiletime=esmtime;
				modfiletime += i*60; //time_t is an integer number of seconds, so adding 60 on increases it by a minute.
				//Re-date file. Provide exception handling in case their permissions are wrong.
				LOG_DEBUG(" -- Setting last modified time for file: \"%s\"", Modlist[i].name.string().c_str());
				try { 
					fs::last_write_time(data_path / Modlist[i].name,modfiletime);
				} catch(fs::filesystem_error e) {
					Output(bosslog, format, " - <span class='error'>Error: Could not change the date of \"" + Modlist[i].name.string() + "\", check the Troubleshooting section of the ReadMe for more information and possible solutions.</span>");
				}
			}
			//Finally, print the mod's messages.
			if (Modlist[i].messages.size()>0) {
				Output(bosslog, format, "\n<ul>\n");
				for (size_t j=0; j<Modlist[i].messages.size(); j++)
					ShowMessage(bosslog, format,Modlist[i].messages[j]);  //Print messages.
				Output(bosslog, format, "</ul>\n");
			} else
				Output(bosslog, format, "<br />\n<br />\n");
		}
	}
	Output(bosslog, format, "</p>\n\n</div>\n<br />\n<br />\n");
	LOG_INFO("User file ordering applied successfully.");
	

	//Find and show found mods not recognised. These are the mods that are found at and after index x in the mods vector.
	//Order their dates to be i days after the master esm to ensure they load last.
	Output(bosslog, format, "<div><span>Unrecogised Mod Files</span><p>Reorder these by hand using your favourite mod ordering utility.</p>\n\n<p>");
	LOG_INFO("Reporting unrecognized mods...");
	for (size_t i=x+1; i<Modlist.size(); i++) {
		//Only act on mods that exist.
		if (Modlist[i].type == MOD && (Exists(data_path / Modlist[i].name))) {
			string text = "Unknown mod file: " + TrimDotGhost(Modlist[i].name.string());
			if (IsGhosted(data_path / Modlist[i].name)) 
				text += " <span class='ghosted'> - Ghosted</span>";
			if (showCRCs)
				text += "<i> - Checksum: " + IntToHexString(GetCrc32(data_path / Modlist[i].name)) + "</i>";
			Output(bosslog, format, text); 

			modfiletime=esmtime;
			modfiletime += i*60; //time_t is an integer number of seconds, so adding 60 on increases it by a minute.
			modfiletime += i*86400; //time_t is an integer number of seconds, so adding 86,400 on increases it by a day.
			//Re-date file. Provide exception handling in case their permissions are wrong.
			LOG_DEBUG(" -- Setting last modified time for file: \"%s\"", Modlist[i].name.string().c_str());
			try { 
				fs::last_write_time(data_path / Modlist[i].name,modfiletime);
			} catch(fs::filesystem_error e) {
				Output(bosslog, format, " - <span class='error'>Error: Could not change the date of \"" + Modlist[i].name.string() + "\", check the Troubleshooting section of the ReadMe for more information and possible solutions.</span>");
			}
			Output(bosslog, format, "<br />\n");
		}
	}
	Output(bosslog, format, "</p>\n\n</div>\n<br />\n<br />\n");
	LOG_INFO("Unrecognized mods reported.");
	
	//Let people know the program has stopped.
	Output(bosslog, format, "<div><span>BOSS Execution Complete</span></div>\n</body>\n</html>");
	bosslog.close();
	LOG_INFO("Launching boss log in browser.");
	if ( !silent ) 
		Launch(bosslog_path.string());	//Displays the BOSSlog.txt.
	LOG_INFO("BOSS finished.");
	return (0);
}