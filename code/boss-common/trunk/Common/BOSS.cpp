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
#include "boost/filesystem.hpp"

#include "BOSS.h"

#define MAXLENGTH 4096			//maximum length of a file name or comment. Big arbitrary number.

namespace fs = boost::filesystem;

using namespace boss;

//BOSS [--update | -u] [--help | -h] [--version-check | -V]
int main(int argc, char *argv[]) {					
	
	int x;							//random useful integers
	string textbuf,textbuf2;		//a line of text from a file (should usually end up being be a file name);
	time_t esmtime,modfiletime;		//File modification times.
	bool found;
	bool update = false;			//To update masterlist or not?
	bool version_parse = true;		//Enable parsing of mod's headers to look for version strings
	int game;						//What game's mods are we sorting? 1 = Oblivion, 2 = Fallout 3, 3 = Morrowind.
	bool isghost;					//Is the file ghosted or not?

	//Parse command line arguments.
	if (argc > 1) {
		for (int i=0; i < argc; i++) {
			if (strcmp("--update", argv[i]) == 0 || strcmp("-u", argv[i]) == 0) {
				update = true;
			} else if (strcmp("--disable-version-parse", argv[i]) == 0 || strcmp("-V-", argv[i]) == 0) {
				version_parse = false;
			} else if (strcmp("--help", argv[i]) == 0 || strcmp("-h", argv[i]) == 0) {
				cout << "Better Oblivion Sorting Software is a utility that sorts the load order of TESIV: Oblivion, TESIII: Morrowind and Fallout 3 mods according to their relative positions on a frequently-updated masterlist ";
				cout << "to ensure proper load order and minimise incompatibilities between mods." << endl << endl;
				cout << "Optional Parameters" << endl << endl;
				cout << "-u, --update: " << endl << "    Automatically updates the local copy of the masterlist using the latest version available on the Google Code repository." << endl << endl;
				cout << "-V-, --disable-version-parse: " << endl << "    Enables the parsing of each mod's description and if found extracts from there the author stamped mod's version and prints it along other data in the generated bosslog.txt." << endl << endl;
				exit (0);
			}
		}
	}

	//Try to create BOSS sub-directory.
	try { fs::create_directory("BOSS\\");
	} catch(fs::filesystem_error e) {
		cout << "Critical Error: Sub-directory \"Data\\BOSS\\\" could not be created!" << endl
			 << "Check your permissions and make sure you have write access to your Data folder." << endl
			 << "! Utility will end now." << endl << endl;
		cout << "Press ENTER to quit...";
		cin.ignore(1,'\n');
		exit(1); //fail in screaming heap.
	}

	//Check for creation of BOSSlog.txt.
	bosslog.open("BOSS\\BOSSlog.html");
	if (bosslog.fail()) {							
		cout << endl << "Critical Error: BOSSlog.html could not be written to!" << endl
					 << "Make sure you are running as Administrator if using Windows Vista or Windows 7." << endl
					 << "! Utility will end now." << endl;
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

	if (fs::exists("Oblivion.esm")) game = 1;
	else if (fs::exists("Fallout3.esm")) game = 2;
	else if (fs::exists("Morrowind.esm")) game = 3;
	else {
		bosslog << endl << "<p class='error'>Critical Error: Master .ESM file not found!<br />" << endl
						<< "Make sure you're running this in your Data folder.<br />" << endl
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
		else cout << "Error: Masterlist update failed." << endl;
	}

	cout << endl << "Better Oblivion Sorting Software working..." << endl;

	bosslog << "<div><span>Special Mod Detection</span>"<<endl<<"<p>";
	if (game == 1) {
		//Check if FCOM or not
		if (fcom=fs::exists("FCOM_Convergence.esm")) bosslog << "FCOM detected.<br />" << endl;
			else bosslog << "FCOM not detected.<br />" << endl;
		if (fs::exists("FCOM_Convergence.esp") && !fcom) bosslog << "WARNING: FCOM_Convergence.esm seems to be missing.<br />" << endl;
		//Check if OOO or not
		if (ooo=fs::exists("Oscuro's_Oblivion_Overhaul.esm")) bosslog << "OOO detected.<br />" << endl;
			else bosslog << "OOO not detected.<br />" << endl;
		//Check if Better Cities or not
		if (bc=fs::exists("Better Cities Resources.esm")) bosslog << "Better Cities detected.<br />" << endl;
			else bosslog << "Better Cities not detected.<br />" << endl;
	} else if (game == 2) {
		//Check if fook2 or not
		if (fook2=fs::exists("FOOK2 - Main.esm")) bosslog << "FOOK2 Detected.<br />" << endl;
			else bosslog << "FOOK2 not detected.<br />" << endl;
		if (fs::exists("FOOK2 - Main.esp") && !fook2) bosslog << "WARNING: FOOK2.esm seems to be missing.<br />" << endl;
		//Check if fwe or not
		if (fwe=fs::exists("FO3 Wanderers Edition - Main File.esm")) bosslog << "FWE detected.<br />" << endl;
			else bosslog << "FWE not detected.<br />" << endl;
	}
	bosslog <<"</p>"<<endl<<"</div><br /><br />"<<endl;

	//open masterlist.txt
	order.open("BOSS\\masterlist.txt");	
	if (order.fail()) {							
		bosslog << endl << "<p class='error'>Critical Error: masterlist.txt cannot be read!<br />" << endl 
						<< "! Utility will end now.</p>" << endl
						<< "</body>"<<endl<<"</html>";
		bosslog.close();
		system ("start BOSS\\BOSSlog.html");	//Displays the BOSSlog.txt.
		exit (1); //fail in screaming heap.
	} //if

	//Generate list of all .esp or .esm files.
	if (fs::exists ("BOSS\\modlist.txt")) {	//add an additional undo level just in case.
		if (fs::exists ("BOSS\\modlist.old")) {
			system ("attrib -r BOSS\\modlist.old");	//Clears read only attribute of modlist.old if present, so we can delete the file.
			system ("del BOSS\\modlist.old");
		}
		system ("attrib -r BOSS\\modlist.txt");	//Clears read only attribute of modlist.txt if present, so we can rename the file.
		system ("ren BOSS\\modlist.txt modlist.old");
	} //if
	system ("dir *.es? /a:-d /b /o:d /t:w > BOSS\\modlist.txt"); // quick way to list the mod files: pipe them into a text file.

	//Open modlist.txt file and verify success																
	modlist.open("BOSS\\modlist.txt");
	if (modlist.fail()) {
		bosslog << endl << "<p class='error'>Critical Error: modlist.txt could not be written to!<br />" << endl
						<< "Make sure you are running as Administrator if using Windows Vista or Windows 7.<br />" << endl
						<< "! Utility will end now.</p>" << endl
						<< "</body>"<<endl<<"</html>";
		bosslog.close();
		system ("start BOSS\\BOSSlog.html");	//Displays the BOSSlog.txt.
		exit(1); //fail in screaming heap.
	} //if

	//Remove any read only attributes from esm/esp files if present.
	//This way pops up an error message in console if there are no ghosted files. Is there a better way?
	system("attrib -r *.es?");
	system("attrib -r *.ghost");

	//Change mod date of each file in the list to oblivion.esm date _plus_ 1 year. Ensures unknown mods go last in original order.
	x=0;
	while (!modlist.eof()) {	
		textbuf=ReadLine("modlist");
		if (IsValidLine(textbuf)) {
			x++;				
			modfiletime=esmtime;
			modfiletime += x*60; //time_t is an integer number of seconds, so adding 60 on increases it by a minute.
			modfiletime += x*86400; //time_t is an integer number of seconds, so adding 86,400 on increases it by a day.
			try { fs::last_write_time(textbuf,modfiletime);
			} catch(fs::filesystem_error e) {
				bosslog << "<br /><span class='error'>Could not change the date of file " << textbuf << ", make sure you have write access to it.</span>" << endl;
			}
		} //if
	} //while

	//Re-order .esp/.esm files to masterlist.txt order	and output messages
	//Note: \, *, % and ? were chosen as parse switches because they are not valid file name characters and can't appear in an ESP or ESM file name
	bosslog << "<div><span>Recognised And Re-ordered Mod Files</span>"<<endl<<"<p>";
	//This next line is only needed until the modlist class gets implemented in trunk.
	bosslog << "<ul>" << endl;
	x=0;
	found=false;
	while (!order.eof()) {					
		textbuf=ReadLine("order");
		if (IsValidLine(textbuf) && textbuf[0]!='\\') {		//Filter out blank lines, oblivion.esm and remark lines starting with \.
			if (!IsMessage(textbuf)) {						//Deal with mod lines only here. Message lines will be dealt with below.
				isghost = false;
				if (fs::exists(textbuf+".ghost") && !fs::exists(textbuf)) isghost = true;
				if (fs::exists(textbuf) || isghost) {					//Tidy function not needed as file system removes trailing spaces and isn't case sensitive
					found=true;
					string text = version_parse ? GetModHeader(textbuf, isghost) : textbuf;
					modfiletime=esmtime;
					modfiletime += x*60; //time_t is an integer number of seconds, so adding 60 on increases it by a minute.
					x++;
					if (isghost) {
						text += " (*Ghosted*)";
						fs::last_write_time(textbuf+".ghost",modfiletime);
						try { fs::last_write_time(textbuf+".ghost",modfiletime);
						} catch(fs::filesystem_error e) {
							bosslog << "<br /><span class='error'>Could not change the date of file " << textbuf+".ghost" << ", make sure you have write access to it.</span>" << endl;
						}
					} else {
						try { fs::last_write_time(textbuf,modfiletime);
						} catch(fs::filesystem_error e) {
							bosslog << "<br /><span class='error'>Could not change the date of file " << textbuf<< ", make sure you have write access to it.</span>" << endl;
						}
					}
					bosslog << "</ul>" << endl << text << endl << "<ul>" << endl;		// show which mod file is being processed.
				} //if
				else found=false;
			} //if
			else if (found) ShowMessage(textbuf, fcom, ooo, bc, fook2, fwe);		//Deal with message lines here.
		} //if
	} //while
	//Again, this line is only needed until the modlist class is implemented.
	bosslog << "<ul>" << endl;
	bosslog <<"</p>"<<endl<<"</div><br /><br />"<<endl;

	//Find and show found mods not recognised. Parse each file in modlist.txt and try finding it in masterlist.txt. If not found, unknown.
	bosslog << "<div><span>Unrecogised Mod Files</span><p>Reorder these by hand using your favourite mod ordering utility.</p>"<<endl<<"<p>";
	modlist.clear();						//reset position in modlist.txt to start.
	modlist.seekg (0, order.beg);			// "
	while (!modlist.eof()) {	
		textbuf=ReadLine("modlist");
		if (IsValidLine(textbuf) && textbuf[0]!='\\')  { //Filter out blank lines, oblivion.esm and remark lines starting with \.
			found=false;
			order.clear ();						//reset position in masterlist.txt to start.
			order.seekg (0, order.beg);			// "
			while (!order.eof() && !found) {	//repeat until end of masterlist.txt or file found.				
				textbuf2=ReadLine("order");	
				if (IsMod(textbuf2))			//filter out comment, blank and message lines when checking for match - speeds process up.
					if (Tidy(textbuf)==Tidy(textbuf2))
						found=true;
			} // while
			if (!found && textbuf.length()>1) bosslog << "Unknown mod file: " << textbuf << "<br /><br />" << endl;
		}
	} //while
	bosslog <<"</p>"<<endl<<"</div><br /><br />"<<endl;

	//Let people know the program has stopped.
	bosslog <<"<div><span>Done.</span></div><br /><br />"<<endl<<"</body>"<<endl<<"</html>";
	bosslog.close();
	system ("start BOSS\\BOSSlog.html");	//Displays the BOSSlog.txt.
	return (0);
}