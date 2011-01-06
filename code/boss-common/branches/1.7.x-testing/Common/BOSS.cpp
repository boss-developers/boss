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

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <cstring>
#include <string>
#include <ctype.h>
#include <time.h>

#define MAXLENGTH 4096			//maximum length of a file name or comment. Big arbitrary number.


using namespace boss;
using namespace std;
namespace po = boost::program_options;


const wstring g_version     = L"1.7";
const wstring g_releaseDate = L"December 1, 2010";


void ShowVersion() {
	cout << "BOSS: Better Oblivion Sorting Software" << endl;
	wcout << "Version " << g_version << " (" << g_releaseDate << ")" << endl;
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

int main(int argc, char *argv[]) {

	int x;							//random useful integers
	wstring textbuf;                 //a line of text from a file (should usually end up being be a file name);
	time_t esmtime = 0, modfiletime;	//File modification times.
	bool found;
	bool isghost;					//Is the file ghosted or not?
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
		LOG_ERROR(L"cannot specify options multiple times; please use the '--help' option to see usage instructions");
		Fail();
	}catch (exception & e){
		LOG_ERROR(L"%s; please use the '--help' option to see usage instructions", e.what());
		Fail();
	}
	
	// set whether to track log statement origins
	g_logger.setOriginTracking(debug);

	LOG_INFO(L"BOSS starting...");

	if (vm.count("verbose")) {
		if (0 > verbosity) {
			LOG_ERROR(L"invalid option for 'verbose' parameter: %d", verbosity);
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
			LOG_ERROR(L"invalid option for 'revert' parameter: %d", revert);
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
			LOG_ERROR(L"invalid option for 'game' parameter: '%s'", gameStr.c_str());
			Fail();
		}
	
		LOG_DEBUG(L"game autodectection overridden with: '%s' (%d)", gameStr.c_str(), game);
	}

	const string bosslogFilename = bosslog_path.string();
	LOG_DEBUG(L"opening '%s'", bosslogFilename.c_str());
	bosslog.open(bosslogFilename.c_str());
	if (bosslog.fail()) {							
		LOG_ERROR(L"file '%s' could not be accessed for writing; check the"
				  L" Troubleshooting section of the ReadMe for more"
				  L" information and possible solutions", bosslogFilename.c_str());
		Fail();
	}

	//Output HTML start and <head>
	bosslog << "<!DOCTYPE html>"<<endl<<"<html>"<<endl<<"<head>"<<endl<<"<meta http-equiv='Content-Type' content='text/html; charset=UTF-8'>"<<endl
			<< "<title>BOSS Log</title>"<<endl<<"<style type='text/css'>"<<endl
			<< "body {font-family:Calibri,Arial,Verdana,sans-serifs;}"<<endl
			<< "#title {font-size:2.4em; font-weight:bold; text-align: center;}"<<endl
			<< "div > span:first-child {font-weight:bold; font-size:1.3em;}"<<endl
			<< "ul {margin-top:0px; list-style:none; margin-bottom:1.1em;}"<<endl
			<< "ul li {margin-left:-1em; margin-bottom:0.4em;}"<<endl
			<< ".error {color:red;}"<<endl
			<< ".success {color:green}"<<endl
			<< ".warn {color:#FF6600;}"<<endl
			<< ".version {color:teal;}"<<endl
			<< ".ghosted {font-style:italic; color:grey;}"<<endl
			<< ".tags {color:maroon;}"<<endl
			<< "</style>"<<endl<<"</head>"<<endl
			//Output start of <body>
			<< "<body>"<<endl<<"<div id='title'>Better Oblivion Sorting Software Log</div><br />"<<endl
			<< "<div style='text-align:center;'>&copy; Random007 &amp; the BOSS development team, 2009-2010. Some rights reserved.<br />"<<endl
			<< "<a href='http://creativecommons.org/licenses/by-nc-nd/3.0/'>CC Attribution-Noncommercial-No Derivative Works 3.0</a><br />"<<endl
			<< "v"<<g_version<<" ("<<g_releaseDate<<")"<<endl<<"</div><br /><br />";

	if (0 == game) {
		LOG_DEBUG(L"Detecting game...");
		if (fs::exists(data_path / "Oblivion.esm")) game = 1;
		else if (fs::exists(data_path / "Fallout3.esm")) game = 2;
		else if (fs::exists(data_path / "Nehrim.esm")) game = 3;
		else if (fs::exists(data_path / "FalloutNV.esm")) game = 4;
		else {
			LOG_ERROR(L"None of the supported games were detected...");
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

	LOG_INFO(L"Game detected: %d", game);

	if (update || updateonly) {
		bosslog << "<div><span>Masterlist Update</span>"<<endl<<"<p>";
		cout << endl << "Updating to the latest masterlist from the Google Code repository..." << endl;
		LOG_DEBUG(L"Updating masterlist...");
		UpdateMasterlist(game);
		LOG_DEBUG(L"Masterlist updated successfully.");
		bosslog <<"</p>"<<endl<<"</div><br /><br />"<<endl;
	}

	//Masterlist UTF-8 validator.
	order.open(masterlist_path.c_str());
	if (!order.fail()) {
		while (!order.eof()) {
			textbuf=ReadLine("order");
			wstring::iterator end_it = utf8::find_invalid(textbuf.begin(), textbuf.end());
			//Debug: print out the malformed line.
			if (end_it != textbuf.end()) bosslog << "<span class='error'>Error: Masterlist line \"" << textbuf << "\" is not valid UTF-8. Report the line in question to an official BOSS thread." << "<br />" <<endl;
		}
	}
	order.close();
	
	if (updateonly == true) {
		return (0);
	}

	if (game==1 && fs::exists(data_path / "Nehrim.esm")) {
		bosslog << endl << "<p class='error'>Critical Error: Oblivion.esm and Nehrim.esm have both been found!<br />" << endl
						<< "Please ensure that you have installed Nehrim correctly. In a correct install of Nehrim, there is no Oblivion.esm.<br />" << endl
						<< "Utility will end now.</p>" << endl
						<< "</body>"<<endl<<"</html>";
		bosslog.close();
		LOG_ERROR(L"Installation error found: check BOSSLOG.");
		if ( !silent ) 
			Launch(bosslog_path.string());	//Displays the BOSSlog.txt.
		exit (1); //fail in screaming heap.
	}

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
		LOG_ERROR(L"Failed to set modification time of game master file, error was: %s", e.what());
		if ( !silent ) 
			Launch(bosslog_path.string());	//Displays the BOSSlog.txt.
		exit (1); //fail in screaming heap.
	}

	cout << endl << "Better Oblivion Sorting Software working..." << endl;

	Mods modlist;
	modlist.AddMods();
	if (revert<1) {
		int i=modlist.SaveModList();
		if (i!=0) {
			if (i==1) bosslog << endl << "<p class='error'>Critical Error: modlist.old could not be written to!<br />" << endl;
			else bosslog << endl << "<p class='error'>Critical Error: modlist.txt could not be written to!<br />" << endl;
			bosslog << "Check the Troubleshooting section of the ReadMe for more information and possible solutions.<br />" << endl
					<< "Utility will end now.</p>" << endl
					<< "</body>"<<endl<<"</html>";
			bosslog.close();
			if ( !silent ) 
				Launch(bosslog_path.string());	//Displays the BOSSlog.txt.
			exit (1); //fail in screaming heap.
		}
	}
	Rules userlist;
	if (fs::exists(userlist_path) && revert<1) userlist.AddRules();

	if (revert<1) {
		LOG_DEBUG(L"Checking for special mods...");
		bosslog << "<div><span>Special Mod Detection</span>"<<endl<<"<p>";
		if (game == 1) {
			//Check if FCOM or not
			if ((fcom=PluginExists(data_path / "FCOM_Convergence.esm"))) bosslog << "FCOM detected.<br />" << endl;
				else bosslog << "FCOM not detected.<br />" << endl;
			if (PluginExists(data_path / "FCOM_Convergence.esp") && !fcom) bosslog << "WARNING: FCOM_Convergence.esm seems to be missing.<br />" << endl;
			//Check if OOO or not
			if ((ooo=PluginExists(data_path / "Oscuro's_Oblivion_Overhaul.esm"))) bosslog << "OOO detected.<br />" << endl;
				else bosslog << "OOO not detected.<br />" << endl;
			//Check if Better Cities or not
			if ((bc=PluginExists(data_path / "Better Cities Resources.esm"))) bosslog << "Better Cities detected.<br />" << endl;
				else bosslog << "Better Cities not detected.<br />" << endl;
				
			LOG_INFO(L"Special mods found: %s %s %s", fcom ? "FCOM" : "", ooo ? "OOO" : "", bc ? "BC" : "");
		} else if (game == 2) {
			//Check if fook2 or not
			if ((fcom=PluginExists(data_path / "FOOK2 - Main.esm"))) bosslog << "FOOK2 Detected.<br />" << endl;
				else bosslog << "FOOK2 not detected.<br />" << endl;
			if (PluginExists(data_path / "FOOK2 - Main.esp") && !fcom) bosslog << "WARNING: FOOK2.esm seems to be missing.<br />" << endl;
			//Check if fwe or not
			if ((ooo=PluginExists(data_path / "FO3 Wanderers Edition - Main File.esm"))) bosslog << "FWE detected.<br />" << endl;
				else bosslog << "FWE not detected.<br />" << endl;
			
			LOG_INFO(L"Special mods found: %s %s", fcom ? "FOOK2" : "", ooo ? "FWE" : "");
		}
		bosslog <<"</p>"<<endl<<"</div><br /><br />"<<endl;
	}

	//open masterlist.txt
	fs::path sortfile;
	if (revert==1) sortfile = curr_modlist_path;	
	else if (revert==2) sortfile = prev_modlist_path;
	else sortfile = masterlist_path;

	LOG_INFO(L"Using sorting file: %s", sortfile.filename().c_str());
	order.open(sortfile.c_str());

	if (order.fail()) {							
		bosslog << endl << "<p class='error'>Critical Error: ";

		bosslog << sortfile.wstring();

		bosslog << " cannot be read!<br />" << endl
				<< "Check the Troubleshooting section of the ReadMe for more information and possible solutions.<br />" << endl
				<< "Utility will end now.</p>" << endl
				<< "</body>"<<endl<<"</html>";
		bosslog.close();
		LOG_ERROR(L"Couldn't open sorting file: %s", sortfile.filename().c_str());
		if ( !silent ) 
			Launch(bosslog_path.string());	//Displays the BOSSlog.txt.
		exit (1); //fail in screaming heap.
	} //if

	x=0;
	found=false;
	LOG_INFO(L"Starting main sort process...");
	while (!order.eof()) {
		textbuf=ReadLine("order");
		wstring::iterator end_it = utf8::find_invalid(textbuf.begin(), textbuf.end());
		wstring textbuf2 = utf8ToUTF16(textbuf);
		LOG_TRACE(L">> Text line read from sort file: \"%s\"", textbuf.c_str());
		if (textbuf.length()>1 && textbuf[0]!='\\') {		//Filter out blank lines, oblivion.esm and remark lines starting with \.
			if (!IsMessage(textbuf)) {						//Deal with mod lines only here. Message lines will be dealt with below.
				isghost = false;
				if (fs::exists(data_path / fs::path(textbuf2+L".ghost"))) isghost = true;
				if (fs::exists(data_path / textbuf2) || isghost) {					//Tidy function not needed as file system removes trailing spaces and isn't case sensitive

					LOG_DEBUG(L"-- Sorting %smod: \"%s\" into position: %d", isghost ? "ghosted " : "", textbuf.c_str(), x);
					const wstring& filename = isghost ? textbuf+L".ghost" : textbuf;

					int i = modlist.GetModIndex(filename);
					if (i < x || i >= int(modlist.mods.size()))	//The first check is to prevent mods being sorted twice and screwing everything up.
						continue;
					
					found=true;
					LOG_DEBUG(L"  >> Mod found at index: %d in load order.", i);

					// Save current mod's messages for later
					vector<wstring> messages = modlist.modmessages[i];

					// Erase data from current mod's position
					modlist.mods.erase(modlist.mods.begin()+i);
					modlist.modmessages.erase(modlist.modmessages.begin()+i);

					// Use x only if it doesn't overruns the vector size
					int newpos = std::min(x, int(modlist.mods.size()));

					// Adds the mod and its messages to the new position					
					modlist.mods.insert(modlist.mods.begin() + newpos, filename);
					modlist.modmessages.insert(modlist.modmessages.begin() + newpos, messages);

					x++;
				} //if
				else found=false;
			} //if
			else if (found) modlist.modmessages[x-1].push_back(textbuf);
		} //if
	} //while
	order.close();		//Close the masterlist stream, as it's not needed any more.
	LOG_INFO(L"Main sort process finished.");

	if (fs::exists(userlist_path) && revert<1) {
		bosslog << "<div><span>Userlist Messages</span>"<<endl<<"<p>";
		//Go through each rule.
		LOG_INFO(L"Starting userlist sort process... Total %" PRIuS L" user rules statements to process.", userlist.rules.size());
		for (int i=0;i<(int)userlist.rules.size();i++) {
			int start = userlist.rules[i];
			int end;
			if (i==(int)userlist.rules.size()-1) end = (int)userlist.keys.size();
			else end = userlist.rules[i+1];
			//Go through each line of the rule. The first line is given by keys[start] and objects[start].
			LOG_DEBUG(L" -- Processing rule #%d starting at: %d and ending at %d.", i, start, end);
			for (int j=start;j<end;j++) {
				//A mod sorting line.
				LOG_TRACE(L"  -- Processing line: %d.", j);
				if ((userlist.keys[j]==L"before" || userlist.keys[j]==L"after") && IsPlugin(userlist.objects[j])) {
					vector<wstring> currentmessages;
					int index,index1;
					//Get current mod messages and remove mod from current modlist position.
					index1 = modlist.GetModIndex(userlist.objects[start]);
					// Only increment 'x' if we've taken the 'source' mod from below the 'last-sorted' mark
					if (userlist.keys[start]==L"add" && index1 >= x) 
						x++;
					//If it adds a mod already sorted, skip the rule.
					else if (userlist.keys[start]==L"add"  && index1 < x) {
						userlist.messages += L"<span class='warn'>\""+userlist.objects[start]+L"\" is already in the masterlist. Rule skipped.</span><br /><br />";
						LOG_WARN(L" * \"%s\" is already in the masterlist.", userlist.objects[start].c_str());
						break;
					} else if (userlist.keys[start]==L"override" && index1 >= x) {
						userlist.messages += L"<span class='error'>\""+userlist.objects[start]+L"\" is not in the masterlist, cannot override. Rule skipped.</span><br /><br />";
						LOG_WARN(L" * \"%s\" is not in the masterlist, cannot override.", userlist.objects[start].c_str());
						break;
					}

					wstring filename = modlist.mods[index1];
					currentmessages.assign(modlist.modmessages[index1].begin(),modlist.modmessages[index1].end());
					modlist.mods.erase(modlist.mods.begin()+index1);
					modlist.modmessages.erase(modlist.modmessages.begin()+index1);
					//Need to insert mod and mod's messages to a specific position.
					index = modlist.GetModIndex(userlist.objects[j]);

					//Let's take this up to 11 - super awesome stuff beginning.
					//The sort mod isn't installed - so scour the [s]Shire[/s] masterlist for it.
					if (index < 0 || index >= int(modlist.mods.size())) {
						LOG_TRACE(L" --> Referenced mod: \"%s\" is not installed, searching for it.", userlist.objects[j].c_str());
						bool lookforinstalledmod=false;
						order.open(masterlist_path.c_str());
						while (!order.eof()) {
							textbuf=ReadLine("order");
							if (textbuf.length()>1 && textbuf[0]!='\\') {		//Filter out blank lines, oblivion.esm and remark lines starting with \.
								if (!IsMessage(textbuf)) {		//Awww, it's a mod.
									if (Tidy(userlist.objects[j])==Tidy(textbuf)) {
										//The mod in this masterlist line matches the sort mod. Start looking for the next mod that is installed.
										lookforinstalledmod=true;
									} else if (lookforinstalledmod) {
										//Look to see if the mod is installed.
										int i = modlist.GetModIndex(textbuf);
										if (i < 0 || i >= int(modlist.mods.size()))
											continue;
										index = i;
										lookforinstalledmod=false;
										if (userlist.keys[j]==L"after") index -= 1;
										break;
									}
								}
							}
						}
						order.close();
						if (index < 0 || index >= int(modlist.mods.size())) {
							userlist.messages += L"<span class='warn'>\""+userlist.objects[j]+L"\" is not installed, and is not in the masterlist. Rule skipped.</span><br /><br />";
							modlist.mods.insert(modlist.mods.begin()+index1,filename);
							modlist.modmessages.insert(modlist.modmessages.begin()+index1,currentmessages);
							LOG_WARN(L" * \"%s\" is not installed or in the masterlist.", userlist.objects[j].c_str());
							break;
						}
					}
					//Uh oh, the awesomesauce ran out...
					if (index >= x-1) {
						if (userlist.keys[start]==L"add")
							x--;
						userlist.messages += L"<span class='error'>\""+userlist.objects[j]+L"\" is not in the masterlist and has not been sorted by a rule. Rule skipped.</span><br /><br />";
						modlist.mods.insert(modlist.mods.begin()+index1,filename);
						modlist.modmessages.insert(modlist.modmessages.begin()+index1,currentmessages);
						LOG_WARN(L" * \"%s\" is not in the masterlist and has not been sorted by a rule.", userlist.objects[j].c_str());
						break;
					}

					if (userlist.keys[j]==L"after") index += 1;
					modlist.mods.insert(modlist.mods.begin()+index,filename);
					modlist.modmessages.insert(modlist.modmessages.begin()+index,currentmessages);
					userlist.messages += L"<span class='success'>\""+userlist.objects[start]+L"\" has been sorted "+Tidy(userlist.keys[j]) + L" \"" + userlist.objects[j] + L"\".</span><br /><br />";

				//A group sorting line.
				} else if ((userlist.keys[j]==L"before" || userlist.keys[j]==L"after") && !IsPlugin(userlist.objects[j])) {
					//Search masterlist for rule group. Once found, search it for mods in modlist, recording the mods that match.
					//Then search masterlist for sort group. Again, search and record matching modlist mods.
					//If sort keyword is before, discard all but the first recorded sort group mod, and if it is after, discard all but the last recorded sort group mod.
					//Then insert the recorded rule group mods before or after the remaining sort group mod and erase them from their old positions.
					//Remember to move their messages too.

					order.open(masterlist_path.c_str());
					int count=0;
					bool lookforrulemods=false,lookforsortmods=false;
					vector<wstring> rulemods,sortmods,currentmessages;
					while (!order.eof()) {					
						textbuf=ReadLine("order");
						if (textbuf.length()>1 && (textbuf.substr(1,10)==L"BeginGroup" || textbuf.substr(1,8)==L"EndGroup")) {
							//A group starts or ends. Check rule to see if it matches any.
							if (textbuf.substr(1,10)==L"BeginGroup") {
								if (Tidy(userlist.objects[start])==Tidy(textbuf.substr(14))) {
									//Rule match. Now search for a line that matches something in modlist.
									lookforrulemods=true;
									lookforsortmods=false;
									count = 0;
								} else if (Tidy(userlist.objects[j])==Tidy(textbuf.substr(14))) {
									//Sort match. Now search for lines that match something in modlist.
									lookforsortmods=true;
									lookforrulemods=false;
									count = 0;
								}
								count += 1;
							} else if (count>0 && textbuf.substr(1,8)==L"EndGroup") {
								count -= 1;
								if (count==0) {
									//The end of the matched group has been found. Stop searching for mods to move.
									lookforrulemods=false;
									lookforsortmods=false;
								}
							}
						} else if ((lookforrulemods || lookforsortmods)  && textbuf[0]!='\\') {
							if (!IsMessage(textbuf)) {
								if (PluginExists(data_path / textbuf)) {
									//Found a mod.
									int gm = modlist.GetModIndex(textbuf);
									if (lookforrulemods) {
										rulemods.push_back(modlist.mods[gm]);
									} else if (lookforsortmods) {
										sortmods.push_back(modlist.mods[gm]);
									}
								}
							}
						}
					}
					order.close();
					if (rulemods.empty()) {
						userlist.messages += L"<span class='error'>The group \""+userlist.objects[start]+L"\" does not contain any installed mods, or is not in the masterlist. Rule skipped.</span><br /><br />";
						LOG_WARN(L" * \"%s\" does not contain any mods, or is not in the masterlist.", userlist.objects[start].c_str());
						break;
					} else if (sortmods.empty()) {
						userlist.messages += L"<span class='error'>The group \""+userlist.objects[j]+L"\" does not contain any installed mods, or is not in the masterlist. Rule skipped.</span><br /><br />";
						LOG_WARN(L" * \"%s\" does not contain any mods, or is not in the masterlist.", userlist.objects[j].c_str());
						break;
					}
					if (userlist.keys[j]==L"before") {
						for (int k=0;k<(int)rulemods.size();k++) {
							int index1 = modlist.GetModIndex(rulemods[k]);
							currentmessages.assign(modlist.modmessages[index1].begin(),modlist.modmessages[index1].end());
							modlist.mods.erase(modlist.mods.begin()+index1);
							modlist.modmessages.erase(modlist.modmessages.begin()+index1);
							//Position of mod to load rulemods before. Insert adds in front of this, so don't change the value.
							//As you move mods around, this will change, so look for it again in each loop.
							int index = modlist.GetModIndex(sortmods.front());
							modlist.mods.insert(modlist.mods.begin()+index,rulemods[k]);
							modlist.modmessages.insert(modlist.modmessages.begin()+index,currentmessages);
						}
					} else if (userlist.keys[j]==L"after") {	
						//Iterate backwards to make sure they're added in the right order.
						for (int k=(int)rulemods.size()-1;k>-1;k--) {
							int index1 = modlist.GetModIndex(rulemods[k]);
							currentmessages.assign(modlist.modmessages[index1].begin(),modlist.modmessages[index1].end());
							modlist.mods.erase(modlist.mods.begin()+index1);
							modlist.modmessages.erase(modlist.modmessages.begin()+index1);
							//Position of mod to load rulemods before. Insert adds in front of this, so add one to make sure they're added after.
							//As you move mods around, this will change, so look for it again in each loop.
							int index = modlist.GetModIndex(sortmods.back())+1;
							modlist.mods.insert(modlist.mods.begin()+index,rulemods[k]);
							modlist.modmessages.insert(modlist.modmessages.begin()+index,currentmessages);
						}
					}
					userlist.messages += L"<span class='success'>The group \""+userlist.objects[start]+L"\" has been sorted "+Tidy(userlist.keys[j]) + L" the group \"" + userlist.objects[j] + L"\".</span><br /><br />";
				//An insertion line.
				} else if (userlist.keys[j]==L"top" || userlist.keys[j]==L"bottom") {
					vector<wstring> currentmessages;
					//Get current mod messages and remove mod from current modlist position.
					int index1 = modlist.GetModIndex(userlist.objects[start]);
					// Only increment 'x' if we've taken the 'source' mod from below the 'last-sorted' mark
					if (userlist.keys[start]==L"add" && index1 >= x) 
						x++;
					//If it adds a mod already sorted, skip the rule.
					else if (userlist.keys[start]==L"add"  && index1 < x) {
						userlist.messages += L"<span class='warn'>\""+userlist.objects[start]+L"\" is already in the masterlist. Rule skipped.</span><br /><br />";
						break;
					} else if (userlist.keys[start]==L"override" && index1 >= x) {
						userlist.messages += L"<span class='error'>\""+userlist.objects[start]+L"\" is not in the masterlist, cannot override. Rule skipped.</span><br /><br />";
						LOG_WARN(L" * \"%s\" is not in the masterlist, cannot override.", userlist.objects[start].c_str());
						break;
					}
					wstring filename = modlist.mods[index1];
					currentmessages.assign(modlist.modmessages[index1].begin(),modlist.modmessages[index1].end());
					modlist.mods.erase(modlist.mods.begin()+index1);
					modlist.modmessages.erase(modlist.modmessages.begin()+index1);
					//Need to insert mod and mod's messages to a specific position.
					//This is the tricky bit.
					order.open(masterlist_path.c_str());
					int count=0;
					bool lookforsortmods=false,overtime=false;
					vector<wstring> sortmods;
					LOG_TRACE(L" --> Looking in masterlist for the referenced group: \"%s\".", userlist.objects[j].c_str());
					while (!order.eof()) {
						textbuf=ReadLine("order");
						if (textbuf.length()>1 && (textbuf.substr(1,10)==L"BeginGroup" || textbuf.substr(1,8)==L"EndGroup")) {
							//A group starts or ends. Check rule to see if it matches.
							if (textbuf.substr(1,10)==L"BeginGroup") {
								if (Tidy(userlist.objects[j])==Tidy(textbuf.substr(14))) {
									//Sort group match. Now search for lines that match something in modlist.
									lookforsortmods=true;
									count = 0;
								}
								count += 1;
							} else if (count>0 && textbuf.substr(1,8)==L"EndGroup") {
								count -= 1;
								if (count==0) {
									if ((int)sortmods.size()>0) {
										//The end of the matched group has been found, and we have found at least one mod in that group. Stop searching for installed mods.
										lookforsortmods=false;
										break;
									} else {
										//The end of the matched group was found, but we still don't have any mods to sort relative to.
										//Keep searching for mods, but now top and bottom both mean "before the mod found next".
										overtime=true;
									}
								}
							}
						} else if (lookforsortmods && textbuf[0]!='\\') {
							if (!IsMessage(textbuf)) {
								if (PluginExists(data_path / textbuf)) {
									//Found a mod.
									int gm = modlist.GetModIndex(textbuf);
									sortmods.push_back(modlist.mods[gm]);
									if (overtime) break;	//Stop looking for more mods immediately.
								}
							}
						}
					}
					order.close();
					if (sortmods.empty()) {
						userlist.messages += L"<span class='error'>The group \""+userlist.objects[j]+L"\" does not contain any installed mods, or is not in the masterlist. Rule skipped.</span><br /><br />";
						LOG_WARN(L" * \"%s\" does not contain any mods, or is not in the masterlist.", userlist.objects[j].c_str());
						break;
					}
					int index = 0;
					if ((int)sortmods.size()>0) {
						if (userlist.keys[j]==L"top") 
							index = modlist.GetModIndex(sortmods.front());
						else if (userlist.keys[j]==L"bottom") {
							index = modlist.GetModIndex(sortmods.back());
							if (!overtime) index += 1;
						}
					} else index = x-1;
					modlist.mods.insert(modlist.mods.begin()+index,filename);
					modlist.modmessages.insert(modlist.modmessages.begin()+index,currentmessages);
					if (userlist.keys[j]==L"top") 
						userlist.messages += L"<span class='success'>\""+userlist.objects[start]+L"\" inserted into the top of group \"" + userlist.objects[j] + L"\".</span><br /><br />";
					else if (userlist.keys[j]==L"bottom") 
						userlist.messages += L"<span class='success'>\""+userlist.objects[start]+L"\" inserted into the bottom of group \"" + userlist.objects[j] + L"\".</span><br /><br />";
			
				//A message line.
				} else if (userlist.keys[j]==L"append" || userlist.keys[j]==L"replace") {
					//Look for the modlist line that contains the match mod of the rule.
					int index = modlist.GetModIndex(userlist.objects[start]);
					userlist.messages += L"<span class='success'>\"" + userlist.objects[j] + L"\"";
					if (userlist.keys[j]==L"append") {			//Attach the rule message to the mod's messages list.
						userlist.messages += L" appended to ";
					} else if (userlist.keys[j]==L"replace") {	//Clear the message list and then attach the message.
						modlist.modmessages[index].clear();
						userlist.messages += L" replaced ";
					}
					modlist.modmessages[index].push_back(userlist.objects[j]);
					userlist.messages += L"messages attached to \"" + userlist.objects[start] + L"\".</span><br /><br />";
				}
			}
		}
		userlist.PrintMessages(bosslog);
		if ((int)userlist.rules.size()==0) bosslog << L"No valid rules were found in your userlist.txt.<br />" << endl;
		bosslog <<L"</p>"<<endl<<L"</div><br /><br />"<<endl;
		LOG_INFO(L"Userlist sorting process finished.");
	}

	//Re-order .esp/.esm files to masterlist.txt order and output messages
	if (revert<1) bosslog << L"<div><span>Recognised And Re-ordered Mod Files</span>"<<endl<<L"<p>"<<endl;
	else if (revert==1) bosslog << L"<div><span>Restored Load Order (Using modlist.txt)</span>"<<endl<<L"<p>"<<endl;
	else if (revert==2) bosslog << L"<div><span>Restored Load Order (Using modlist.old)</span>"<<endl<<L"<p>"<<endl;

	x = min(x, int(modlist.mods.size()));

	LOG_INFO(L"Applying calculated ordering to user files...");
	bosslog << "<div></div>" << endl; //This fixes the Oblivion.esm comment (or first block element, really) being displayed irrespective of CSS.
	for (int i=0;i<x;i++) {
		bool ghosted = false;
		wstring filename,version;
		if (Tidy(modlist.mods[i].substr(modlist.mods[i].length()-6))==L".ghost") {
			ghosted=true;
			filename = modlist.mods[i].substr(0,modlist.mods[i].length()-6);
		} else 
			filename = modlist.mods[i];
		wstring text = L"<b>"+filename;
		if (!skip_version_parse) {
			version = GetModHeader(filename, ghosted);
			if (!version.empty())
				text += L" <span class='version'>[Version "+version+L"]</span>";
		}
		text += L"</b>";
		if (ghosted) 
			text += L" <span class='ghosted'> - Ghosted</span>";
		bosslog << text;		// show which mod file is being processed.
		modfiletime=esmtime;
		modfiletime += i*60; //time_t is an integer number of seconds, so adding 60 on increases it by a minute.
		if (IsValidLine(modlist.mods[i])) {
			//Re-date file. Provide exception handling in case their permissions are wrong.
			LOG_DEBUG(L" -- Setting last modified time for file: \"%s\"", modlist.mods[i].c_str());
			wstring utf16filename = utf8ToUTF16(modlist.mods[i]);
			try { fs::last_write_time(data_path / utf16filename,modfiletime);
			} catch(fs::filesystem_error e) {
				bosslog << L" - <span class='error'>Error: Could not change the date of \"" << modlist.mods[i] << L"\", check the Troubleshooting section of the ReadMe for more information and possible solutions.</span>";
			}
		}
		if (modlist.modmessages[i].size()>0) {
			bosslog << endl << L"<ul>" << endl;
			for (int j=0;j<(int)modlist.modmessages[i].size();j++) {
				ShowMessage(modlist.modmessages[i][j], game);		//Deal with message lines here.
			}
			bosslog << L"</ul>" << endl;
		} else 
			bosslog << endl << L"<br /><br />" << endl;
	}
	LOG_INFO(L"User file ordering applied successfully.");
	
	bosslog <<L"</p>"<<endl<<L"</div><br /><br />"<<endl;

	//Find and show found mods not recognised. These are the mods that are found at and after index x in the mods vector.
	//Order their dates to be +1 month after the master esm to ensure they load last.
	bosslog << L"<div><span>Unrecogised Mod Files</span><p>Reorder these by hand using your favourite mod ordering utility.</p>"<<endl<<L"<p>";
	LOG_INFO(L"Reporting unrecognized mods...");
	for (int i=x;i<(int)modlist.mods.size();i++) {
		if (modlist.mods[i].length()>1) {
			if (modlist.mods[i].find(L".ghost") != string::npos) bosslog << L"Unknown mod file: " << modlist.mods[i].substr(0,modlist.mods[i].length()-6) << L" <span class='ghosted'> - Ghosted</span>";
			else bosslog << L"Unknown mod file: " << modlist.mods[i];
			modfiletime=esmtime;
			modfiletime += i*60; //time_t is an integer number of seconds, so adding 60 on increases it by a minute.
			modfiletime += i*86400; //time_t is an integer number of seconds, so adding 86,400 on increases it by a day.
			//Re-date file. Provide exception handling in case their permissions are wrong.
			LOG_DEBUG(L" -- Setting last modified time for file: \"%s\"", modlist.mods[i].c_str());
			wstring utf16filename = utf8ToUTF16(modlist.mods[i]);
			try { fs::last_write_time(data_path / utf16filename,modfiletime);
			} catch(fs::filesystem_error e) {
				bosslog << L" - <span class='error'>Error: Could not change the date of \"" << modlist.mods[i] << L"\", check the Troubleshooting section of the ReadMe for more information and possible solutions.</span>";
			}
			bosslog << endl << L"<br />" << endl;
		}
	} //while
	bosslog <<L"</p>"<<endl<<L"</div><br /><br />"<<endl;
	LOG_INFO(L"Unrecognized mods reported.");

	//Let people know the program has stopped.
	bosslog <<L"<div><span>Done.</span></div><br /><br />"<<endl<<L"</body>"<<endl<<L"</html>";
	bosslog.close();
	LOG_INFO(L"Launching boss log in browser.");
	if ( !silent ) 
		Launch(bosslog_path.string());	//Displays the BOSSlog.txt.
	LOG_INFO(L"BOSS finished.");
	return (0);
}