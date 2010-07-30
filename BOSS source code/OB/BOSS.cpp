/*	Better Oblivion Sorting Software
	2.1
	Quick and Dirty Load Order Utility for Oblivion, Fallout 3 and Morrowind
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/
*/

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
#include <direct.h>
#include <sstream>
#define CURL_STATICLIB			//Tells the compiler to use curl as a static library.
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

#define SIZE 26 				//used in convertion of date/time struct to a string. Has to be this length.
#define MAXLENGTH 4096			//maximum length of a file name or comment. Big arbitary number.

using namespace std;

ifstream order;						//masterlist.txt - the grand mod order list
ifstream modlist;					//modlist.txt - list of esm/esp files in oblivion/data
ofstream bosslog;					//BOSSlog.txt - output file.
bool fcom;							//true if key FCOM files are found.
bool ooo;                      	 	//true if OOO esm is found.
bool bc;                        	//true if Better Cities esm is found.
bool fook2;							//true if key FOOK2 files are found.
bool fwe;							//true if FWE esm is found

string Tidy(string filename) {						//Changes uppercase to lowercase and removes trailing spaces to do what Windows filesystem does to filenames.	
	int endpos = filename.find_last_not_of(" \t");
	
	if (filename.npos == endpos) return (""); 			//sanity check for empty string
	else {
		filename = filename.substr(0, endpos+1);
		for (unsigned int i = 0; i < filename.length(); i++) filename[i] = tolower(filename[i]);
		return (filename);
	}
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
	if (a!=0) bosslog << endl << "Program error - file " << textbuf << " could not have its date changed, code " << a << endl;		
}

bool IsMod(string textbuf) {
	return (((textbuf[0]!='\\') && (textbuf[0]!='*') && (textbuf[0]!='\?') && (textbuf[0]!='%') && (textbuf[0]!=':') && (textbuf[0]!='$') &&(textbuf[0]!='^') && (textbuf[0]!='"')));
}

bool IsMessage(string textbuf) {
	return (((textbuf[0]=='\?') || (textbuf[0]=='*') || (textbuf[0]=='%') || (textbuf[0]==':') || (textbuf[0]=='$') || (textbuf[0]=='^') || (textbuf[0]=='"')));
}

bool IsValidLine(string textbuf) {
	return ((textbuf.length()>1) && (Tidy(textbuf)!="Oblivion.esm") && (Tidy(textbuf)!="fallout3.esm") && (Tidy(textbuf)!="Morrowind.esm"));
}

void ShowMessage(string textbuf, bool fcom, bool ooo, bool bc, bool fook2, bool fwe) {
	switch (textbuf[0]) {	
		case '*':
			if (fcom) bosslog << "  !!! FCOM INSTALLATION ERROR: " << textbuf.substr(1) << endl;
			else if (fook2) bosslog << "  !!! FOOK2 INSTALLATION ERROR: " << textbuf.substr(1) << endl;
		break;
		case ':':
			bosslog << " . Requires: " << textbuf.substr(1) << endl;
		break;
		case '$':
			if (ooo) bosslog << " . OOO Specific Note: " << textbuf.substr(1) << endl;
			else if (fwe) bosslog << "  . FWE Specific Note: " << textbuf.substr(1) << endl;
		break;
		case '%':
			bosslog << "  . Bashed Patch tag suggestion: " << textbuf.substr(1) << endl;
		break;
		case '\?':
			bosslog << "  . Note: " << textbuf.substr(1) << endl;
		break;
		case '"':
			bosslog << "  . Incompatible with: " << textbuf.substr(1) << endl;
		break;
		case '^':
			bosslog << "  . Better Cities Specific Note: " << textbuf.substr(1) <<endl;
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
	if (file=="order") {		//If parsing masterlist.txt, parse only lines that start with > or < depending on FCOM installation. Allows both FCOM and nonFCOM differentiaton.
		if ((textbuf[0]=='>') && (fcom)) textbuf.erase(0,1);
		if ((textbuf[0]=='>') && (!fcom)) textbuf='\\';
		if ((textbuf[0]=='<') && (!fcom)) textbuf.erase(0,1);
		if ((textbuf[0]=='<') && (fcom)) textbuf='\\';
	} //if
	return (textbuf);
}

int writer(char *data, size_t size, size_t nmemb, string *buffer){
	int result = 0;
	if(buffer != NULL) {
		buffer -> append(data, size * nmemb);
		result = size * nmemb;
	}
	return result;
} 

int UpdateMasterlist(int game) {
	char *url;					//Masterlist file url
	CURL *curl;					//Some cURL resource...
	string buffer,revision,oldline,newline;		//A bunch of strings.
	int start,end;				//Position holders for trimming strings.
	ifstream in;				//Input stream.
	ofstream out;				//Output stream.
	char cbuffer[MAXLENGTH];	//Another buffer for holding lines to be processed.

	//Which masterlist to get?
	if (game == 1) url = "http://better-oblivion-sorting-software.googlecode.com/svn/masterlist.txt";
	else if (game == 2) url = "http://better-oblivion-sorting-software.googlecode.com/svn/FO3Masterlist/masterlist.txt";
	else if (game == 3) url = "http://better-oblivion-sorting-software.googlecode.com/svn/MWmasterlist/masterlist.txt";

	//Get SVN masterlist file.
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
		out.open("masterlist.tmp");
		out << buffer;
		out.close();
    }

	//Get HEAD revision number from http://better-oblivion-sorting-software.googlecode.com/svn/ page text.
	curl = curl_easy_init();
	if (curl){
		curl_easy_setopt(curl, CURLOPT_URL, "http://better-oblivion-sorting-software.googlecode.com/svn/");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);	
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer );
		curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}
	start = buffer.find("Revision ");
	end = buffer.find(": /");
	end = end - (start+9);
	revision = buffer.substr(start+9,end);
	stringstream ss(revision);
	ss >> end;

	//Add revision number to masterlist and fix the line breaks.
	oldline = "? Masterlist Information: $Revision$, $Date$, $LastChangedBy$";
	newline = "? Masterlist Revision: "+revision;
	in.open("masterlist.tmp");
	out.open("masterlist.txt");
	while (!in.eof()) {	
		in.getline(cbuffer,MAXLENGTH);
		buffer = (string)cbuffer;
		if (buffer.length()>0) {
			int pos = buffer.find(oldline);
			if (pos > -1) {
				out << newline << endl;
			} else {
				pos = buffer.find("\r");
				buffer.replace(pos,2,"\n");
				out << buffer;
			}
		}
	}
	in.close();
	out.close();
	//Remove temporary masterlist file.
	system ("del masterlist.tmp");
	//Return revision number.
	return end;
}

//BOSS [--update | -u] [--help | -h]
int main(int argc, char *argv[]) {					
	
	int x;							//random useful integers
	string textbuf,textbuf2;		//a line of text from a file (should usually end up being be a file name); 			
	struct __stat64 buf;			//temp buffer of info for _stat function
	struct tm esmtime;			    //the modification date/time of the main .esm file
	struct tm modfiletime;			//useful variable to store a file's date/time
	bool found;						
	char modfilechar [SIZE];		//used to convert stuff.

	bool update = false;			//To update masterlist or not?
	int game;						//What game's mods are we sorting? 1 = Oblivion, 2 = Fallout 3, 3 = Morrowind.

	//Parse command line arguments.
	if (argc > 1) {
		for (int i=0; i < argc; i++) {
			if (strcmp("--update", argv[i]) == 0 || strcmp("-u", argv[i]) == 0) {
				update = true;
			}else if (strcmp("--help", argv[i]) == 0 || strcmp("-h", argv[i]) == 0) {
				cout << endl << "BOSS [--update | -u] [--help | -h]" << endl << endl;
				cout << "Better Oblivion Sorting Software is a utility that sorts the load order of TESIV: Oblivion, TESIII: Morrowind and Fallout 3 mods according to their relative positions on a frequently-updated masterlist ";
				cout << "to ensure proper load order and minimise incompatibilities between mods." << endl << endl;
				cout << "Optional Parameters" << endl << endl;
				cout << "-u" << endl << "--update" << endl << "Automatically updates the local copy of the masterlist using the latest version available on the Google Code repository." << endl << endl;
				exit (0);
			}
		}
	}

	if (FileExists("oblivion.esm")) game = 1;
	else if (FileExists("fallout3.esm")) game = 2;
	else if (FileExists("morrowind.esm")) game = 3;

	if (update == true) {
		cout << endl << "Updating to the latest masterlist from the Google Code repository..." << endl;
		int rev = UpdateMasterlist(game);
		if (rev > 0) cout << "masterlist.txt updated to revision " << rev << endl;
		else cout << "Masterlist update failed." << endl;
	}

	cout << endl << "Better Oblivion Sorting Software working..." << endl;
	
	//Check for creation of BOSSlog.txt.
	bosslog.open("BOSSlog.txt");
	if (bosslog.fail()) {							
		cout << endl << "Critical Error! BOSSlog.txt should have been created but it wasn't." << endl;
		cout << 		"Make sure you are running as Administrator if using Windows Vista or Windows 7." << endl;
		cout <<    		"! Utility will end now." << endl;
		exit (1); //fail in screaming heap.
	}

	bosslog << endl << endl << "-----------------------------------------------------------" << endl;
	bosslog <<                 " Better Oblivion Sorting Software       Load Order Utility " << endl << endl;
	bosslog <<                 "   (c) Random007 & the BOSS development team, 2009-2010    " << endl;
	bosslog <<                 "   Some rights reserved.                                   " << endl;
	bosslog <<                 "   CC Attribution-Noncommercial-No Derivative Works 3.0    " << endl;
	bosslog <<                 "   http://creativecommons.org/licenses/by-nc-nd/3.0/       " << endl;
	bosslog <<                 "   v2.1 (30 July 2010)									   " << endl;
	bosslog <<                 "-----------------------------------------------------------" << endl << endl;

	//open masterlist.txt
	order.open("masterlist.txt");	
	if (order.fail()) {							
		bosslog << endl << "Critical Error! masterlist.txt does not exist or can't be read!" << endl; 
		bosslog <<         "! Utility will end now." << endl;
		bosslog.close();
		system ("start BOSSlog.txt");	//Displays the BOSSlog.txt.
		exit (1); //fail in screaming heap.
	} //if

	//get date for oblivion.esm.
	if (game == 1) _stat64("oblivion.esm", &buf);
	else if (game == 2) _stat64("fallout3.esm", &buf);
	else if (game == 3) _stat64("morrowind.esm", &buf);
	else {
		bosslog << endl << "Critical Error: Master .ESM file not found (or not accessible)!" << endl;
		bosslog <<         "Make sure you're running this in your Data folder." <<endl;
		bosslog <<         "! Utility will end now." << endl;
		bosslog.close();
		system ("start BOSSlog.txt");	//Displays the BOSSlog.txt.
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
	if (FileExists ("modlist.txt")) {	//add an additional undo level just in case.
		if (FileExists ("modlist.old")) {
			system ("attrib -r modlist.old");	//Clears read only attribute of modlist.old if present, so we can delete the file.
			system ("del modlist.old");
		}
		system ("attrib -r modlist.txt");	//Clears read only attribute of modlist.txt if present, so we can rename the file.
		system ("ren modlist.txt modlist.old");
	} //if
	system ("dir *.es? /a:-d /b /o:d /t:w > modlist.txt"); // quick way to list the mod files: pipe them into a text file.

	//Open modlist.txt file and verify success																
	modlist.open("modlist.txt");			
	if (modlist.fail()) {
		bosslog << endl << "Critical Error! Internal program error! modlist.txt should have been created but it wasn't." << endl;
		bosslog <<         "Make sure you are running as Administrator if using Windows Vista." << endl;
		bosslog <<         "! Utility will end now." << endl;
		bosslog.close();
		system ("start BOSSlog.txt");	//Displays the BOSSlog.txt.
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
	bosslog <<   endl << "------------------------------------" << endl;
	bosslog <<           "Recognised and re-ordered mod files:" << endl;
	bosslog <<           "------------------------------------" << endl;
	x=0;
	found=FALSE;
	while (!order.eof()) {					
		textbuf=ReadLine("order");
		if (IsValidLine(textbuf) && textbuf[0]!='\\') {		//Filter out blank lines, oblivion.esm and remark lines starting with \.
			if (!IsMessage(textbuf)) {						//Deal with mod lines only here. Message lines will be dealt with below.
				if (FileExists(textbuf)) {					//Tidy function not needed as file system removes trailing spaces and isn't case sensitive
					found=TRUE;
					bosslog << endl << textbuf << endl;		// show which mod file is being processed.
					x++;
					modfiletime=esmtime;
					modfiletime.tm_min += x;				//files are ordered in minutes after oblivion.esp .
					ChangeFileDate(textbuf, modfiletime);
				} //if
				else found=FALSE;
			} //if
			else if (found) ShowMessage(textbuf, fcom, ooo, bc, fook2, fwe);		//Deal with message lines here.
		} //if
	} //while

	//Find and show found mods not recognised. Parse each file in modlist.txt and try finding it in masterlist.txt. If not found, unknown.
	bosslog <<   endl << "-----------------------------------------------------------------" << endl;
	bosslog <<           "Unrecognised mod files:                                          " << endl;
	bosslog <<           "Reorder these by hand using your favourite mod ordering utility. " << endl;
	bosslog <<           "-----------------------------------------------------------------" << endl << endl;
	modlist.clear();						//reset position in modlist.txt to start.
	modlist.seekg (0, order.beg);				// "
	while (!modlist.eof()) {	
		textbuf=ReadLine("modlist");
		found=FALSE;
		order.clear ();						//reset position in masterlist.txt to start.
		order.seekg (0, order.beg);			// "
		while (!order.eof() && !found) {	//repeat until end of masterlist.txt or file found.				
			textbuf2=ReadLine("order");
			if (IsMod(textbuf2)) if (Tidy(textbuf)==Tidy(textbuf2)) found=TRUE;		//filter out comment, blank and message lines when checking for match - speeds process up.
		} // while
		if (!found) bosslog << "Unknown mod file: " << textbuf << endl;
	} //while

	//Let people know the program has stopped.
	bosslog <<   endl << endl << "-----------------------------------------------------------" << endl;
	bosslog << "Done.";
	bosslog.close();
	system ("start BOSSlog.txt");	//Displays the BOSSlog.txt.
	return (0);
}