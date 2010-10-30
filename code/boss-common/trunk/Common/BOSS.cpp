/*	Better Oblivion Sorting Software
	1.62
	Quick and Dirty Load Order Utility for Oblivion, Nehrim, Fallout 3 and Fallout: New Vegas
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/
*/


#define NOMINMAX // we don't want the dummy min/max macros since they overlap with the std:: algorithms

#include "BOSS.h"

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


//BOSS [--update | -u] [--help | -h] [--disable-version-parse | -V-]  [--revert-level | -r] [1 | 2]
int main(int argc, char *argv[]) {					
	
	int x;							//random useful integers
	string textbuf,textbuf2;		//a line of text from a file (should usually end up being be a file name);
	time_t esmtime,modfiletime;		//File modification times.
	bool found;
	bool update = false;			//To update masterlist or not?
	bool version_parse = true;		//Enable parsing of mod's headers to look for version strings
	bool silent = false;            //Silent mode?
	bool isghost;					//Is the file ghosted or not?
	int game;						//What game's mods are we sorting? 1 = Oblivion, 2 = Fallout 3, 3 = Nehrim, 4 = Fallout: New Vegas.
	int revert=0;					//What level to revert to?
	
	//Parse command line arguments.
	if (argc > 1) {
		for (int i=0; i < argc; i++) {
			if (strcmp("--update", argv[i]) == 0 || strcmp("-u", argv[i]) == 0) {
				update = true;
			}if (strcmp("--silent-mode", argv[i]) == 0 || strcmp("-s", argv[i]) == 0) {
				silent = true;
			} else if (strcmp("--disable-version-parse", argv[i]) == 0 || strcmp("-V-", argv[i]) == 0) {
				version_parse = false;
			} else if (strcmp("--revert-level", argv[i]) == 0 || strcmp("-r", argv[i]) == 0) {
				//If the correct argument is given, use it. If not, assume that they meant to roll back one level.
				if (i+1<argc) {
					if (strcmp("1", argv[i+1]) == 0 || strcmp("2", argv[i+1]) == 0) revert = atoi(argv[i+1]);
					else revert = 1;
				} else revert = 1;
			} else if (strcmp("--help", argv[i]) == 0 || strcmp("-h", argv[i]) == 0) {
				cout << "Better Oblivion Sorting Software is a utility that sorts the load order of TESIV: Oblivion, Nehrim, Fallout 3 and Fallout: New Vegas mods according to their relative positions on a frequently-updated masterlist ";
				cout << "to ensure proper load order and minimise incompatibilities between mods." << endl << endl;
				cout << "Optional Parameters" << endl << endl;
				cout << "-u, --update: " << endl << "    Automatically updates the local copy of the masterlist using the latest version available on the Google Code repository." << endl << endl;
				cout << "-V-, --disable-version-parse: " << endl << "    Enables the parsing of each mod's description and if found extracts from there the author stamped mod's version and prints it along other data in the generated bosslog.html." << endl << endl;
				cout << "-r, --revert-level : " << endl << "	Can accept values of 1 or 2. Sets BOSS to revert its changes back the given number of levels." << endl << endl;
				cout << "-s, --silent-mode: " << endl << "    Disables launch of the log; just runs silently.." << endl << endl;
				exit (0);
			}
		}
	}

	//Try to create BOSS sub-directory.
	try { fs::create_directory(boss_path);
	} catch(fs::filesystem_error e) {
		cout << "Critical Error: Sub-directory '" << boss_path << "' could not be created!" << endl
			 << "Check the Troubleshooting section of the ReadMe for more information and possible solutions." << endl
			 << "Utility will end now." << endl << endl;
		cout << "Press ENTER to quit...";
		cin.ignore(1,'\n');
		exit(1); //fail in screaming heap.
	}

	//Check for creation of BOSSlog.txt.
	bosslog.open(bosslog_path.external_file_string().c_str());
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
			<< "v1.62 (29 October 2010)"<<endl<<"</div><br /><br />";

	if (fs::exists("Oblivion.esm")) game = 1;
	else if (fs::exists("Fallout3.esm")) game = 2;
	else if (fs::exists("Nehrim.esm")) game = 3;
	else if (fs::exists("FalloutNV.esm")) game = 4;
	else {
		bosslog << endl << "<p class='error'>Critical Error: Master .ESM file not found!<br />" << endl
						<< "Check the Troubleshooting section of the ReadMe for more information and possible solutions.<br />" << endl
						<< "Utility will end now.</p>" << endl
						<< "</body>"<<endl<<"</html>";
		bosslog.close();
		if ( !silent ) 
			Launch(bosslog_path.external_file_string());	//Displays the BOSSlog.txt.
		exit (1); //fail in screaming heap.
	} //else

	if (game==1 && fs::exists("Nehrim.esm")) {
		bosslog << endl << "<p class='error'>Critical Error: Oblivion.esm and Nehrim.esm have both been found!<br />" << endl
						<< "Please ensure that you have installed Nehrim correctly. In a correct install of Nehrim, there is no Oblivion.esm.<br />" << endl
						<< "Utility will end now.</p>" << endl
						<< "</body>"<<endl<<"</html>";
		bosslog.close();
		if ( !silent ) 
			Launch(bosslog_path.external_file_string());	//Displays the BOSSlog.txt.
		exit (1); //fail in screaming heap.
	}

	//Get the master esm's modification date. 
	//Not sure if this needs exception handling, since by this point the file definitely exists. Do it anyway.
	try {
		if (game == 1) esmtime = fs::last_write_time("Oblivion.esm");
		else if (game == 2) esmtime = fs::last_write_time("Fallout3.esm");
		else if (game == 3) esmtime = fs::last_write_time("Nehrim.esm");
		else if (game == 4) esmtime = fs::last_write_time("FalloutNV.esm");
	} catch(fs::filesystem_error e) {
		bosslog << endl << "<p class='error'>Critical Error: Master .ESM file cannot be read!<br />" << endl
						<< "Check the Troubleshooting section of the ReadMe for more information and possible solutions.<br />" << endl
						<< "Utility will end now.</p>" << endl
						<< "</body>"<<endl<<"</html>";
		bosslog.close();
		if ( !silent ) 
			Launch(bosslog_path.external_file_string());	//Displays the BOSSlog.txt.
		exit (1); //fail in screaming heap.
	}

	if (update == true) {
		bosslog << "<div><span>Masterlist Update</span>"<<endl<<"<p>";
		cout << endl << "Updating to the latest masterlist from the Google Code repository..." << endl;
		UpdateMasterlist(game);
		bosslog <<"</p>"<<endl<<"</div><br /><br />"<<endl;
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
				Launch(bosslog_path.external_file_string());	//Displays the BOSSlog.txt.
			exit (1); //fail in screaming heap.
		}
	}
	Rules userlist;
	if (fs::exists(userlist_path) && revert<1) userlist.AddRules();

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
			if ((fcom=fs::exists("FOOK2 - Main.esm"))) bosslog << "FOOK2 Detected.<br />" << endl;
				else bosslog << "FOOK2 not detected.<br />" << endl;
			if (fs::exists("FOOK2 - Main.esp") && !fcom) bosslog << "WARNING: FOOK2.esm seems to be missing.<br />" << endl;
			//Check if fwe or not
			if ((ooo=fs::exists("FO3 Wanderers Edition - Main File.esm"))) bosslog << "FWE detected.<br />" << endl;
				else bosslog << "FWE not detected.<br />" << endl;
		}
		bosslog <<"</p>"<<endl<<"</div><br /><br />"<<endl;
	}

	//open masterlist.txt
	fs::path sortfile;
	if (revert==1) sortfile = curr_modlist_path;	
	else if (revert==2) sortfile = prev_modlist_path;
	else sortfile = masterlist_path;

	order.open(sortfile.external_file_string().c_str());

	if (order.fail()) {							
		bosslog << endl << "<p class='error'>Critical Error: ";

		bosslog << sortfile.external_file_string();

		bosslog << " cannot be read!<br />" << endl
				<< "Check the Troubleshooting section of the ReadMe for more information and possible solutions.<br />" << endl
				<< "Utility will end now.</p>" << endl
				<< "</body>"<<endl<<"</html>";
		bosslog.close();
		if ( !silent ) 
			Launch(bosslog_path.external_file_string());	//Displays the BOSSlog.txt.
		exit (1); //fail in screaming heap.
	} //if

	x=0;
	found=false;
		while (!order.eof()) {					
		textbuf=ReadLine("order");
		if (textbuf.length()>1 && textbuf[0]!='\\') {		//Filter out blank lines, oblivion.esm and remark lines starting with \.
			if (!IsMessage(textbuf)) {						//Deal with mod lines only here. Message lines will be dealt with below.
				isghost = false;
				if (fs::exists(textbuf+".ghost") && !fs::exists(textbuf)) isghost = true;
				if (fs::exists(textbuf) || isghost) {					//Tidy function not needed as file system removes trailing spaces and isn't case sensitive

					const string& filename = isghost ? textbuf+".ghost" : textbuf;

					int i = modlist.GetModIndex(filename);
					if (i < 0 || i >= int(modlist.mods.size()))
						continue;
					
					found=true;

					// Save current mod's messages for later
					vector<string> messages = modlist.modmessages[i];

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

	if (fs::exists(userlist_path) && revert<1) {
		bosslog << "<div><span>Userlist Messages</span>"<<endl<<"<p>";
		//Go through each rule.
		for (int i=0;i<(int)userlist.rules.size();i++) {
			int start = userlist.rules[i];
			int end;
			if (i==(int)userlist.rules.size()-1) end = (int)userlist.keys.size();
			else end = userlist.rules[i+1];
			//Go through each line of the rule. The first line is given by keys[start] and objects[start].
			for (int j=start;j<end;j++) {
				//A mod sorting line.
				if ((userlist.keys[j]=="BEFORE" || userlist.keys[j]=="AFTER") && IsPlugin(userlist.objects[j])) {
					vector<string> currentmessages;
					int index,index1;
					//Get current mod messages and remove mod from current modlist position.
					index1 = modlist.GetModIndex(userlist.objects[start]);
					// Only increment 'x' if we've taken the 'source' mod from below the 'last-sorted' mark
					if (userlist.keys[start]=="ADD" && index1 >= x) x++;
					//If it adds a mod already sorted, skip the rule.
					else if (userlist.keys[start]=="ADD"  && index1 < x) {
						userlist.messages += "\""+userlist.objects[start]+"\" is already in the masterlist. Rule skipped.<br /><br />";
						break;
					}
					string filename = modlist.mods[index1];
					currentmessages.assign(modlist.modmessages[index1].begin(),modlist.modmessages[index1].end());
					modlist.mods.erase(modlist.mods.begin()+index1);
					modlist.modmessages.erase(modlist.modmessages.begin()+index1);
					//Need to insert mod and mod's messages to a specific position.
					index = modlist.GetModIndex(userlist.objects[j]);

					//Let's take this up to 11 - super awesome stuff beginning.
					//The sort mod isn't installed - so scour the [s]Shire[/s] masterlist for it.
					if (index < 0 || index >= int(modlist.mods.size())) {
						bool lookforinstalledmod=false;
						order.open(masterlist_path.external_file_string().c_str());
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
										if (userlist.keys[j]=="AFTER") index -= 1;
										break;
									}
								}
							}
						}
						order.close();
						if (index < 0 || index >= int(modlist.mods.size())) {
							userlist.messages += "\""+userlist.objects[j]+"\" is not installed, and is not in the masterlist. Rule skipped.<br /><br />";
							modlist.mods.insert(modlist.mods.begin()+index1,filename);
							modlist.modmessages.insert(modlist.modmessages.begin()+index1,currentmessages);
							break;
						}
					}
					//Uh oh, the awesomesauce ran out...

					if (userlist.keys[j]=="AFTER") index += 1;
					modlist.mods.insert(modlist.mods.begin()+index,filename);
					modlist.modmessages.insert(modlist.modmessages.begin()+index,currentmessages);
					userlist.messages += "\""+userlist.objects[start]+"\" has been sorted "+Tidy(userlist.keys[j]) + " \"" + userlist.objects[j] + "\".<br /><br />";
				//A group sorting line.
				} else if ((userlist.keys[j]=="BEFORE" || userlist.keys[j]=="AFTER") && !IsPlugin(userlist.objects[j])) {
					//Search masterlist for rule group. Once found, search it for mods in modlist, recording the mods that match.
					//Then search masterlist for sort group. Again, search and record matching modlist mods.
					//If sort keyword is BEFORE, discard all but the first recorded sort group mod, and if it is AFTER, discard all but the last recorded sort group mod.
					//Then insert the recorded rule group mods before or after the remaining sort group mod and erase them from their old positions.
					//Remember to move their messages too.

					order.open(masterlist_path.external_file_string().c_str());
					int count=0;
					bool lookforrulemods=false,lookforsortmods=false;
					vector<string> rulemods,sortmods,currentmessages;
					while (!order.eof()) {					
						textbuf=ReadLine("order");
						if (textbuf.length()>1 && (textbuf.substr(1,10)=="BeginGroup" || textbuf.substr(1,8)=="EndGroup")) {
							//A group starts or ends. Check rule to see if it matches any.
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
								if (fs::exists(textbuf) || fs::exists(textbuf+".ghost")) {
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
					if (userlist.keys[j]=="BEFORE") {
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
					} else if (userlist.keys[j]=="AFTER") {	
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
					userlist.messages += "The group \""+userlist.objects[start]+"\" has been sorted "+Tidy(userlist.keys[j]) + " the group \"" + userlist.objects[j] + "\".<br /><br />";
				//An insertion line.
				} else if (userlist.keys[j]=="TOP" || userlist.keys[j]=="BOTTOM") {
					vector<string> currentmessages;
					//Get current mod messages and remove mod from current modlist position.
					int index1 = modlist.GetModIndex(userlist.objects[start]);
					// Only increment 'x' if we've taken the 'source' mod from below the 'last-sorted' mark
					if (userlist.keys[start]=="ADD" && index1 >= x) x++;
					//If it adds a mod already sorted, skip the rule.
					else if (userlist.keys[start]=="ADD"  && index1 < x) {
						userlist.messages += "\""+userlist.objects[start]+"\" is already in the masterlist. Rule skipped.<br /><br />";
						break;
					}
					string filename = modlist.mods[index1];
					currentmessages.assign(modlist.modmessages[index1].begin(),modlist.modmessages[index1].end());
					modlist.mods.erase(modlist.mods.begin()+index1);
					modlist.modmessages.erase(modlist.modmessages.begin()+index1);
					//Need to insert mod and mod's messages to a specific position.
					//This is the tricky bit.
					order.open(masterlist_path.external_file_string().c_str());
					int count=0;
					bool lookforsortmods=false,overtime=false;
					vector<string> sortmods;
					while (!order.eof()) {					
						textbuf=ReadLine("order");
						if (textbuf.length()>1 && (textbuf.substr(1,10)=="BeginGroup" || textbuf.substr(1,8)=="EndGroup")) {
							//A group starts or ends. Check rule to see if it matches.
							if (textbuf.substr(1,10)=="BeginGroup") {
								if (Tidy(userlist.objects[j])==Tidy(textbuf.substr(14))) {
									//Sort group match. Now search for lines that match something in modlist.
									lookforsortmods=true;
									count = 0;
								}
								count += 1;
							} else if (count>0 && textbuf.substr(1,8)=="EndGroup") {
								count -= 1;
								if (count==0) {
									if ((int)sortmods.size()>0) {
										//The end of the matched group has been found, and we have found at least one mod in that group. Stop searching for installed mods.
										lookforsortmods=false;
										break;
									} else {
										//The end of the matched group was found, but we still don't have any mods to sort relative to.
										//Keep searching for mods, but now TOP and BOTTOM both mean "before the mod found next".
										overtime=true;
									}
								}
							}
						} else if (lookforsortmods && textbuf[0]!='\\') {
							if (!IsMessage(textbuf)) {
								if (fs::exists(textbuf) || fs::exists(textbuf+".ghost")) {
									//Found a mod.
									int gm = modlist.GetModIndex(textbuf);
									sortmods.push_back(modlist.mods[gm]);
									if (overtime) break;	//Stop looking for more mods immediately.
								}
							}
						}
					}
					order.close();
					int index;
					if ((int)sortmods.size()>0) {
						if (userlist.keys[j]=="TOP") 
							index = modlist.GetModIndex(sortmods.front());
						else if (userlist.keys[j]=="BOTTOM") {
							index = modlist.GetModIndex(sortmods.back());
							if (!overtime) index += 1;
						}
					} else index = x-1;		//Not really sure why this works, but it does. IMO it should be =x when using OVERRIDE, but =x-1 works fine.
					modlist.mods.insert(modlist.mods.begin()+index,filename);
					modlist.modmessages.insert(modlist.modmessages.begin()+index,currentmessages);
					if (userlist.keys[j]=="TOP") 
						userlist.messages += "\""+userlist.objects[start]+"\" inserted into the top of group \"" + userlist.objects[j] + "\".<br /><br />";
					else if (userlist.keys[j]=="BOTTOM") 
						userlist.messages += "\""+userlist.objects[start]+"\" inserted into the bottom of group \"" + userlist.objects[j] + "\".<br /><br />";
			
				//A message line.
				} else if (userlist.keys[j]=="APPEND" || userlist.keys[j]=="REPLACE") {
					//Look for the modlist line that contains the match mod of the rule.
					int index = modlist.GetModIndex(userlist.objects[start]);
					userlist.messages += "\"" + userlist.objects[j] + "\"";
					if (userlist.keys[j]=="APPEND") {			//Attach the rule message to the mod's messages list.
						userlist.messages += " appended to ";
					} else if (userlist.keys[j]=="REPLACE") {	//Clear the message list and then attach the message.
						modlist.modmessages[index].clear();
						userlist.messages += " replaced ";
					}
					modlist.modmessages[index].push_back(userlist.objects[j]);
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

	x = min(x, int(modlist.mods.size()));

	for (int i=0;i<x;i++) {
		bool ghosted = false;
		string filename;
		if (Tidy(modlist.mods[i].substr(modlist.mods[i].length()-6))==".ghost") {
			ghosted=true;
			filename = modlist.mods[i].substr(0,modlist.mods[i].length()-6);
		} else filename = modlist.mods[i];
		string text = version_parse ? GetModHeader(filename,ghosted) : filename;
		if (ghosted) text += " <em> - Ghosted</em>";
		if (modlist.modmessages[i].size()>0) bosslog << "<b>" << text << "</b>";		// show which mod file is being processed.
		else bosslog << text;
		modfiletime=esmtime;
		modfiletime += i*60; //time_t is an integer number of seconds, so adding 60 on increases it by a minute.
		if (IsValidLine(modlist.mods[i])) {
			//Re-date file. Provide exception handling in case their permissions are wrong.
			try { fs::last_write_time(modlist.mods[i],modfiletime);
			} catch(fs::filesystem_error e) {
				bosslog << " - <span class='error'>Error: Could not change the date of \"" << modlist.mods[i] << "\", check the Troubleshooting section of the ReadMe for more information and possible solutions.</span>";
			}
		}
		if (modlist.modmessages[i].size()==0) bosslog << endl << "<br /><br />" << endl;
		if (modlist.modmessages[i].size()>0) {
			bosslog << endl << "<ul>" << endl;
			for (int j=0;j<(int)modlist.modmessages[i].size();j++) {
				ShowMessage(modlist.modmessages[i][j], game);		//Deal with message lines here.
			}
			bosslog << "</ul>" << endl;
		}
	}
	
	bosslog <<"</p>"<<endl<<"</div><br /><br />"<<endl;

	//Find and show found mods not recognised. These are the mods that are found at and after index x in the mods vector.
	//Order their dates to be +1 month after the master esm to ensure they load last.
	bosslog << "<div><span>Unrecogised Mod Files</span><p>Reorder these by hand using your favourite mod ordering utility.</p>"<<endl<<"<p>";
	for (int i=x;i<(int)modlist.mods.size();i++) {
		if (modlist.mods[i].length()>1) {
			if (modlist.mods[i].find(".ghost") != string::npos) bosslog << "Unknown mod file: " << modlist.mods[i].substr(0,modlist.mods[i].length()-6) << " <em> - Ghosted</em>";
			else bosslog << "Unknown mod file: " << modlist.mods[i];
			modfiletime=esmtime;
			modfiletime += i*60; //time_t is an integer number of seconds, so adding 60 on increases it by a minute.
			modfiletime += i*86400; //time_t is an integer number of seconds, so adding 86,400 on increases it by a day.
			//Re-date file. Provide exception handling in case their permissions are wrong.
			try { fs::last_write_time(modlist.mods[i],modfiletime);
			} catch(fs::filesystem_error e) {
				bosslog << " - <span class='error'>Error: Could not change the date of \"" << modlist.mods[i] << "\", check the Troubleshooting section of the ReadMe for more information and possible solutions.</span>";
			}
			bosslog << endl << "<br /><br />" << endl;
		}
	} //while
	bosslog <<"</p>"<<endl<<"</div><br /><br />"<<endl;

	//Let people know the program has stopped.
	bosslog <<"<div><span>Done.</span></div><br /><br />"<<endl<<"</body>"<<endl<<"</html>";
	bosslog.close();
	if ( !silent ) 
		Launch(bosslog_path.external_file_string());	//Displays the BOSSlog.txt.
	return (0);
}