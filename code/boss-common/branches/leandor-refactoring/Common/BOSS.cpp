/*	Better Oblivion Sorting Software
	1.6
	Quick and Dirty Load Order Utility for Oblivion, Fallout 3 and Morrowind
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/
*/

#include "BOSS.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <ctype.h>
#include <time.h>
#include <boost/algorithm/string.hpp>

#define MAXLENGTH 4096			//maximum length of a file name or comment. Big arbitrary number.

using namespace boss;
using namespace boost;

const string BOSS::LEGAL = "\
Better Oblivion Sorting Software is a utility that sorts the load order of TESIV: Oblivion,"
" TESIII: Morrowind and Fallout 3 mods according to their relative positions on a frequently-updated"
" masterlist to ensure proper load order and minimise incompatibilities between mods.\
\
Optional Parameters:\
\
	-u, --update:\
		* Automatically updates the local copy of the masterlist using the latest version available on the Google Code repository.\
\
	-V-, --disable-version-parse:\
		* Enables the parsing of each mod's description and if found extracts from there the author stamped mod's version and prints it along other data in the generated bosslog.txt.\
\
	-r, --revert-level:\
		* Can accept values of 1 or 2. Sets BOSS to revert its changes back the given number of levels.\
\
";

//BOSS [--update | -u] [--help | -h] [--version-check | -V]  [--revert-level | -r] [1 | 2]
int main(int argc, char *argv[]) {					
	BOSS boss(argc, argv);

	int x;							//random useful integers
	string textbuf,textbuf2;		//a line of text from a file (should usually end up being be a file name);
	time_t esmtime,modfiletime;		//File modification times.
	bool found;
	bool update = false;			//To update masterlist or not?
	bool version_parse = true;		//Enable parsing of mod's headers to look for version strings
	int game;						//What game's mods are we sorting? 1 = Oblivion, 2 = Fallout 3, 3 = Morrowind.
	int revert=0;						//What level to revert to?

	//Parse command line arguments.
	if (argc > 1) {
		for (int i=0; i < argc; i++) {
			if (strcmp("--update", argv[i]) == 0 || strcmp("-u", argv[i]) == 0) {
				update = true;
			} else if (strcmp("--disable-version-parse", argv[i]) == 0 || strcmp("-V-", argv[i]) == 0) {
				version_parse = false;
			} else if (strcmp("--revert-level", argv[i]) == 0 || strcmp("-r", argv[i]) == 0) {
				//If the correct argument is given, use it. If not, assume that they meant to roll back one level.
				if (i+1<argc) {
					if (strcmp("1", argv[i+1]) == 0 || strcmp("2", argv[i+1]) == 0) revert = atoi(argv[i+1]);
					else revert = 1;
				} else revert = 1;
			} else if (strcmp("--help", argv[i]) == 0 || strcmp("-h", argv[i]) == 0) {
				cout << "Better Oblivion Sorting Software is a utility that sorts the load order of TESIV: Oblivion, TESIII: Morrowind and Fallout 3 mods according to their relative positions on a frequently-updated masterlist ";
				cout << "to ensure proper load order and minimise incompatibilities between mods." << endl << endl;
				cout << "Optional Parameters" << endl << endl;
				cout << "-u, --update: " << endl << "    Automatically updates the local copy of the masterlist using the latest version available on the Google Code repository." << endl << endl;
				cout << "-V-, --disable-version-parse: " << endl << "    Enables the parsing of each mod's description and if found extracts from there the author stamped mod's version and prints it along other data in the generated bosslog.txt." << endl << endl;
				cout << "-r, --revert-level : " << endl << "	Can accept values of 1 or 2. Sets BOSS to revert its changes back the given number of levels." << endl << endl;
				exit (0);
			}
		}
	}

	//Try to create BOSS sub-directory.
	try { 
		fs::create_directory("BOSS\\");
	} catch(fs::filesystem_error e) {
		cout << "Critical Error: Sub-directory \"Data\\BOSS\\\" could not be created!" << endl
			 << "Check the Troubleshooting section of the ReadMe for more information and possible solutions." << endl
			 << "Utility will end now." << endl << endl;
		cout << "Press ENTER to quit...";
		cin.ignore(1,'\n');
		exit(1); //fail in screaming heap.
	}

	//Check for creation of BOSSlog.txt.
	bosslog.open("BOSS\\BOSSlog.html");
	if (bosslog.fail()) {							
		cout << endl << "Critical Error: BOSSlog.html could not be written to!" << endl
					 << "Check the Troubleshooting section of the ReadMe for more information and possible solutions." << endl
					 << "Utility will end now." << endl << endl;
		cout << "Press ENTER to quit...";
		cin.ignore(1,'\n');
		exit (1); //fail in screaming heap.
	}

	//Output HTML start and <head>
	bosslog << "<!DOCTYPE html>"<<endl<<"<html>"<<endl<<"<head>"<<endl<<"<meta http-equiv='Content-Type' content='text/html; charset=UTF-8'>"<<endl
			<< "<title>BOSS Log</title>"<<endl<<"<style type='text/css'>"<<endl<<"#body {font-family:Calibri,Arial,Verdana,sans-serifs;}"<<endl
			<< "#title {font-size:2.4em; font-weight:bold; text-align: center;}"<<endl<<"div > span {font-weight:bold; font-size:1.3em;}"<<endl
			<< "ul li {margin-bottom:10px;}"<<endl<<".error {color:red; font-weight:normal; font-size:1em;}"<<endl<<"</style>"<<endl<<"</head>"<<endl
			//Output start of <body>
			<< "<body id='body'>"<<endl<<"<div id='title'>Better Oblivion Sorting Software Log</div><br />"<<endl
			<< "<div style='text-align:center;'>&copy; Random007 &amp; the BOSS development team, 2009-2010. Some rights reserved.<br />"<<endl
			<< "<a href='http://creativecommons.org/licenses/by-nc-nd/3.0/'>CC Attribution-Noncommercial-No Derivative Works 3.0</a><br />"<<endl
			<< "v1.6 (03 Spetember 2010)"<<endl<<"</div><br /><br />";

	if (fs::exists("Oblivion.esm")) game = 1;
	else if (fs::exists("Fallout3.esm")) game = 2;
	else if (fs::exists("Morrowind.esm")) game = 3;
	else {
		bosslog << endl << "<p class='error'>Critical Error: Master .ESM file not found!<br />" << endl
						<< "Check the Troubleshooting section of the ReadMe for more information and possible solutions.<br />" << endl
						<< "Utility will end now.</p>" << endl
						<< "</body>"<<endl<<"</html>";
		bosslog.close();
		system("start BOSS\\BOSSlog.html");	//Displays the BOSSlog.txt.
		exit (1); //fail in screaming heap.
	} //else

	//Get the master esm's modification date. 
	//Not sure if this needs exception handling, since by this point the file definitely exists. Do it anyway.
	try {
		if (game == 1) esmtime = fs::last_write_time("Oblivion.esm");
		else if (game == 2) esmtime = fs::last_write_time("Fallout3.esm");
		else if (game == 3) esmtime = fs::last_write_time("Morrowind.esm");
	} catch(fs::filesystem_error e) {
		bosslog << endl << "<p class='error'>Critical Error: Master .ESM file cannot be read!<br />" << endl
						<< "Check the Troubleshooting section of the ReadMe for more information and possible solutions.<br />" << endl
						<< "Utility will end now.</p>" << endl
						<< "</body>"<<endl<<"</html>";
		bosslog.close();
		system("start BOSS\\BOSSlog.html");	//Displays the BOSSlog.txt.
		exit (1); //fail in screaming heap.
	}

	if (update == true) {
		cout << endl << "Updating to the latest masterlist from the Google Code repository..." << endl;
		int rev = UpdateMasterlist(game);
		if (rev > 0) cout << "masterlist.txt updated to revision " << rev << endl;
		else cout << "Error: Masterlist update failed." << endl << "Check the Troubleshooting section of the ReadMe for more information and possible solutions." << endl;
	}

	cout << endl << "Better Oblivion Sorting Software working..." << endl;

	ModList modlist;

	if (revert<1) {
		int i=modlist.Save();
		if (i!=0) {
			if (i==1) bosslog << endl << "<p class='error'>Critical Error: modlist.old could not be written to!<br />" << endl;
			else bosslog << endl << "<p class='error'>Critical Error: modlist.txt could not be written to!<br />" << endl;
			bosslog << "Check the Troubleshooting section of the ReadMe for more information and possible solutions.<br />" << endl
					<< "Utility will end now.</p>" << endl
					<< "</body>"<<endl<<"</html>";
			bosslog.close();
			system("start BOSS\\BOSSlog.html");	//Displays the BOSSlog.txt.
			exit (1); //fail in screaming heap.
		}
	}

	Rules userlist;
	if (fs::exists("BOSS\\userlist.txt") && revert<1) userlist.AddRules();

	if (revert<1) {
		bosslog << "<div><span>Special Mod Detection</span>"<<endl<<"<p>";
		if (game == 1) {
			//Check if FCOM or not
			if ((fcom=fs::exists("FCOM_Convergence.esm"))) bosslog << "FCOM detected.<br />" << endl;
				else bosslog << "FCOM not detected.<br />" << endl;
			if (fs::exists("FCOM_Convergence.esp") && !fcom) bosslog << "WARNING: FCOM_Convergence.esm seems to be missing.<br />" << endl;
			//Check if OOO or not
			if ((ooo=fs::exists("Oscuro's_Oblivion_Overhaul.esm"))) bosslog << "OOO detected.<br />" << endl;
				else bosslog << "OOO not detected.<br />" << endl;
			//Check if Better Cities or not
			if ((bc=fs::exists("Better Cities Resources.esm"))) bosslog << "Better Cities detected.<br />" << endl;
				else bosslog << "Better Cities not detected.<br />" << endl;
		} else if (game == 2) {
			//Check if fook2 or not
			if ((fook2=fs::exists("FOOK2 - Main.esm"))) bosslog << "FOOK2 Detected.<br />" << endl;
				else bosslog << "FOOK2 not detected.<br />" << endl;
			if (fs::exists("FOOK2 - Main.esp") && !fook2) bosslog << "WARNING: FOOK2.esm seems to be missing.<br />" << endl;
			//Check if fwe or not
			if ((fwe=fs::exists("FO3 Wanderers Edition - Main File.esm"))) bosslog << "FWE detected.<br />" << endl;
				else bosslog << "FWE not detected.<br />" << endl;
		}
		bosslog <<"</p>"<<endl<<"</div><br /><br />"<<endl;
	}

	//open masterlist.txt
	if (revert==1) order.open("BOSS\\modlist.txt");	
	else if (revert==2) order.open("BOSS\\modlist.old");	
	else order.open("BOSS\\masterlist.txt");
	if (order.fail()) {							
		bosslog << endl << "<p class='error'>Critical Error: ";

		if (revert==1) bosslog << "BOSS\\modlist.txt";	
		else if (revert==2) bosslog << "BOSS\\modlist.old";	
		else bosslog << "BOSS\\masterlist.txt";

		bosslog << " cannot be read!<br />" << endl
				<< "Check the Troubleshooting section of the ReadMe for more information and possible solutions.<br />" << endl
				<< "Utility will end now.</p>" << endl
				<< "</body>"<<endl<<"</html>";
		bosslog.close();
		system("start BOSS\\BOSSlog.html");	//Displays the BOSSlog.txt.
		exit (1); //fail in screaming heap.
	} //if

	x=0;
	found=false;
	ModList::Iterator current = modlist.end();
	while (!order.eof()) {					
		textbuf=ReadLine("order");
		if (textbuf.length()>1 && textbuf[0]!='\\') {		//Filter out blank lines, oblivion.esm and remark lines starting with \.
			if (!IsMessage(textbuf)) {						//Deal with mod lines only here. Message lines will be dealt with below.

				current = modlist.Find(textbuf);

				if (modlist.IsValid(current)) {					//Tidy function not needed as file system removes trailing spaces and isn't case sensitive
					found=true;
					
					modlist.SetPosition(current, x);

					if (revert<1) {
						int i = userlist.GetRuleIndex(textbuf, "ADD");
						while (i>-1) {
							userlist.messages += "\""+userlist.objects[i]+"\" is already in the masterlist. Rule skipped.<br /><br />";
							int ruleindex,max;
							for (int j=0;j<(int)userlist.rules.size();j++) {
								if (i==userlist.rules[j]) {
									ruleindex = j;
									break;
								}
							}
							if (ruleindex+1==(int)userlist.rules.size()) max = (int)userlist.keys.size();
							else max = userlist.rules[ruleindex+1];
							for (int j=i;j<max;j++) {
								userlist.keys[j]="";
								userlist.objects[j]="";
							}
							userlist.rules.erase(userlist.rules.begin()+ruleindex);
							i = userlist.GetRuleIndex(textbuf,"ADD");
						}
					}
					x++;
				} //if
				else found=false;
			} //if
			else if (found) {
				modlist.AddMessage(current, textbuf);
			}
		} //if
	} //while
	order.close();		//Close the masterlist stream, as it's not needed any more.

	if (fs::exists("BOSS\\userlist.txt") && revert<1) {
		bosslog << "<div><span>Userlist Messages</span>"<<endl<<"<p>";
		//Go through each rule.
		for (int i=0;i<(int)userlist.rules.size();i++) {
			int start = userlist.rules[i];
			int end;
			if (i==(int)userlist.rules.size()-1) end = (int)userlist.keys.size();
			else end = userlist.rules[i+1];
			//Go through each line of the rule. The first line is given by keys[start] and objects[start].
			for (int j=start;j<end;j++) {
				//A sorting line.
				if ((userlist.keys[j]=="BEFORE" || userlist.keys[j]=="AFTER") && IsPlugin(userlist.objects[j])) {
					if (userlist.keys[start]=="ADD") x++;
					vector<string> currentmessages;
					//Get current mod messages and remove mod from current modlist position.
					ModList::Iterator iter = modlist.Find(userlist.objects[start]);
					if (!modlist.IsValid(iter)) continue;

					//Need to insert mod and mod's messages to a specific position.
					int index = modlist.IndexOf(userlist.objects[j]);
					if (userlist.keys[j]=="AFTER") index += 1;
					modlist.SetPosition(iter, index);

					userlist.messages += "\""+userlist.objects[start]+"\" has been sorted "+Tidy(userlist.keys[j]) + " \"" + userlist.objects[j] + "\".<br /><br />";
				} else if ((userlist.keys[j]=="BEFORE" || userlist.keys[j]=="AFTER") && !IsPlugin(userlist.objects[j])) {
					//Search masterlist for rule group. Once found, search it for mods in modlist, recording the mods that match.
					//Then search masterlist for sort group. Again, search and record matching modlist mods.
					//If sort keyword is BEFORE, discard all but the first recorded sort group mod, and if it is AFTER, discard all but the last recorded sort group mod.
					//Then insert the recorded rule group mods before or after the remaining sort group mod and erase them from their old positions.
					//Remember to move their messages too.

					order.open("BOSS\\masterlist.txt");
					int count=0;
					bool lookforrulemods,lookforsortmods;
					vector<string> rulemods,sortmods,currentmessages;
					while (!order.eof()) {					
						textbuf=ReadLine("order");
						if (textbuf.length()>1 && (textbuf.substr(1,10)=="BeginGroup" || textbuf.substr(1,8)=="EndGroup")) {
							//A group starts or ends. Search rules to see if it matches any.
							if (textbuf.substr(1,10)=="BeginGroup") {
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
							} else if (count>0 && textbuf.substr(1,8)=="EndGroup") {
								count -= 1;
								if (count==0) {
									//The end of the matched group has been found. Stop searching for mods to move.
									lookforrulemods=false;
									lookforsortmods=false;
								}
							}
						} else if ((lookforrulemods || lookforsortmods)  && textbuf[0]!='\\') {
							if (!IsMessage(textbuf)) {

								ModList::Iterator iter = modlist.Find(textbuf);
								if (modlist.IsValid(iter)) {
									//Found a mod.
									string modname = (*iter)->Name();

									if (lookforrulemods) {
										rulemods.push_back(modname);
									} else if (lookforsortmods) {
										sortmods.push_back(modname);
									}
								}
							}
						}
					}
					order.close();
					if (userlist.keys[j]=="BEFORE") {
						for (int k=0;k<(int)rulemods.size();k++) {

							ModList::Iterator mod = modlist.Find(rulemods[k]);
							if (!modlist.IsValid(mod)) continue;

							//Position of mod to load rulemods before. Insert adds in front of this, so don't change the value.
							//As you move mods around, this will change, so look for it again in each loop.
							int index = modlist.IndexOf(sortmods.front());
							modlist.SetPosition(mod, index);
						}
					} else if (userlist.keys[j]=="AFTER") {	
						//Iterate backwards to make sure they're added in the right order.
						for (int k=(int)rulemods.size()-1;k>-1;k--) {

							ModList::Iterator mod = modlist.Find(rulemods[k]);
							if (!modlist.IsValid(mod)) continue;

							//Position of mod to load rulemods before. Insert adds in front of this, so add one to make sure they're added after.
							//As you move mods around, this will change, so look for it again in each loop.
							int index = modlist.IndexOf(sortmods.back()) + 1;
							modlist.SetPosition(mod, index);
						}
					}
					userlist.messages += "The group \""+userlist.objects[start]+"\" has been sorted "+Tidy(userlist.keys[j]) + " the group \"" + userlist.objects[j] + "\".<br /><br />";
					
				} else if (userlist.keys[j]=="APPEND" || userlist.keys[j]=="REPLACE") {
					
					//Look for the modlist line that contains the match mod of the rule.
					ModList::Iterator mod = modlist.Find(userlist.objects[start]);
					if (!modlist.IsValid(mod)) continue;

					userlist.messages += "\"" + userlist.objects[j] + "\"";

					string rule = userlist.keys[j];

					bool replacing = (rule == "REPLACE") ? true : false;
					string action = replacing ? " replaced " : " appended to ";

					userlist.messages += action;
					modlist.AddMessage(mod, userlist.objects[j], replacing);
					userlist.messages += "messages attached to \"" + userlist.objects[start] + "\".<br /><br />";
				}
			}
		}
		userlist.PrintMessages(bosslog);
		if ((int)userlist.rules.size()==0) bosslog << "No valid rules were found in your userlist.txt.<br />" << endl;
		bosslog <<"</p>"<<endl<<"</div><br /><br />"<<endl;
	}

	//Re-order .esp/.esm files to masterlist.txt order and output messages
	if (revert<1) bosslog << "<div><span>Recognised And Re-ordered Mod Files</span>"<<endl<<"<p>"<<endl;
	else if (revert==1) bosslog << "<div><span>Restored Load Order (Using modlist.txt)</span>"<<endl<<"<p>"<<endl;
	else if (revert==2) bosslog << "<div><span>Restored Load Order (Using modlist.old)</span>"<<endl<<"<p>"<<endl;

	modfiletime = esmtime;
	for (ModList::Iterator iter = modlist.begin(); iter != modlist.At(x - 1); iter++) {

		ModList::Reference mod = *iter;

		bool ghosted = mod->IsGhosted();
		string filename = mod->Filename();
		const MessageList& messages = mod->Messages();

		string text = version_parse ? GetModHeader(mod->Filename(), ghosted) : filename;

		if (mod->IsGhosted()) 
				text += " <em> - Ghosted</em>";

		if (messages.NotEmpty()) 
			bosslog << "<b>" << text << "</b>";		// show which mod file is being processed.
		else 
			bosslog << text;

		modfiletime += 60; //time_t is an integer number of seconds, so adding 60 on increases it by a minute.
		if (IsValidLine(filename)) {
			//Re-date file. Provide exception handling in case their permissions are wrong.
			try { 
				fs::last_write_time(filename, modfiletime);
			} catch(fs::filesystem_error e) {
				bosslog << " - <span class='error'>Error: Could not change the date of \"" << mod << "\", check the Troubleshooting section of the ReadMe for more information and possible solutions.</span>";
			}
		}

		if (messages.IsEmpty()) 
			bosslog << endl << "<br /><br />" << endl;

		if (messages.NotEmpty()) {
			
			bosslog << endl << "<ul>" << endl;

			for (MessageList::ConstIterator iter = messages.begin(); iter != messages.end() ; iter++) {
				ShowMessage(*iter, fcom, ooo, bc, fook2, fwe);		//Deal with message lines here.
			}
			
			bosslog << "</ul>" << endl;
		}
	}
	
	bosslog <<"</p>"<<endl<<"</div><br /><br />"<<endl;

	//Find and show found mods not recognised. These are the mods that are found at and after index x in the mods vector.
	//Order their dates to be +1 month after the master esm to ensure they load last.
	bosslog << "<div><span>Unrecogised Mod Files</span><p>Reorder these by hand using your favourite mod ordering utility.</p>"<<endl<<"<p>";
	modfiletime += 60; //time_t is an integer number of seconds, so adding 60 on increases it by a minute.
	for (ModList::Iterator iter = modlist.At(x); iter != modlist.end(); iter++) {
		
		if (!modlist.IsValid(iter)) continue;
		
		ModList::Reference mod = *iter;
		if (!mod) continue;

		modfiletime += 86400; //time_t is an integer number of seconds, so adding 86,400 on increases it by a day.

		bosslog << "Unknown mod file: " << mod;
		//Re-date file. Provide exception handling in case their permissions are wrong.
		try { 
			fs::last_write_time(mod->Filename(), modfiletime);
		} catch(fs::filesystem_error e) {
			bosslog << " - <span class='error'>Error: Could not change the date of \"" << mod << "\", check the Troubleshooting section of the ReadMe for more information and possible solutions.</span>";
		}
		bosslog << endl << "<br /><br />" << endl;
	} //while
	bosslog <<"</p>"<<endl<<"</div><br /><br />"<<endl;

	//Let people know the program has stopped.
	bosslog <<"<div><span>Done.</span></div><br /><br />"<<endl<<"</body>"<<endl<<"</html>";
	bosslog.close();
	system("start BOSS\\BOSSlog.html");	//Displays the BOSSlog.txt.
	return (0);
}

void boss::BOSS::ParseArguments( const ArgumentList& args )
{
	for (ArgumentList::const_iterator iter = args.begin(); iter != args.end(); iter++) {
		const string& arg = to_lower_copy(*iter);

		if (arg == "--update" || arg == "-u") {
			options.update = true;
		} else if (arg == "--disable-version-parse" || arg == "-V-" {
			options.version_parse = false;
		} else if (arg == "--revert-level" || arg == "-r") {

			if (++iter == args.end())
				break;

			//If the correct argument is given, use it. If not, assume that they meant to roll back one level.
			const string value =  *iter;

			if (i+1 < argc) {
				if (strcmp("1", argv[i+1]) == 0 || strcmp("2", argv[i+1]) == 0) 
					revert = atoi(argv[i+1]);
				else 
					revert = 1;
			} 
			else 
				revert = 1;
		} else if (strcmp("--help", argv[i]) == 0 || strcmp("-h", argv[i]) == 0) {
			cout << "Better Oblivion Sorting Software is a utility that sorts the load order of TESIV: Oblivion, TESIII: Morrowind and Fallout 3 mods according to their relative positions on a frequently-updated masterlist ";
			cout << "to ensure proper load order and minimise incompatibilities between mods." << endl << endl;
			cout << "Optional Parameters" << endl << endl;
			cout << "-u, --update: " << endl << "    Automatically updates the local copy of the masterlist using the latest version available on the Google Code repository." << endl << endl;
			cout << "-V-, --disable-version-parse: " << endl << "    Enables the parsing of each mod's description and if found extracts from there the author stamped mod's version and prints it along other data in the generated bosslog.txt." << endl << endl;
			cout << "-r, --revert-level : " << endl << "	Can accept values of 1 or 2. Sets BOSS to revert its changes back the given number of levels." << endl << endl;
			exit (0);
		}
	}
}

boss::Game boss::BOSS::DetectGame()
{

}
