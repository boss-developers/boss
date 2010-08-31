/*	Better Oblivion Sorting Software
	1.6
	Quick and Dirty Load Order Utility for Oblivion, Fallout 3 and Morrowind
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/
*/

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/utime.h>

#include "BOSS.h"

#define SIZE 26 				//used in conversion of date/time struct to a string. Has to be this length.
#define MAXLENGTH 4096			//maximum length of a file name or comment. Big arbitrary number.

using namespace boss;

//BOSS [--update | -u] [--help | -h] [--version-check | -V] [--revert-level | -r]
int main(int argc, char *argv[]) {					
	
	int x;							//random useful integers
	string textbuf,textbuf2;		//a line of text from a file (should usually end up being be a file name); 			
	struct __stat64 buf;			//temp buffer of info for _stat function
	struct tm esmtime;			    //the modification date/time of the main .esm file
	struct tm modfiletime;			//useful variable to store a file's date/time
	bool found=false,update=false,version_parse=true,isghost;		//Mod found? Update masterlist? Parse versions? Is mod a ghost?
	int game=0,revert=0;						//What game's mods are we sorting? 1 = Oblivion, 2 = Fallout 3, 3 = Morrowind. What level to revert to?

	//Parse command line arguments.
	if (argc > 1) {
		for (int i=0; i < argc; i++) {
			if (strcmp("--update", argv[i]) == 0 || strcmp("-u", argv[i]) == 0) {
				update = true;
			} else if (strcmp("--version-check", argv[i]) == 0 || strcmp("-V", argv[i]) == 0) {
				version_parse = true;
			} else if (strcmp("--revert-level", argv[i]) == 0 || strcmp("-r", argv[i]) == 0) {
				revert = stoi(argv[i+1]);
			} else if (strcmp("--help", argv[i]) == 0 || strcmp("-h", argv[i]) == 0) {
				cout << "Better Oblivion Sorting Software is a utility that sorts the load order of TESIV: Oblivion, TESIII: Morrowind and Fallout 3 mods according to their relative positions on a frequently-updated masterlist ";
				cout << "to ensure proper load order and minimise incompatibilities between mods." << endl << endl;
				cout << "Optional Parameters" << endl << endl;
				cout << "-u, --update: " << endl << "    Automatically updates the local copy of the masterlist using the latest version available on the Google Code repository." << endl << endl;
				cout << "-V, --version-check: " << endl << "    Enables the parsing of each mod's description and if found extracts from there the author stamped mod's version and prints it along other data in the generated bosslog.txt." << endl << endl;
				exit(0);
			}
		}
	}

	//Try to create BOSS sub-directory.
	try { boost::filesystem::create_directory("BOSS\\");
	} catch(boost::filesystem::filesystem_error e) {
		cout << "Critical Error! Sub-directory \"Data\\BOSS\\\" could not be created." << endl
			 << "Check your permissions and make sure you have write access to your Data folder." << endl
			 << "! Utility will end now." << endl << endl;
		cout << "Press ENTER to quit...";
		cin.ignore(1,'\n');
		exit(1); //fail in screaming heap.
	}

	//Check for creation of BOSSlog.txt.
	bosslog.open("BOSS\\BOSSlog.html");
	if (bosslog.fail()) {							
		cout << endl << "Critical Error! BOSSlog.html could not be written to." << endl
					 << "Check your permissions and make sure you have write access to your Data\\BOSS folder." << endl
					 << "! Utility will end now." << endl << endl;
		cout << "Press ENTER to quit...";
		cin.ignore(1,'\n');
		exit (1); //fail in screaming heap.
	}

	//Output HTML start and <head>
	bosslog << "<!DOCTYPE html>"<<endl<<"<html>"<<endl<<"<head>"<<endl<<"<meta http-equiv='Content-Type' content='text/html; charset=UTF-8'>"<<endl
			<< "<title>BOSS Log</title>"<<endl<<"<style type='text/css'>"<<endl<<"#body {font-family:Calibri,Arial,Verdana,sans-serifs;}"<<endl
			<< "#title {font-size:2.4em; font-weight:bold; text-align: center;}"<<endl<<"div > span {font-weight:bold; font-size:1.3em;}"<<endl
			<< "ul li {margin-bottom:10px;}"<<endl<<".error {color:red;}"<<endl<<"</style>"<<endl<<"</head>"<<endl
			//Output start of <body>
			<< "<body id='body'>"<<endl<<"<div id='title'>Better Oblivion Sorting Software Log</div><br />"<<endl
			<< "<div style='text-align:center;'>&copy; Random007 &amp; the BOSS development team, 2009-2010. Some rights reserved.<br />"<<endl
			<< "<a href='http://creativecommons.org/licenses/by-nc-nd/3.0/'>CC Attribution-Noncommercial-No Derivative Works 3.0</a><br />"<<endl
			<< "v1.6 (20 August 2010)"<<endl<<"</div><br /><br />";

	if (FileExists("oblivion.esm")) game = 1;
	else if (FileExists("fallout3.esm")) game = 2;
	else if (FileExists("morrowind.esm")) game = 3;
	else {
		bosslog << endl << "<p class='error'>Critical Error: Master .ESM file cannot be read.<br />" << endl
						<< "Make sure you're running this in your Data folder.<br />" << endl
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
		else cout << "Masterlist update failed." << endl;
	}

	cout << endl << "Better Oblivion Sorting Software working..." << endl;

	Mods modlist;
	modlist.AddMods();
	if (revert<1) {
		if (!modlist.SaveModList(bosslog)) {
			bosslog.close();
			system("start BOSS\\BOSSlog.html");	//Displays the BOSSlog.txt.
			exit (1); //fail in screaming heap.
		}
	}
	Rules userlist;
	if (boost::filesystem::exists("BOSS\\userlist.txt") && revert<1) userlist.AddRules();
	
	if (revert<1) {
		bosslog << "<div><span>Special Mod Detection</span>"<<endl<<"<p>";
		if (game == 1) {
			//Check if FCOM or not
			if (fcom=FileExists("FCOM_Convergence.esm")) bosslog << "FCOM detected.<br />" << endl;
				else bosslog << "FCOM not detected.<br />" << endl;
			if (FileExists("FCOM_Convergence.esp") && !fcom) bosslog << "WARNING: FCOM_Convergence.esm seems to be missing.<br />" << endl;
			//Check if OOO or not
			if (ooo=FileExists("Oscuro's_Oblivion_Overhaul.esm")) bosslog << "OOO detected.<br />" << endl;
				else bosslog << "OOO not detected.<br />" << endl;
			//Check if Better Cities or not
			if (bc=FileExists("Better Cities Resources.esm")) bosslog << "Better Cities detected.<br />" << endl;
				else bosslog << "Better Cities not detected.<br />" << endl;
		} else if (game == 2) {
			//Check if fook2 or not
			if (fook2=FileExists("FOOK2 - Main.esm")) bosslog << "FOOK2 Detected.<br />" << endl;
				else bosslog << "FOOK2 not detected.<br />" << endl;
			if (FileExists("FOOK2 - Main.esp") && !fook2) bosslog << "WARNING: FOOK2.esm seems to be missing.<br />" << endl;
			//Check if fwe or not
			if (fwe=FileExists("FO3 Wanderers Edition - Main File.esm")) bosslog << "FWE detected.<br />" << endl;
				else bosslog << "FWE not detected.<br />" << endl;
		}
		bosslog <<"</p>"<<endl<<"</div><br /><br />"<<endl;
	}

	//open masterlist.txt
	if (revert==1) order.open("BOSS\\modlist.txt");	
	else if (revert==2) order.open("BOSS\\modlist.old");	
	else order.open("BOSS\\masterlist.txt");
	if (order.fail()) {							
		bosslog << endl << "<p class='error'>Critical Error! ";

		if (revert==1) bosslog << "BOSS\\modlist.txt";	
		else if (revert==2) bosslog << "BOSS\\modlist.old";	
		else bosslog << "BOSS\\masterlist.txt";

		bosslog << " cannot be read. Make sure it exists.<br />" << endl
						<< "Utility will end now.</p>" << endl
						<< "</body>"<<endl<<"</html>";
		bosslog.close();
		system("start BOSS\\BOSSlog.html");	//Displays the BOSSlog.txt.
		exit (1); //fail in screaming heap.
	} //if

	x=0;
	found=false;
		while (!order.eof()) {					
		textbuf=ReadLine("order");
		if (IsValidLine(textbuf) && textbuf[0]!='\\') {		//Filter out blank lines, oblivion.esm and remark lines starting with \.
			if (!IsMessage(textbuf)) {						//Deal with mod lines only here. Message lines will be dealt with below.
				isghost = false;
				if (FileExists(textbuf+".ghost") && !FileExists(textbuf)) isghost = true;
				if (FileExists(textbuf) || isghost) {					//Tidy function not needed as file system removes trailing spaces and isn't case sensitive
					found=true;
					int i;
					if (isghost) i = modlist.GetModIndex(textbuf+".ghost");
					else i = modlist.GetModIndex(textbuf);		//Remove ordered files from modlist class.
					modlist.mods.erase(modlist.mods.begin()+i);
					if (isghost) modlist.mods.insert(modlist.mods.begin()+x,textbuf+".ghost");
					else modlist.mods.insert(modlist.mods.begin()+x,textbuf);
					if (revert<1) {
						i = userlist.GetRuleIndex(textbuf);
						if (i>-1 && userlist.keys[i]=="ADD") {
							userlist.messages += "\""+userlist.objects[i]+"\" is already in the masterlist. Rule skipped.<br /><br />";
							int ruleindex;
							for (int j=0;j<(int)userlist.rules.size();j++) {
								if (i==userlist.rules[j]) {
									ruleindex = j;
									break;
								}
							}
							if (ruleindex+1==(int)userlist.rules.size()) {
								for (int j=i;j<(int)userlist.keys.size();j++) {
									userlist.keys[j]="";
									userlist.objects[j]="";
								}
							} else {
								for (int j=i;j<userlist.rules[ruleindex+1];j++) {
									userlist.keys[j]="";
									userlist.objects[j]="";
								}
							}
							userlist.rules.erase(userlist.rules.begin()+ruleindex);		
						}
					}
					x++;
				} //if
				else found=false;
			} //if
			else if (found) modlist.modmessages[x-1].push_back(textbuf);
		} //if
	} //while
	order.close();		//Close the masterlist stream, as it's not needed any more.

	if (boost::filesystem::exists("BOSS\\userlist.txt") && revert<1) {
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
					int index1 = modlist.GetModIndex(userlist.objects[start]);
					string filename = modlist.mods[index1];
					currentmessages.assign(modlist.modmessages[index1].begin(),modlist.modmessages[index1].end());
					modlist.mods.erase(modlist.mods.begin()+index1);
					modlist.modmessages.erase(modlist.modmessages.begin()+index1);
					//Need to insert mod and mod's messages to a specific position.
					int index = modlist.GetModIndex(userlist.objects[j]);
					if (userlist.keys[j]=="AFTER") index += 1;
					modlist.mods.insert(modlist.mods.begin()+index,filename);
					modlist.modmessages.insert(modlist.modmessages.begin()+index,currentmessages);
					userlist.messages += "\""+userlist.objects[start]+"\" has been sorted "+Tidy(userlist.keys[j]) + " \"" + userlist.objects[j] + "\".<br /><br />";
				} else if ((userlist.keys[j]=="BEFORE" || userlist.keys[j]=="AFTER") && !IsPlugin(userlist.objects[j])) {
					//Search masterlist for rule group. Once found, search it for mods in modlist, recording the mods that match.
					//Then search masterlist for sort group. Again, search and record matching modlist mods.
					//If sort keyword is BEFORE, discard all but the first recorded sort group mod, and if it is AFTER, discard all but the last recorded sort group mod.
					//Then insert the recorded rule group mods before or after the remaining sort group mod and erase them from their old positions.
					//Remember to move their messages too.
					//Seriously hacky this is, but I can't think of a better solution with what we've got.

					order.open("BOSS\\masterlist.txt");
					int count=0;
					bool lookforrulemods,lookforsortmods;
					vector<string> rulemods,sortmods,currentmessages;
					while (!order.eof()) {					
						textbuf=ReadLine("order");
						if (IsValidLine(textbuf) && (textbuf.substr(1,10)=="BeginGroup" || textbuf.substr(1,8)=="EndGroup")) {
							//A group starts or ends. Search rules to see if it matches any.
							if (textbuf.substr(1,10)=="BeginGroup") {
								if (Tidy(userlist.objects[start])==Tidy(textbuf.substr(14))) {
									//Rule match. Now search for a line that matches something in modlist.
									lookforrulemods=true;
									lookforsortmods=false;
									count = 0;
								} else if (Tidy(userlist.objects[j])==Tidy(textbuf.substr(14))) {
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
								isghost = false;
								if (FileExists(textbuf+".ghost") && !FileExists(textbuf)) isghost = true;
								if (FileExists(textbuf) || isghost) {
									//Found a mod.
									int i;
									if (isghost) i = modlist.GetModIndex(textbuf+".ghost");
									else i = modlist.GetModIndex(textbuf);
									if (lookforrulemods) {
										rulemods.push_back(modlist.mods[i]);
									} else if (lookforsortmods) {
										sortmods.push_back(modlist.mods[i]);
									}
								}
							}
						}
					}
					order.close();
					if (userlist.keys[j]=="BEFORE") {
						for (int k=0;k<(int)rulemods.size();k++) {
							int index = modlist.GetModIndex(sortmods.front())-1;
							int index1 = modlist.GetModIndex(rulemods[k]);
							currentmessages.assign(modlist.modmessages[index1].begin(),modlist.modmessages[index1].end());
							modlist.mods.erase(modlist.mods.begin()+index1);
							modlist.modmessages.erase(modlist.modmessages.begin()+index1);
							modlist.mods.insert(modlist.mods.begin()+index,rulemods[k]);
							modlist.modmessages.insert(modlist.modmessages.begin()+index,currentmessages);
						}
					} else if (userlist.keys[j]=="AFTER") {	
						for (int k=(int)rulemods.size()-1;k>-1;k--) {
							int index = modlist.GetModIndex(sortmods.back());
							int index1 = modlist.GetModIndex(rulemods[k]);
							currentmessages.assign(modlist.modmessages[index1].begin(),modlist.modmessages[index1].end());
							modlist.mods.erase(modlist.mods.begin()+index1);
							modlist.modmessages.erase(modlist.modmessages.begin()+index1);
							modlist.mods.insert(modlist.mods.begin()+index,rulemods[k]);
							modlist.modmessages.insert(modlist.modmessages.begin()+index,currentmessages);
						}
					}
					userlist.messages += "The group \""+userlist.objects[start]+"\" has been sorted "+Tidy(userlist.keys[j]) + " the group \"" + userlist.objects[j] + "\".<br /><br />";

				} else if (userlist.keys[j]=="APPEND" || userlist.keys[j]=="REPLACE") {
					//Look for the modlist line that contains the match mod of the rule.
					int index = modlist.GetModIndex(userlist.objects[start]);
					userlist.messages += "\"" + userlist.objects[j] + "\"";
					if (userlist.keys[j]=="APPEND") {			//Attach the rule message to the mod's messages list.
						modlist.modmessages[index].push_back(userlist.objects[j]);
						userlist.messages += " appended to ";
					} else if (userlist.keys[j]=="REPLACE") {	//Clear the message list and then attach the message.
						modlist.modmessages[index].clear();
						modlist.modmessages[index].push_back(userlist.objects[j]);
						userlist.messages += " replaced ";
					}
					userlist.messages += "messages attached to \"" + userlist.objects[start] + "\".<br /><br />";
				}
			}
		}
		userlist.PrintMessages(bosslog);
		bosslog <<"</p>"<<endl<<"</div><br /><br />"<<endl;
	}

	//Remove any read only attributes from esm/esp files if present.
	//This isn't very neat, since it'll print an error message if you don't have any ghosted plugins. Also, apparently system() calls are evil.
	//I'm not aware that you can do it with BOOST though. :(
	//I'd also like to be able to print error messages if this doesn't work.
	system("attrib -r *.es?");
	system("attrib -r *.ghost");

	//get date for master .esm. Again, a filesystem interaction, we should have error handling for this.
	//Now this CAN be done with BOOST, which would be better since it's more standard. I'll possibly convert it later.
	if (game == 1) _stat64("Oblivion.esm", &buf);
	else if (game == 2) _stat64("Fallout3.esm", &buf);
	else if (game == 3) _stat64("morrowind.esm", &buf);
	_gmtime64_s(&esmtime, &buf.st_mtime);		//convert _stat64 modification date data to date/time struct.

	//Re-order .esp/.esm files to masterlist.txt order and output messages
	if (revert<1) bosslog << "<div><span>Recognised And Re-ordered Mod Files</span>"<<endl<<"<p>";
	else if (revert==1) bosslog << "<div><span>Restored Load Order (Using modlist.txt)</span>"<<endl<<"<p>";
	else if (revert==2) bosslog << "<div><span>Restored Load Order (Using modlist.old)</span>"<<endl<<"<p>";
	for (int i=0;i<x;i++) {
		bool ghosted = false;
		string filename;
		if (Tidy(modlist.mods[i].substr(modlist.mods[i].length()-6))==".ghost") {
			ghosted=true;
			filename = modlist.mods[i].substr(0,modlist.mods[i].length()-6);
		} else filename = modlist.mods[i];
		string text = version_parse ? GetModHeader(filename,ghosted) : filename;
		if (ghosted) text += " <em> - Ghosted</em>";
		if (modlist.modmessages[i].size()>0) bosslog << "<b>" << text << "</b>" << endl;		// show which mod file is being processed.
		else bosslog << text << "<br /><br />" << endl;
		modfiletime=esmtime;
		modfiletime.tm_min += i;				//files are ordered in minutes after the master esm file. Again, a filesystem interaction, we should have error handling for this.
		ChangeFileDate(modlist.mods[i], modfiletime);
		if (modlist.modmessages[i].size()>0) {
			bosslog << "<ul>" << endl;
			for (int j=0;j<(int)modlist.modmessages[i].size();j++) {
				ShowMessage(modlist.modmessages[i][j], fcom, ooo, bc, fook2, fwe);		//Deal with message lines here.
			}
			bosslog << "</ul>" << endl;
		}
	}

	bosslog <<"</p>"<<endl<<"</div><br /><br />"<<endl;

	//Find and show found mods not recognised. These are the mods that are found at and after index x in the mods vector.
	//Order their dates to be +1 month after the master esm to ensure they load last.
	//Again, a filesystem interaction, we should have error handling for this.
	bosslog << "<div><span>Unrecogised Mod Files</span><p>Reorder these by hand using your favourite mod ordering utility.</p>"<<endl<<"<p>";
	for (int i=x;i<(int)modlist.mods.size();i++) {
		if (modlist.mods[i].length()>1) {
			bosslog << "Unknown mod file: " << modlist.mods[i] << "<br /><br />" << endl;
			modfiletime=esmtime;
			modfiletime.tm_mon+=1;					//shuffle all mods foward a month 
			modfiletime.tm_min=i;					//and order (in minutes) to original order
			ChangeFileDate(modlist.mods[i], modfiletime);
		}
	} //while
	bosslog <<"</p>"<<endl<<"</div><br /><br />"<<endl;

	//Let people know the program has stopped.
	bosslog <<"<div><span>Done.</span></div><br /><br />"<<endl<<"</body>"<<endl<<"</html>";
	bosslog.close();
	system("start BOSS\\BOSSlog.html");	//Displays the BOSSlog.txt.
	return (0);
}