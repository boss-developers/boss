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

#define SIZE 26 				//used in conversion of date/time struct to a string. Has to be this length.
#define MAXLENGTH 4096			//maximum length of a file name or comment. Big arbitrary number.

using namespace boss;

//BOSS [--update | -u] [--help | -h] [--version-check | -V]
int main(int argc, char *argv[]) {					
	
	int x;							//random useful integers
	string textbuf,textbuf2;		//a line of text from a file (should usually end up being be a file name); 			
	struct __stat64 buf;			//temp buffer of info for _stat function
	struct tm esmtime;			    //the modification date/time of the main .esm file
	struct tm modfiletime;			//useful variable to store a file's date/time
	bool found;						
	char modfilechar [SIZE];		//used to convert stuff.
	bool update = false;			//To update masterlist or not?
	bool version_parse = false;		//Enable parsing of mod's headers to look for version strings
	int game;						//What game's mods are we sorting? 1 = Oblivion, 2 = Fallout 3, 3 = Morrowind.
	bool isghost;					//Is the file ghosted or not?

	//Parse command line arguments.
	if (argc > 1) {
		for (int i=0; i < argc; i++) {
			if (strcmp("--update", argv[i]) == 0 || strcmp("-u", argv[i]) == 0) {
				update = true;
			} else if (strcmp("--version-check", argv[i]) == 0 || strcmp("-V", argv[i]) == 0) {
				version_parse = true;
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

	if (FileExists("oblivion.esm")) game = 1;
	else if (FileExists("fallout3.esm")) game = 2;
	else if (FileExists("morrowind.esm")) game = 3;

	CreateDirectory("BOSS\\",NULL);

	if (update == true) {
		cout << endl << "Updating to the latest masterlist from the Google Code repository..." << endl;
		int rev = UpdateMasterlist(game);
		if (rev > 0) cout << "masterlist.txt updated to revision " << rev << endl;
		else cout << "Masterlist update failed." << endl;
	}

	cout << endl << "Better Oblivion Sorting Software working..." << endl;
	
	//Check for creation of BOSSlog.txt.
	bosslog.open("BOSS\\BOSSlog.txt");
	if (bosslog.fail()) {							
		cout << endl << "Critical Error! BOSSlog.txt should have been created but it wasn't." << endl
					 << "Make sure you are running as Administrator if using Windows Vista or Windows 7." << endl
					 << "! Utility will end now." << endl;
		exit (1); //fail in screaming heap.
	}

	bosslog << endl << endl << "-----------------------------------------------------------" << endl
							<< " Better Oblivion Sorting Software       Load Order Utility " << endl << endl
							<< "   (c) Random007 & the BOSS development team, 2009-2010    " << endl
							<< "   Some rights reserved.                                   " << endl
							<< "   CC Attribution-Noncommercial-No Derivative Works 3.0    " << endl
							<< "   http://creativecommons.org/licenses/by-nc-nd/3.0/       " << endl
							<< "   v1.6 (1 August 2010)									   " << endl
							<< "-----------------------------------------------------------" << endl << endl;

	//open masterlist.txt
	order.open("BOSS\\masterlist.txt");	
	if (order.fail()) {							
		bosslog << endl << "Critical Error! masterlist.txt does not exist or can't be read!" << endl
						<< "! Utility will end now." << endl;
		bosslog.close();
		system ("start BOSS\\BOSSlog.txt");	//Displays the BOSSlog.txt.
		exit (1); //fail in screaming heap.
	} //if

	//get date for oblivion.esm.
	if (game == 1) _stat64("oblivion.esm", &buf);
	else if (game == 2) _stat64("fallout3.esm", &buf);
	else if (game == 3) _stat64("morrowind.esm", &buf);
	else {
		bosslog << endl << "Critical Error: Master .ESM file not found (or not accessible)!" << endl
						<< "Make sure you're running this in your Data folder." << endl
						<< "! Utility will end now." << endl;
		bosslog.close();
		system ("start BOSS\\BOSSlog.txt");	//Displays the BOSSlog.txt.
		exit (1); //fail in screaming heap.
	} //else

	_gmtime64_s(&esmtime, &buf.st_mtime);		//convert _stat64 modification date data to date/time struct.

	//Display oblivion.esm's modification date (mostly for debugging)
	_ctime64_s (modfilechar, SIZE, &buf.st_mtime);	//convert date/time to printable string for output.
	bosslog << "Master .ESM date: " << (string)modfilechar;

	if (game == 1) {
		//Check if FCOM or not
		if (fcom=FileExists("FCOM_Convergence.esm")) bosslog << "FCOM detected." << endl;
			else bosslog << "FCOM not detected." << endl;
		if (FileExists("FCOM_Convergence.esp") && !fcom) bosslog << "WARNING: FCOM_Convergence.esm seems to be missing." << endl;
		//Check if OOO or not
		if (ooo=FileExists("Oscuro's_Oblivion_Overhaul.esm")) bosslog << "OOO detected." << endl;
			else bosslog << "OOO not detected." << endl;
		//Check if Better Cities or not
		if (bc=FileExists("Better Cities Resources.esm")) bosslog << "Better Cities detected." << endl;
			else bosslog << "Better Cities not detected." << endl;
	} else if (game == 2) {
		//Check if fook2 or not
		if (fook2=FileExists("FOOK2 - Main.esm")) bosslog << "FOOK2 Detected" << endl;
			else bosslog << "FOOK2 not detected." << endl;
		if (FileExists("FOOK2 - Main.esp") && !fook2) bosslog << "WARNING: FOOK2.esm seems to be missing." << endl;
		//Check if fwe or not
		if (fwe=FileExists("FO3 Wanderers Edition - Main File.esm")) bosslog << "FWE detected." << endl;
			else bosslog << "FWE not detected." << endl;
	}
	bosslog << endl;

	//Generate list of all .esp or .esm files.
	if (FileExists ("BOSS\\modlist.txt")) {	//add an additional undo level just in case.
		if (FileExists ("BOSS\\modlist.old")) {
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
		bosslog << endl << "Critical Error! Internal program error! modlist.txt should have been created but it wasn't." << endl
						<< "Make sure you are running as Administrator if using Windows Vista." << endl
						<< "! Utility will end now." << endl;
		bosslog.close();
		system ("start BOSS\\BOSSlog.txt");	//Displays the BOSSlog.txt.
		exit(1); //fail in screaming heap.
	} //if

	//Remove any read only attributes from esm/esp files if present.
	system("attrib -r *.es?");

	//Change mod date of each file in the list to oblivion.esm date _plus_ 1 year. Ensures unknown mods go last in original order.
	x=0;
	while (!modlist.eof()) {	
		textbuf=ReadLine("modlist");
		if (IsValidLine(textbuf)) {
			x++;				
			modfiletime=esmtime;
			modfiletime.tm_mon+=1;					//shuffle all mods foward a month 
			modfiletime.tm_min=x;					//and order (in minutes) to original order
			ChangeFileDate(textbuf, modfiletime);
		} //if
	} //while

	//Re-order .esp/.esm files to masterlist.txt order	and output messages
	//Note: \, *, % and ? were chosen as parse switches because they are not valid file name characters and can't appear in an ESP or ESM file name
	bosslog << endl << "------------------------------------" << endl
					<< "Recognised and re-ordered mod files:" << endl
					<< "------------------------------------" << endl;
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

					if (isghost) {
						text += " (*Ghosted*)";
						ChangeFileDate(textbuf+".ghost", modfiletime);
					} else ChangeFileDate(textbuf, modfiletime);

					bosslog << endl << text << endl;		// show which mod file is being processed.
				} //if
				else found=FALSE;
			} //if
			else if (found) ShowMessage(textbuf, fcom, ooo, bc, fook2, fwe);		//Deal with message lines here.
		} //if
	} //while

	//Find and show found mods not recognised. Parse each file in modlist.txt and try finding it in masterlist.txt. If not found, unknown.
	bosslog << endl << "-----------------------------------------------------------------" << endl
					<< "Unrecognised mod files:                                          " << endl
					<< "Reorder these by hand using your favourite mod ordering utility. " << endl
					<< "-----------------------------------------------------------------" << endl << endl;
	modlist.clear();						//reset position in modlist.txt to start.
	modlist.seekg (0, order.beg);				// "
	while (!modlist.eof()) {	
		textbuf=ReadLine("modlist");
		found=FALSE;
		order.clear ();						//reset position in masterlist.txt to start.
		order.seekg (0, order.beg);			// "
		while (!order.eof() && !found) {	//repeat until end of masterlist.txt or file found.				
			textbuf2=ReadLine("order");
			if (IsValidLine(textbuf) && textbuf[0]!='\\') {		//Filter out blank lines, oblivion.esm and remark lines starting with \.
				if (IsMod(textbuf2)) if (Tidy(textbuf)==Tidy(textbuf2)) found=TRUE;		//filter out comment, blank and message lines when checking for match - speeds process up.
			}
		} // while
		if (!found && textbuf.length()>1) bosslog << "Unknown mod file: " << textbuf << endl;
	} //while

	//Let people know the program has stopped.
	bosslog << endl << endl << "-----------------------------------------------------------" << endl;
	bosslog << "Done.";
	bosslog.close();
	system ("start BOSS\\BOSSlog.txt");	//Displays the BOSSlog.txt.
	return (0);
}