/*	Better Oblivion Sorting Software
	1.6
	Quick and Dirty Load Order Utility for Oblivion, Fallout 3 and Morrowind
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/
*/

#include <stdio.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <ctype.h>
#include <direct.h>
#include <sstream>
#include <time.h>
#include <sys/stat.h>
#include <sys/utime.h>

#include "BOSS.h"
#include "Userlist.h"

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
	bool found=false,update=false,version_parse=false,isghost;		//Mod found? Update masterlist? Parse versions? Is mod a ghost?
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
				cout << revert;
			} else if (strcmp("--help", argv[i]) == 0 || strcmp("-h", argv[i]) == 0) {
				cout << "Better Oblivion Sorting Software is a utility that sorts the load order of TESIV: Oblivion, TESIII: Morrowind and Fallout 3 mods according to their relative positions on a frequently-updated masterlist ";
				cout << "to ensure proper load order and minimise incompatibilities between mods." << endl << endl;
				cout << "Optional Parameters" << endl << endl;
				cout << "-u, --update: " << endl << "    Automatically updates the local copy of the masterlist using the latest version available on the Google Code repository." << endl << endl;
				cout << "-V, --version-check: " << endl << "    Enables the parsing of each mod's description and if found extracts from there the author stamped mod's version and prints it along other data in the generated bosslog.txt." << endl << endl;
				exit (0);
			}
		}
	}

	//Check for creation of BOSSlog.txt.
	bosslog.open("BOSS\\BOSSlog.html");
	if (bosslog.fail()) {							
		cout << endl << "Critical Error! BOSSlog.txt should have been created but it wasn't." << endl
					 << "Make sure you are running as Administrator if using Windows Vista or Windows 7." << endl
					 << "! Utility will end now." << endl;
		exit (1); //fail in screaming heap.
	}

	//Output HTML start and <head>
	bosslog << "<!DOCTYPE html>"<<endl<<"<html>"<<endl<<"<head>"<<endl<<"<meta http-equiv='Content-Type' content='text/html; charset=UTF-8'>"<<endl
			<< "<title>BOSS Log</title>"<<endl<<"<style type='text/css'>"<<endl<<"#body {font-family:Calibri,Arial,Verdana,sans-serifs;}"<<endl
			<< "#title {font-size:2.4em; font-weight:bold; text-align: center;}"<<endl<<"div > span {font-weight:bold; font-size:1.3em;}"<<endl
			<< "ul li {margin-bottom:10px;}"<<endl<<"</style>"<<endl<<"</head>"<<endl
			//Output start of <body>
			<< "<body id='body'>"<<endl<<"<div id='title'>Better Oblivion Sorting Software Log</div><br />"<<endl
			<< "<div style='text-align:center;'>&copy; Random007 &amp; the BOSS development team, 2009-2010. Some rights reserved.<br />"<<endl
			<< "<a href='http://creativecommons.org/licenses/by-nc-nd/3.0/'>CC Attribution-Noncommercial-No Derivative Works 3.0</a><br />"<<endl
			<< "v1.6 (20 August 2010)"<<endl<<"</div><br /><br />";

	if (FileExists("oblivion.esm")) game = 1;
	else if (FileExists("fallout3.esm")) game = 2;
	else if (FileExists("morrowind.esm")) game = 3;
	else {
		bosslog << endl << "<p style='color:red;'>Critical Error: Master .ESM file not found (or not accessible)!<br />" << endl
						<< "Make sure you're running this in your Data folder.<br />" << endl
						<< "Utility will end now.</p>" << endl
						<< "</body>"<<endl<<"</html>";
		bosslog.close();
		system("start BOSS\\BOSSlog.html");	//Displays the BOSSlog.txt.
		exit (1); //fail in screaming heap.
	}

	CreateDirectory("BOSS\\",NULL);

	if (update == true) {
		cout << endl << "Updating to the latest masterlist from the Google Code repository..." << endl;
		int rev = UpdateMasterlist(game);
		if (rev > 0) cout << "masterlist.txt updated to revision " << rev << endl;
		else cout << "Masterlist update failed." << endl;
	}

	cout << endl << "Better Oblivion Sorting Software working..." << endl;

	Mods modlist;
	modlist.AddMods();
	if (revert<1) modlist.SaveModList();
	

	
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

	//Remove any read only attributes from esm/esp files if present.
	system("attrib -r *.es?");

	//get date for master .esm.
	if (game == 1) _stat64("oblivion.esm", &buf);
	else if (game == 2) _stat64("fallout3.esm", &buf);
	else if (game == 3) _stat64("morrowind.esm", &buf);
	_gmtime64_s(&esmtime, &buf.st_mtime);		//convert _stat64 modification date data to date/time struct.

	//open masterlist.txt
	if (revert==1) order.open("BOSS\\modlist.txt");	
	else if (revert==2) order.open("BOSS\\modlist.old");	
	else order.open("BOSS\\masterlist.txt");	
	if (order.fail()) {							
		bosslog << endl << "<p style='color:red;'>Critical Error! ";

		if (revert==1) bosslog << "BOSS\\modlist.txt";	
		else if (revert==2) bosslog << "BOSS\\modlist.old";	
		else bosslog << "BOSS\\masterlist.txt";

		bosslog << " does not exist or can't be read!<br />" << endl 
						<< "Utility will end now.</p>" << endl
						<< "</body>"<<endl<<"</html>";
		bosslog.close();
		system("start BOSS\\BOSSlog.html");	//Displays the BOSSlog.txt.
		exit (1); //fail in screaming heap.
	} //if

	//Re-order .esp/.esm files to masterlist.txt order and output messages
	bosslog << "<div><span>Recognised And Re-ordered Mod Files</span>"<<endl<<"<p>";
	x=0;
	found=FALSE;
	while (!order.eof()) {					
		textbuf=ReadLine("order");
		if (IsValidLine(textbuf) && textbuf[0]!='\\') {		//Filter out blank lines, oblivion.esm and remark lines starting with \.
			if (!IsMessage(textbuf)) {						//Deal with mod lines only here. Message lines will be dealt with below.
				isghost = false;
				if (FileExists(textbuf+".ghost") && !FileExists(textbuf)) isghost = true;
				if (FileExists(textbuf) || isghost) {					//Tidy function not needed as file system removes trailing spaces and isn't case sensitive
					found=TRUE;
					string text = version_parse ? GetModHeader(textbuf, isghost) : textbuf;
					x++;
					modfiletime=esmtime;
					modfiletime.tm_min += x;				//files are ordered in minutes after oblivion.esp .
					int i;
					if (isghost) {
						text += " (*Ghosted*)";
						ChangeFileDate(textbuf+".ghost", modfiletime);
						if ((i = modlist.GetModIndex(textbuf+".ghost")) > -1) modlist.mods.erase(modlist.mods.begin()+i);	//Remove ordered files from modlist class.
					} else {
						ChangeFileDate(textbuf, modfiletime);
						if ((i = modlist.GetModIndex(textbuf)) > -1) modlist.mods.erase(modlist.mods.begin()+i);		//Remove ordered files from modlist class.
					}
					if (x!=1) bosslog << "</ul></li>"<<endl;
					bosslog << "<li style='list-style-type:none;'>" << text << "<ul>" << endl;		// show which mod file is being processed.
				} //if
				else found=FALSE;
			} //if
			else if (found) ShowMessage(textbuf, fcom, ooo, bc, fook2, fwe);		//Deal with message lines here.
		} //if
	} //while
	order.close();		//Close the masterlist stream, as it's not needed any more.
	bosslog <<"</p>"<<endl<<"</div><br /><br />"<<endl;

	//Find and show found mods not recognised. These are the mods that still remain in the modlist.
	//Order their dates to be +1 month after the master esm to ensure they load last.
	bosslog << "<div><span>Unrecogised Mod Files</span><p>Reorder these by hand using your favourite mod ordering utility.</p>"<<endl<<"<p>";
	for (int i=0;i<(int)modlist.mods.size();i++) {
		if (modlist.mods[i].length()>1) {
			bosslog << "Unknown mod file: " << modlist.mods[i] << "<br /><br />" << endl;
			modfiletime=esmtime;
			modfiletime.tm_mon+=1;					//shuffle all mods foward a month 
			modfiletime.tm_min=i+1;					//and order (in minutes) to original order
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