/*	Better Oblivion Sorting Software
	1.41
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    	Copyright (C) 2009  Random/Random007/jpearce
    	http://creativecommons.org/licenses/by-nc-nd/3.0/
*/

// Now supports master ESM files from Morrowind and Fallout 3. Notes regarding Oblivion apply equally to those games too.
// The B.O.S.S. project does not support the creation of Morrowind or Fallout 3 masterlists but others are free to try based on this program. 

#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utime.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <ctype.h>

#define SIZE 26 				//used in convertion of date/time struct to a string. Has to be this length.
#define MAXLENGTH 4096			//maximum length of a file name or comment. Big arbitary number.

using namespace std;

ifstream order;					//masterlist.txt - the grand mod order list
ifstream modlist;				//modlist.txt - list of esm/esp files in oblivion/data
bool FOOK2;						//true if key FOOK2 files are found.

string StringToLower (string str) {					//changes uppercase to lowercase.		
	unsigned int i;

	for(i = 0; i < str.length(); i++) str[i] = tolower(str[i]);
	return (str);
}

string TrimSpaces (string str) {					//removes trailing spaces.			
	int endpos = str.find_last_not_of(" \t");

	if (str.npos == endpos) return (""); 			//sanity check for empty string
	else return (str.substr(0, endpos+1));
}

string Tidy(string filename) {						//combines trimspaces and stringtolower to do what Windows filesystem does to filenames.					
	return (StringToLower(TrimSpaces(filename)));
}

bool FileExists(string filename) {
//file-exists check function				
	struct __stat64 fileinfo;						//variable that holds the result of _stat
	string str = Tidy(filename);

	return (_stat64(str.c_str(),&fileinfo)==0);		//will be true if stat opened successfully
}

void ChangeFileDate(string textbuf, struct tm modfiletime)  {
//changes a file's modification date
	struct __utimbuf64 ut;							//way to change time data for Windows _utime function
		int a;										//holds result of file changes

	ut.actime = _mkgmtime64(&modfiletime);			//set up ut structure for _utime64.
	ut.modtime = _mkgmtime64(&modfiletime);	
	a = _utime64(textbuf.c_str(), &ut);				//finally, change the file date.
	if (a!=0) cout << endl << "Program error - file " << textbuf << " could not have its date changed, code " << a << endl;		
}

bool IsMod(string textbuf) {
	return (((textbuf[0]!='\\') && (textbuf[0]!='*') && (textbuf[0]!='\?') && (textbuf[0]!='%')));
}

bool IsMessage(string textbuf) {
	return (((((textbuf[0]==':') || (textbuf[0]=='"') || (textbuf[0]=='%') || (textbuf[0]=='*') || (textbuf[0]=='\?')))));
}

bool IsValidLine(string textbuf) {
	return ((textbuf.length()>1) && (Tidy(textbuf)!="Oblivion.esm") && (Tidy(textbuf)!="fallout3.esm") && (Tidy(textbuf)!="Morrowind.esm"));
}

void ShowMessage(string textbuf, bool FOOK2) {
	switch (textbuf[0]) {	
		case '*':
			if (FOOK2) cout << "  !!! FOOK2 INSTALLATION ERROR: " << textbuf.substr(1) << endl;
		break;
		case '%':
			cout << "  . Bashed Patch tag suggestion: " << textbuf.substr(1) << endl;
		break;
		case '\?':
			cout << "  . Note: " << textbuf.substr(1) << endl;
		break;
		case ':' :
			cout << "  . Requires: " << textbuf.substr(1) << endl;
		break;
		case '"' :
			cout << "  . Conflicts with: " << textbuf.substr(1) << endl;
		break;
	} //switch
}

string ReadLine (string file) {						//Read a line from a file. Could be rewritten better.
	char cbuffer[MAXLENGTH];						//character buffer.
	string textbuf;

	if (file=="order") order.getline(cbuffer,MAXLENGTH);				//get a line of text from the masterlist.txt text file
	if (file=="modlist") modlist.getline(cbuffer,MAXLENGTH);			//get a line of text from the modlist.txt text file
	//No internal error handling here.
	textbuf=cbuffer;
	if (file=="order") {		//If parsing masterlist.txt, parse only lines that start with > or < depending on FOOK2 installation. Allows both FOOK2 and nonFOOK2 differentiaton.
		if ((textbuf[0]=='>') && (FOOK2)) textbuf.erase(0,1);
		if ((textbuf[0]=='>') && (!FOOK2)) textbuf='\\';
		if ((textbuf[0]=='<') && (!FOOK2)) textbuf.erase(0,1);
		if ((textbuf[0]=='<') && (FOOK2)) textbuf='\\';
	} //if
	return (textbuf);
}

int main() {					
	
	int x;							//random useful integers
	string textbuf,textbuf2;		//a line of text from a file (should usually end up being be a file name); 			
	struct __stat64 buf;			//temp buffer of info for _stat function
	struct tm esmtime;			    //the modification date/time of the main .esm file
	struct tm modfiletime;			//useful variable to store a file's date/time
	bool found;						
	string modfilestring;			//used to convert stuff.
	char modfilechar [SIZE];		//used to convert stuff.

	cout << endl << endl << "-----------------------------------------------------------" << endl;
	cout <<                 " Better Oblivion Sorting Software       Load Order Utility " << endl << endl;
	cout <<                 "   (c) Random007 & the BOSS development team, 2009         " << endl;
	cout <<                 "   Some rights reserved.                                   " << endl;
	cout <<                 "   CC Attribution-Noncommercial-No Derivative Works 3.0    " << endl;
	cout <<                 "   http://creativecommons.org/licenses/by-nc-nd/3.0/       " << endl;
	cout <<                 " v1.41 (31 August 09)                                       " << endl;
	cout <<                 "-----------------------------------------------------------" << endl << endl;

	//open masterlist.txt
	order.open("masterlist.txt");	
	if (order.fail()) {							
		cout << endl << "Critical Error! masterlist.TXT does not exist or can't be read!" << endl; 
		cout <<         "! Uitlity will end now." << endl;
		exit (1); //fail in screaming heap.
	} //if

	//get date for oblivion.esm.
	if (FileExists("oblivion.esm")) _stat64("oblivion.esm", &buf);
	else if (FileExists("morrowind.esm")) _stat64("morrowind.esm", &buf);
	else if (FileExists("fallout3.esm")) _stat64("fallout3.esm", &buf);
	else {				
		cout << endl << "Critical Error: Master .ESM file not found (or not accessable)!" << endl;
		cout <<         "Make sure you're running this in your Data folder." <<endl;
		cout <<         "! Utility will end now." << endl;
		exit (1); //fail in screaming heap.
	} //else

	_gmtime64_s(&esmtime, &buf.st_mtime);		//convert _stat64 modification date data to date/time struct.

	//Display oblivion.esm's modification date (mostly for debugging)
	_ctime64_s (modfilechar, SIZE, &buf.st_mtime);	//convert date/time to printable string for output.
	modfilestring=modfilechar;
	cout << "Master .ESM date: " << modfilestring;

	//Check if FOOK2 or not
	if (FOOK2=FileExists("FOOK2 - Main.esm")) cout << "FOOK2 Detected" << endl << endl;
		else cout << "FOOK2 not detected." << endl << endl;
	if (FileExists("FOOK2 - Main.esp") && !FOOK2) cout << "WARNING: FOOK2.esm seems to be missing." << endl << endl;
	
	//Generate list of all .esp or .esm files.
	//also, clear file attributes.
	system ("attrib -R -H -S *.*");		//clear any read only attriutes from oblivion/data.
	if (FileExists ("modlist.txt")) {	//add an additional undo level just in case.
		system ("del modlist.old");
		system ("ren modlist.txt modlist.old");
	} //if
	system ("dir *.es? /a:-d /b /o:d /t:w > modlist.txt"); // quick way to list the mod files: pipe them into a text file.

	//Open modlist.txt file and verify success																
	modlist.open("modlist.txt");			
	if (modlist.fail()) {
		cout << endl << "Critical Error! Internal program error! modlist.txt should have been created but it wasn't." << endl;
		cout <<         "Make sure you are running as Administrator if using Windows Vista." << endl;
		cout <<         "! Utility will end now." << endl;
		exit(1); //fail in screaming heap.
	} //if

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
	cout <<   endl << "------------------------------------" << endl;
	cout <<           "Recognised and re-ordered mod files:" << endl;
	cout <<           "------------------------------------" << endl;
	x=0;
	found=FALSE;
	while (!order.eof()) {					
		textbuf=ReadLine("order");
		if (IsValidLine(textbuf) && textbuf[0]!='\\') {		//Filter out blank lines, oblivion.esm and remark lines starting with \.
			if (!IsMessage(textbuf)) {						//Deal with mod lines only here. Message lines will be dealt with below.
				if (FileExists(textbuf)) {					//Tidy function not needed as file system removes trailing spaces and isn't case sensitive
					found=TRUE;
					cout << endl << textbuf << endl;		// show which mod file is being processed.
					x++;
					modfiletime=esmtime;
					modfiletime.tm_min += x;				//files are ordered in minutes after oblivion.esp .
					ChangeFileDate(textbuf, modfiletime);
				} //if
				else found=FALSE;
			} //if
			else if (found) ShowMessage(textbuf, FOOK2);		//Deal with message lines here.
		} //if
	} //while

	//Find and show found mods not recognised. Parse each file in modlist.txt and try finding it in masterlist.txt. If not found, unknown.
	cout <<   endl << "-----------------------------------------------------------------" << endl;
	cout <<           "Unrecognised mod files:                                          " << endl;
	cout <<           "Reorder these by hand using your favourite mod ordering utility. " << endl;
	cout <<           "-----------------------------------------------------------------" << endl << endl;
	modlist.clear();						//reset position in modlist.txt to start.
	modlist.seekg (0, ios.beg);				// "
	while (!modlist.eof()) {	
		textbuf=ReadLine("modlist");
		found=FALSE;
		order.clear ();						//reset position in masterlist.txt to start.
		order.seekg (0, ios.beg);			// "
		while (!order.eof() && !found) {	//repeat until end of masterlist.txt or file found.				
			textbuf2=ReadLine("order");
			if (IsMod(textbuf2)) if (Tidy(textbuf)==Tidy(textbuf2)) found=TRUE;		//filter out comment, blank and message lines when checking for match - speeds process up.
		} // while
		if (!found) cout << "Unknown mod file: " << textbuf << endl;
	} //while

	//Let people know the program has stopped.
	cout <<   endl << endl << "-----------------------------------------------------------" << endl;
	cout << "Done.";
	return (0);
}

