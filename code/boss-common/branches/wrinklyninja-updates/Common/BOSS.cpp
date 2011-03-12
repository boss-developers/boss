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

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <cstring>
#include <string>
#include <ctype.h>
#include <time.h>



using namespace boss;
using namespace std;
namespace po = boost::program_options;
using boost::algorithm::trim_copy;
using boost::algorithm::to_lower_copy;


const string g_version     = "1.7";
const string g_releaseDate = "February 18, 2011";


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
	string textbuf;                 //a line of text from a file (should usually end up being be a file name);
	time_t esmtime = 0, modfiletime;	//File modification times.
	int game = 0;						//What game's mods are we sorting? 1 = Oblivion, 2 = Fallout 3, 3 = Nehrim, 4 = Fallout: New Vegas.

	// set option defaults
	bool update             = false; // update masterlist?
	bool updateonly         = false; // only update the masterlist and don't sort currently.
	bool silent             = false; // silent mode?
	bool skip_version_parse = false; // enable parsing of mod's headers to look for version strings
	int revert              = 0;     // what level to revert to
	int verbosity           = 0;     // log levels above INFO to output
	string gameStr;                  // allow for autodetection override
	bool debug              = false; // whether to include origin information in logging statements
	formatType format		= HTML;  // what format the output should be in.

	//Set the locale to get encoding conversions working correctly.
	setlocale(LC_CTYPE, "");
	std::locale global_loc = std::locale();
	std::locale loc(global_loc, new boost::filesystem::detail::utf8_codecvt_facet());
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
								" 'Fallout3NV'")
		("debug,d", po::value(&debug)->zero_tokens(),
								"add source file references to logging statements");

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
		else if (boost::iequals("Fallout3NV", gameStr)) { game = 4; }
		else {
			LOG_ERROR("invalid option for 'game' parameter: '%s'", gameStr.c_str());
			Fail();
		}
	
		LOG_DEBUG("game autodectection overridden with: '%s' (%d)", gameStr.c_str(), game);
	}

	//BOSSLog bosslog;
	LOG_DEBUG("opening '%s'", bosslog_path.string().c_str());
	bosslog.open(bosslog_path.c_str());
	if (bosslog.fail()) {
		LOG_ERROR("file '%s' could not be accessed for writing; check the"
				  " Troubleshooting section of the ReadMe for more"
				  " information and possible solutions", bosslog_path.string().c_str());
		Fail();
	}

	//Output HTML start and <head>
	OutputHeader(bosslog,format);
	//Output start of <body>
	Output(bosslog,format, "<div id='title'>Better Oblivion Sorting Software Log</div><br />");
	Output(bosslog,format, "<div style='text-align:center;'>&copy; Random007 &amp; the BOSS development team, 2009-2010. Some rights reserved.<br />");
	Output(bosslog,format, "<a href='http://creativecommons.org/licenses/by-nc-nd/3.0/'>CC Attribution-Noncommercial-No Derivative Works 3.0</a><br />");
	Output(bosslog,format, "v"+g_version+" ("+g_releaseDate+")</div><br /><br />");
	//Output(bosslog,format, );

	if (0 == game) {
		LOG_DEBUG("Detecting game...");
		if (fs::exists(data_path / "Oblivion.esm")) {
			game = 1;
			if (fs::exists(data_path / "Nehrim.esm")) {
				Output(bosslog,format, "<p class='error'>Critical Error: Oblivion.esm and Nehrim.esm have both been found!<br />");
				Output(bosslog,format, "Please ensure that you have installed Nehrim correctly. In a correct install of Nehrim, there is no Oblivion.esm.<br />");
				Output(bosslog,format, "Utility will end now.</p>");
				Output(bosslog,format, "</body></html>");
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
			bosslog << endl << "<p class='error'>Critical Error: Master .ESM file not found!<br />" << endl
							<< "Check the Troubleshooting section of the ReadMe for more information and possible solutions.<br />" << endl
							<< "Utility will end now.</p>" << endl
							<< "</body>"<<endl<<"</html>";
			bosslog.close();
			if ( !silent ) 
				Launch(bosslog_path.string());	//Displays the BOSSlog.txt.
			exit (1); //fail in screaming heap.
		}
	}

	LOG_INFO("Game detected: %d", game);

	if (update || updateonly) {
		bosslog << "<div><span>Masterlist Update</span>"<<endl<<"<p>";
		cout << endl << "Updating to the latest masterlist from the Google Code repository..." << endl;
		LOG_DEBUG("Updating masterlist...");
		try {
			int revision = UpdateMasterlist(game);  //Need to sort out the output of this - ATM it's very messy.
			if (revision == 0) {
				bosslog << "masterlist.txt is already at the latest version. Update skipped.<br />" << endl;
				cout << "masterlist.txt is already at the latest version. Update skipped." << endl;
			} else {
				bosslog << "masterlist.txt updated to revision " << revision << "<br />" << endl;
				cout << "masterlist.txt updated to revision " << revision << endl;
			}
		} catch (boss_error & e) {
			string const * detail = boost::get_error_info<err_detail>(e);
			bosslog << "Error: Masterlist update failed!<br />" << endl;
			bosslog << "Details: " << *detail << "<br />" << endl;
			bosslog << "Check the Troubleshooting section of the ReadMe for more information and possible solutions.<br />" << endl;
		}
		LOG_DEBUG("Masterlist updated successfully.");
		bosslog <<"</p>"<<endl<<"</div><br /><br />"<<endl;
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
		bosslog << endl << "<p class='error'>Critical Error: Master .ESM file cannot be read!<br />" << endl
						<< "Check the Troubleshooting section of the ReadMe for more information and possible solutions.<br />" << endl
						<< "Utility will end now.</p>" << endl
						<< "</body>"<<endl<<"</html>";
		bosslog.close();
		LOG_ERROR("Failed to set modification time of game master file, error was: %s", e.what());
		if ( !silent ) 
			Launch(bosslog_path.string());	//Displays the BOSSlog.txt.
		exit (1); //fail in screaming heap.
	}

	//Build and save modlist.
	vector<item> Modlist;
	BuildModlist(Modlist);
	if (revert<1) {
		try {
			SaveModlist(Modlist, curr_modlist_path);
		} catch (boss_error &e) {
			string const * detail = boost::get_error_info<err_detail>(e);
			bosslog << "Critical Error: " << *detail << "<br />" << endl;
			bosslog << "Check the Troubleshooting section of the ReadMe for more information and possible solutions.<br />" << endl
					<< "Utility will end now.</p>" << endl
					<< "</body>"<<endl<<"</html>";
			bosslog.close();
			if ( !silent ) 
				Launch(bosslog_path.string());	//Displays the BOSSlog.txt.
			exit (1); //fail in screaming heap.
		}
	}

	//Parse userlist.
	vector<rule> userlistRules;
	if (revert<1 && fs::exists(userlist_path)) {
		bool parsed = parseUserlist(userlist_path,userlistRules);
		if (!parsed) {
			cout << "Userlist parse error!" << endl;
		} else {
			cout << "Userlist parsed successfully!" << endl;
		}
	/*	cout << "Obtained:" << endl;
		for (size_t i=0; i<userlistRules.size(); i++) {
			cout << GetKeyString(userlistRules[i].ruleKey) << ":" << userlistRules[i].ruleObject << endl;
			for (size_t j=0; j<userlistRules[i].lines.size(); j++)
				cout << GetKeyString(userlistRules[i].lines[j].key) << ":" << userlistRules[i].lines[j].object << endl;
			cout << endl;
		}*/
	}

	//Parse masterlist/modlist backup
	vector<item> Masterlist;
	fs::path sortfile;
	if (revert==1)
		sortfile = curr_modlist_path;	
	else if (revert==2) 
		sortfile = prev_modlist_path;
	else 
		sortfile = masterlist_path;
	LOG_INFO("Using sorting file: %s", sortfile.string().c_str());
	//Check if it actually exists, because the parser doesn't fail if there is no file...
	if (!fs::exists(sortfile)) {                                                     
                bosslog << endl << "<p class='error'>Critical Error: ";
                bosslog << sortfile.string();
                bosslog << " cannot be read!<br />" << endl
                        << "Check the Troubleshooting section of the ReadMe for more information and possible solutions.<br />" << endl
                        << "Utility will end now.</p>" << endl
                        << "</body>"<<endl<<"</html>";
                bosslog.close();
                LOG_ERROR("Couldn't open sorting file: %s", sortfile.filename().string().c_str());
                if ( !silent ) 
                        Launch(bosslog_path.string());  //Displays the BOSSlog.txt.
                exit (1); //fail in screaming heap.
    }

	//Parse masterlist/modlist backup into data structure.
	bool parsed = parseMasterlist(sortfile,Masterlist);
	if (!parsed) {
		cout << "Masterlist parse error!" << endl;
	} else {
		cout << "Masterlist parsed successfully!" << endl;
	}
	/*cout << "Obtained:" << endl;
	for (size_t i = 0; i<Masterlist.size(); i++) {
		cout << GetTypeString(Masterlist[i].type) << ":" << Masterlist[i].name.string() << endl;
		for (size_t j=0; j<Masterlist[i].messages.size(); j++)
			cout << GetKeyString(Masterlist[i].messages[j].key) << ":" << Masterlist[i].messages[j].data << endl;
	}*/

	/*Need to compare masterlist against modlist and userlist and:
	1. Remove mods in the masterlist that are not in the userlist or modlist.
	2. Remove mods in the modlist that are in the masterlist, to separate out unknowns.*/
	
	//To optimise this, create a hashset of all the mods in the userlist and modlist.
	boost::unordered_set<string> hashset;
	//Add all modlist mods to hashset.
	for (size_t i=0; i<Modlist.size(); i++) {
		if (Modlist[i].type == MOD)
			hashset.insert(Tidy(Modlist[i].name.string()));
	}
	//Add all userlist mods to hashset.
	for (size_t i=0; i<userlistRules.size(); i++) {
		for (size_t j=0; j<userlistRules[i].lines.size(); j++) {
			if (IsPlugin(userlistRules[i].lines[j].object))
				hashset.insert(Tidy(userlistRules[i].lines[j].object));
		}
	}

	//Now compare masterlist against hashmap.
	vector<item>::iterator iter = Masterlist.begin();
	vector<item> holdingVec;
	while (iter != Masterlist.end()) {
		item Item = *iter;
		if (Item.type == MOD) {
			boost::unordered_set<string>::iterator pos = hashset.find(Tidy(Item.name.string()));  //Look for mods.
			//We also want to remove those mods from the Modlist.
			if (pos != hashset.end()) {  //Mod found in hashset. Record it in the holding vector.
				holdingVec.push_back(Item);
				//Also remove it from the Modlist.
				size_t pos = GetModPos(Modlist,Item.name.string());
				if (pos != (size_t)-1)
					Modlist.erase(Modlist.begin()+pos);
			}
		} else //Group lines must stay recorded.
			holdingVec.push_back(Item);
		++iter;
	}
	Masterlist = holdingVec;
	//Record position of last sorted mod.
	x = holdingVec.size()-1;

	/*Holding vector now contains:
	1. All group markers.
	2. Mods recognised by BOSS that are either installed or referenced to in the userlist (or both), in their sorted order.
	
	Modlist now contains only unrecognised mods that are installed.
	Hence to complete the mod set, the modlist just needs to be appended to the holding vector.
	*/

	//Add modlist's mods to holding vector, then set the modlist to the holding vector, since that's a more sensible name to work with.
	holdingVec.insert(holdingVec.end(),Modlist.begin(),Modlist.end());
	Modlist = holdingVec;

	//Debug - Output contents of modlist.
	/*for (size_t i = 0; i<Modlist.size(); i++) {
		cout << GetTypeString(Modlist[i].type) << ":" << Modlist[i].name.string() << endl;
		for (size_t j=0; j<Modlist[i].messages.size(); j++)
			cout << GetKeyString(Modlist[i].messages[j].key) << ":" << Modlist[i].messages[j].data << endl;
	}*/

	//Apply userlist rules to modlist.
	if (revert<1 && fs::exists(userlist_path)) {
		bosslog << "<div><span>Userlist Messages</span>"<<endl<<"<p>";
		//Go through each rule.
		LOG_INFO("Starting userlist sort process... Total %" PRIuS " user rules statements to process.", userlistRules.size());
		for (size_t i=0; i<userlistRules.size(); i++) {
			//Go through each line of the rule.
			LOG_DEBUG(" -- Processing rule #%" PRIuS ".", i+1);
			for (size_t j=0; j<userlistRules[i].lines.size(); j++) {
				//A mod sorting rule.
				if ((userlistRules[i].lines[j].key == BEFORE || userlistRules[i].lines[j].key == AFTER) && IsPlugin(userlistRules[i].lines[j].object)) {
					size_t index1,index2;
					item mod;
					index1 = GetModPos(Modlist,userlistRules[i].ruleObject);  //Find the rule mod in the modlist.
					mod = Modlist[index1];  //Record the rule mod in a new variable.
					//Do checks/increments.
					if (userlistRules[i].ruleKey == ADD && index1 > x) 
						x++;
					//If it adds a mod already sorted, skip the rule.
					else if (userlistRules[i].ruleKey == ADD  && index1 <= x) {
						bosslog << "<span class='warn'>\""+userlistRules[i].ruleObject+"\" is already in the masterlist. Rule skipped.</span><br /><br />";
						LOG_WARN(" * \"%s\" is already in the masterlist.", userlistRules[i].ruleObject.c_str());
						break;
					} else if (userlistRules[i].ruleKey == OVERRIDE && index1 > x) {
						bosslog << "<span class='error'>\""+userlistRules[i].ruleObject+"\" is not in the masterlist, cannot override. Rule skipped.</span><br /><br />";
						LOG_WARN(" * \"%s\" is not in the masterlist, cannot override.", userlistRules[i].ruleObject.c_str());
						break;
					}
					//Remove the rule mod from its current position.
					Modlist.erase(Modlist.begin()+index1);
					//Find the sort mod in the modlist.
					index2 = GetModPos(Modlist,userlistRules[i].lines[j].object);
					//Handle case of mods that don't exist at all.
					if (index2 == (size_t)-1) {
						Modlist.insert(Modlist.begin()+index1,mod);
						bosslog << "<span class='warn'>\""+userlistRules[i].lines[j].object+"\" is not installed, and is not in the masterlist. Rule skipped.</span><br /><br />";
						LOG_WARN(" * \"%s\" is not installed or in the masterlist.", userlistRules[i].lines[j].object.c_str());
						break;
					}
					//Handle the case of a rule sorting a mod into a position in unsorted mod territory.
					if (index2 > x) {
						if (userlistRules[i].ruleKey == ADD)
							x--;
						Modlist.insert(Modlist.begin()+index1,mod);
						bosslog << "<span class='error'>\""+userlistRules[i].lines[j].object+"\" is not in the masterlist and has not been sorted by a rule. Rule skipped.</span><br /><br />";
						LOG_WARN(" * \"%s\" is not in the masterlist and has not been sorted by a rule.", userlistRules[i].lines[j].object.c_str());
						break;
					}
					//Insert the mod into its new position.
					if (userlistRules[i].lines[j].key == AFTER) 
						index2 += 1;
					Modlist.insert(Modlist.begin()+index2,mod);
					if (userlistRules[i].lines[j].key == AFTER) 
						bosslog << "<span class='success'>\""+userlistRules[i].ruleObject+"\" has been sorted after \"" + userlistRules[i].lines[j].object + "\".</span><br /><br />";
					else
						bosslog << "<span class='success'>\""+userlistRules[i].ruleObject+"\" has been sorted before \"" + userlistRules[i].lines[j].object + "\".</span><br /><br />";
				//A group sorting line.
				} else if ((userlistRules[i].lines[j].key == BEFORE || userlistRules[i].lines[j].key == AFTER) && !IsPlugin(userlistRules[i].lines[j].object)) {
					vector<item> group;
					size_t index1, index2;
					//Look for group to sort. Find start and end positions.
					index1 = GetModPos(Modlist, userlistRules[i].ruleObject);
					index2 = GetGroupEndPos(Modlist, userlistRules[i].ruleObject);
					//Check to see group actually exists.
					if (index1 == (size_t)-1 || index2 == (size_t)-1) {
						bosslog << "<span class='error'>The group \""+userlistRules[i].ruleObject+"\" is not in the masterlist or is malformatted. Rule skipped.</span><br /><br />";
						LOG_WARN(" * \"%s\" is not in the masterlist, or is malformatted.", userlistRules[i].ruleObject.c_str());
						break;
					}
					//Copy the start, end and everything in between to a new variable.
					group.assign(Modlist.begin()+index1,Modlist.begin()+index2+1);
					//Now erase group from modlist.
					Modlist.erase(Modlist.begin()+index1,Modlist.begin()+index2+1);
					//Find the group to sort relative to and insert it before or after it as appropriate.
					if (userlistRules[i].lines[j].key == BEFORE)
						index2 = GetModPos(Modlist, userlistRules[i].lines[j].object);  //Find the start.
					else
						index2 = GetGroupEndPos(Modlist, userlistRules[i].lines[j].object)+1;  //Find the end, and add one, as inserting works before the given element.
					//Check that the sort group actually exists.
					if (index2 == (size_t)-1) {
						Modlist.insert(Modlist.begin()+index1,group.begin(),group.end());  //Insert the group back in its old position.
						bosslog << "<span class='error'>The group \""+userlistRules[i].lines[j].object+"\" is not in the masterlist or is malformatted. Rule skipped.</span><br /><br />";
						LOG_WARN(" * \"%s\" is not in the masterlist, or is malformatted.", userlistRules[i].lines[j].object.c_str());
						break;
					}
					//Now insert the group.
					Modlist.insert(Modlist.begin()+index2,group.begin(),group.end());
					//Print success message.
					if (userlistRules[i].lines[j].key == AFTER)
						bosslog << "<span class='success'>The group \""+userlistRules[i].ruleObject+"\" has been sorted after the group \""+userlistRules[i].lines[j].object+"\".</span><br /><br />";
					else
						bosslog << "<span class='success'>The group \""+userlistRules[i].ruleObject+"\" has been sorted before the group \""+userlistRules[i].lines[j].object+"\".</span><br /><br />";
				//An insertion line.
				} else if (userlistRules[i].lines[j].key == TOP || userlistRules[i].lines[j].key == BOTTOM) {
					size_t index1,index2;
					item mod;
					index1 = GetModPos(Modlist,userlistRules[i].ruleObject);  //Find the rule mod in the modlist.
					mod = Modlist[index1];  //Record the rule mod in a new variable.
					//Do checks/increments.
					if (userlistRules[i].ruleKey == ADD && index1 > x) 
						x++;
					//If it adds a mod already sorted, skip the rule.
					else if (userlistRules[i].ruleKey == ADD  && index1 <= x) {
						bosslog << "<span class='warn'>\""+userlistRules[i].ruleObject+"\" is already in the masterlist. Rule skipped.</span><br /><br />";
						LOG_WARN(" * \"%s\" is already in the masterlist.", userlistRules[i].ruleObject.c_str());
						break;
					} else if (userlistRules[i].ruleKey == OVERRIDE && index1 > x) {
						bosslog << "<span class='error'>\""+userlistRules[i].ruleObject+"\" is not in the masterlist, cannot override. Rule skipped.</span><br /><br />";
						LOG_WARN(" * \"%s\" is not in the masterlist, cannot override.", userlistRules[i].ruleObject.c_str());
						break;
					}
					//Remove the rule mod from its current position.
					Modlist.erase(Modlist.begin()+index1);
					//Find the position of the group to sort it to.
					//Find the group to sort relative to and insert it before or after it as appropriate.
					if (userlistRules[i].lines[j].key == TOP)
						index2 = GetModPos(Modlist, userlistRules[i].lines[j].object);  //Find the start.
					else
						index2 = GetGroupEndPos(Modlist, userlistRules[i].lines[j].object);  //Find the end.
					//Check that the sort group actually exists.
					if (index2 == (size_t)-1) {
						Modlist.insert(Modlist.begin()+index1,mod);  //Insert the mod back in its old position.
						bosslog << "<span class='error'>The group \""+userlistRules[i].lines[j].object+"\" is not in the masterlist or is malformatted. Rule skipped.</span><br /><br />";
						LOG_WARN(" * \"%s\" is not in the masterlist, or is malformatted.", userlistRules[i].lines[j].object.c_str());
						break;
					}
					//Now insert the mod into the group.
					Modlist.insert(Modlist.begin()+index2,mod);
					//Print success message.
					if (userlistRules[i].lines[j].key == BOTTOM)
						bosslog << "<span class='success'>\""+userlistRules[i].ruleObject+"\" inserted at the end of group \"" + userlistRules[i].lines[j].object + "\".</span><br /><br />";
					else
						bosslog << "<span class='success'>\""+userlistRules[i].ruleObject+"\" inserted at the start of group \"" + userlistRules[i].lines[j].object + "\".</span><br /><br />";
				//A message line.
				} else if (userlistRules[i].lines[j].key == APPEND || userlistRules[i].lines[j].key == REPLACE) {
					size_t index, pos;
					string key,data;
					message newMessage;
					//Find the mod which will have its messages edited.
					index = GetModPos(Modlist,userlistRules[i].ruleObject);
					//Split the provided message string into a keyword and a data string.
					pos = userlistRules[i].lines[j].object.find(":");
					if (pos!=string::npos) {
						newMessage.key = GetStringKey(Tidy(userlistRules[i].lines[j].object.substr(0,pos)));
						newMessage.data = trim_copy(userlistRules[i].lines[j].object.substr(pos+1));
					}
					//If the rule is to replace messages, clear existing messages.
					if (userlistRules[i].lines[j].key == REPLACE)
						Modlist[index].messages.clear();
					//Append message to message list of mod.
					Modlist[index].messages.push_back(newMessage);

					//Output confirmation.
					bosslog << "<span class='success'>\"" + userlistRules[i].lines[j].object + "\"";
					if (userlistRules[i].lines[j].key == APPEND)
						bosslog << " appended to ";
					else
						bosslog << " replaced ";
					bosslog << "messages attached to \"" + userlistRules[i].ruleObject + "\".</span><br /><br />";
				}
			}
		}
		if (userlistRules.size()==0) 
			bosslog << "No valid rules were found in your userlist.txt.<br />" << endl;
		bosslog <<"</p>"<<endl<<"</div><br /><br />"<<endl;
		LOG_INFO("Userlist sorting process finished.");
	}

	//Re-date .esp/.esm files according to order in modlist and output messages
	if (revert<1) bosslog << "<div><span>Recognised And Re-ordered Mod Files</span>"<<endl<<"<p>"<<endl;
	else if (revert==1) bosslog << "<div><span>Restored Load Order (Using modlist.txt)</span>"<<endl<<"<p>"<<endl;
	else if (revert==2) bosslog << "<div><span>Restored Load Order (Using modlist.old)</span>"<<endl<<"<p>"<<endl;

	//x = min(int(x), int(Modlist.size()));  //Not sure why this is needed.

	LOG_INFO("Applying calculated ordering to user files...");
	for (size_t i=0;i<=x;i++) {
		//Only act on mods that exist.
		if (Modlist[i].type == MOD && (fs::exists(data_path / Modlist[i].name) || fs::exists(data_path / fs::path(Modlist[i].name.string()+".ghost")))) {
			bool ghosted = false;
			fs::path name;
			string version;
			if (Tidy(Modlist[i].name.string().substr(Modlist[i].name.string().length()-6))==".ghost") {
				ghosted = true;
				name = fs::path(Modlist[i].name.string().substr(Modlist[i].name.string().length()-6));
			} else 
				name = Modlist[i].name;
			//Start outputting stuff to log.
			string text = "<b>"+name.string();
			if (!skip_version_parse) {
				version = GetModHeader(name, ghosted);
				if (!version.empty())
					text += " <span class='version'>[Version "+version+"]</span>";
			}
			text += "</b>";
			if (ghosted) 
				text += " <span class='ghosted'> - Ghosted</span>";
			bosslog << text;
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
					bosslog << " - <span class='error'>Error: Could not change the date of \"" << Modlist[i].name.string() << "\", check the Troubleshooting section of the ReadMe for more information and possible solutions.</span>";
				}
			}
			//Finally, print the mod's messages.
			if (Modlist[i].messages.size()>0) {
				bosslog << endl << "<ul>" << endl;
				for (size_t j=0; j<Modlist[i].messages.size(); j++) {
					//Print messages.
					ShowMessage(Modlist[i].messages[j],bosslog);
				}
				bosslog << "</ul>" << endl;
			} else 
				bosslog << endl << "<br /><br />" << endl;
		}
	}
	bosslog <<"</p>"<<endl<<"</div><br /><br />"<<endl;
	LOG_INFO("User file ordering applied successfully.");
	

	//Find and show found mods not recognised. These are the mods that are found at and after index x in the mods vector.
	//Order their dates to be i days after the master esm to ensure they load last.
	bosslog << "<div><span>Unrecogised Mod Files</span><p>Reorder these by hand using your favourite mod ordering utility.</p>"<<endl<<"<p>";
	LOG_INFO("Reporting unrecognized mods...");
	for (size_t i=x; i<Modlist.size(); i++) {
		//Only act on mods that exist.
		if (Modlist[i].type == MOD && (fs::exists(data_path / Modlist[i].name) || fs::exists(data_path / fs::path(Modlist[i].name.string()+".ghost")))) {
			if (Modlist[i].name.string().find(".ghost") != string::npos) 
				bosslog << "Unknown mod file: " << Modlist[i].name.string().substr(0,Modlist[i].name.string().length()-6) << " <span class='ghosted'> - Ghosted</span>";
			else 
				bosslog << "Unknown mod file: " << Modlist[i].name.string();
			modfiletime=esmtime;
			modfiletime += i*60; //time_t is an integer number of seconds, so adding 60 on increases it by a minute.
			modfiletime += i*86400; //time_t is an integer number of seconds, so adding 86,400 on increases it by a day.
			//Re-date file. Provide exception handling in case their permissions are wrong.
			LOG_DEBUG(" -- Setting last modified time for file: \"%s\"", Modlist[i].name.string().c_str());
			try { 
				fs::last_write_time(data_path / Modlist[i].name,modfiletime);
			} catch(fs::filesystem_error e) {
				bosslog << " - <span class='error'>Error: Could not change the date of \"" << Modlist[i].name << "\", check the Troubleshooting section of the ReadMe for more information and possible solutions.</span>";
			}
			bosslog << endl << "<br />" << endl;
		}
	}
	bosslog <<"</p>"<<endl<<"</div><br /><br />"<<endl;
	LOG_INFO("Unrecognized mods reported.");
	
	//Let people know the program has stopped.
	bosslog <<"<div><span>Done.</span></div><br /><br />"<<endl<<"</body>"<<endl<<"</html>";
	bosslog.close();
	LOG_INFO("Launching boss log in browser.");
	if ( !silent ) 
		Launch(bosslog_path.string());	//Displays the BOSSlog.txt.
	LOG_INFO("BOSS finished.");
	return (0);
}