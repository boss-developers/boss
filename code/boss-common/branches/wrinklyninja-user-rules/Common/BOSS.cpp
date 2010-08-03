/*	Better Oblivion Sorting Software
	1.6
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
#include <vector>

#include <Support/Types.h>
#include <Support/ModFormat.h>

#define SIZE 26 				//used in convertion of date/time struct to a string. Has to be this length.
#define MAXLENGTH 4096			//maximum length of a file name or comment. Big arbitary number.

using namespace std;
using namespace boss;

ifstream order;						//masterlist.txt - the grand mod order list
ifstream userlist;					//userlist.txt - holds custom user sorting rules for mods not in masterlist.
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
	else if (file=="modlist") modlist.getline(cbuffer,MAXLENGTH);			//get a line of text from the modlist.txt text file
	else if (file=="userlist") userlist.getline(cbuffer,MAXLENGTH);			//get a line of text from the userlist.txt text file
	else if (file=="masterlist") order.getline(cbuffer,MAXLENGTH);
	//No internal error handling here.
	textbuf=(string)cbuffer;
	if (file=="order") {		//If parsing masterlist.txt, parse only lines that start with > or < depending on FCOM installation. Allows both FCOM and nonFCOM differentiaton.
		if ((textbuf[0]=='>') && (fcom)) textbuf.erase(0,1);
		if ((textbuf[0]=='>') && (!fcom)) textbuf='\\';
		if ((textbuf[0]=='<') && (!fcom)) textbuf.erase(0,1);
		if ((textbuf[0]=='<') && (fcom)) textbuf='\\';
	} else if (file=="masterlist" && (textbuf[0]=='>' || textbuf[0]=='<')) textbuf.erase(0,1);

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
	char *url;									//Masterlist file url
	CURL *curl;									//Some cURL resource...
	string buffer,revision,oldline,newline;		//A bunch of strings.
	int start,end;								//Position holders for trimming strings.
	ofstream out;								//Output stream.
	const string SVN_REVISION_KW = "$" "Revision" "$";                   // Left as separated parts to avoid keyword expansion
    const string SVN_DATE_KW = "$" "Date" "$";                           // Left as separated parts to avoid keyword expansion
    const string SVN_CHANGEDBY_KW= "$" "LastChangedBy" "$";              // Left as separated parts to avoid keyword expansion

	//Get HEAD revision number from http://better-oblivion-sorting-software.googlecode.com/svn/ page text.
	curl = curl_easy_init();
	if (curl){
		curl_easy_setopt(curl, CURLOPT_URL, "http://better-oblivion-sorting-software.googlecode.com/svn/");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);	
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &revision );
		curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}
	start = revision.find("Revision ");
	end = revision.find(": /");
	end = end - (start+9);
	revision = revision.substr(start+9,end);
	stringstream ss(revision);
	ss >> end;

	//Which masterlist to get?
	if (game == 1) url = "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-oblivion/masterlist.txt";
	else if (game == 2) url = "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-fallout/masterlist.txt";
	else if (game == 3) url = "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-morrowind/masterlist.txt";

	//Get SVN masterlist file.
	oldline = "? Masterlist Information: "+SVN_REVISION_KW+", "+SVN_DATE_KW+", "+SVN_CHANGEDBY_KW;
	newline = "? Masterlist Revision: "+revision;
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
		//Correct formatting and replace SVN keywords with revision number.
		out.open("BOSS\\masterlist.txt",ios::in|ios::trunc);
		int pos = buffer.find(oldline);
		buffer.replace(pos,oldline.length(),newline);
		pos = buffer.find("\r");
		while (pos > -1) {
			buffer.replace(pos,2,"\n");
			pos = buffer.find("\r");
		}
		out << buffer;
		out.close();
    }
	//Return revision number.
	return end;
}

/// GetModHeader(string textbuf):
///  - Reads the header from mod file and prints a string representation which includes the version text, if found.
///
string GetModHeader(const string& filename) {

	ostringstream out;

	// Read mod's header now...
	ModHeader header = ReadHeader(filename);

	// The current mod's version if found, or empty otherwise.
	string version = header.Version;

	// Output the mod information...
	out << endl << filename;	// show which mod file is being processed.

	// If version's found the show it...
	if (! version.empty()) {
		out << " [Version ";
		out << version;
		out << "]";
	}

	return out.str();
}

//BOSS [--update | -u] [--help | -h] [--version-check | -V]
int main(int argc, char *argv[]) {					
	
	int x;							//random useful integers
	string textbuf,textbuf2,textbuf3;		//a line of text from a file (should usually end up being be a file name); 			
	struct __stat64 buf;			//temp buffer of info for _stat function
	struct tm esmtime;			    //the modification date/time of the main .esm file
	struct tm modfiletime;			//useful variable to store a file's date/time
	bool found;						
	char modfilechar [SIZE];		//used to convert stuff.

	bool update = false;			//To update masterlist or not?
	bool version_parse = false;		//Enable parsing of mod's headers to look for version strings
	int game;						//What game's mods are we sorting? 1 = Oblivion, 2 = Fallout 3, 3 = Morrowind.

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

	//Create BOSS/ directory.
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
	bosslog <<                 "   v1.6 (1 August 2010)									   " << endl;
	bosslog <<                 "-----------------------------------------------------------" << endl << endl;

	//Enter The Userlist...
	//Yeah, this is a bit of a dump, but I can always streamline it after I've got it working.
	userlist.open("BOSS\\userlist.txt");	//File need not exist for BOSS to work, so allow BOSS to keep running if it's not there.
	if (userlist.good()) {
		bosslog << endl << "------------------" << endl << "Userlist Messages:" << endl << "------------------" << endl << endl;
		order.open("BOSS\\masterlist.txt");
		if (order.fail()) {							
			bosslog << endl << "Critical Error! masterlist.txt does not exist or can't be read!" << endl
							<< "! Utility will end now." << endl;
			bosslog.close();
			system ("start BOSS\\BOSSlog.txt");	//Displays the BOSSlog.txt.
			exit (1); //fail in screaming heap.
		}
		/* Parse userlist, looking for looking for rule definitions consisting of a mod line and a rule line.
		When a mod line is found, search the masterlist for it. If it is in the masterlist, print an error and move to the next rule.
		If it isn't in the masterlist, record the line contents (mod filename), then search the masterlist for the rule line
		(minus the rule character (< or >)). Once the rule character is found, insert the mod line at the end of the line containing
		the rule line in the masterlist. */
		//First store rules in memory.
		vector<string> modlines;
		vector<string> rulelines;
		vector<int> index;
		while (!userlist.eof()) {
			textbuf=ReadLine("userlist");
			if (IsValidLine(textbuf) && IsMod(textbuf.substr(1)) && (textbuf[0]=='\?' || textbuf[0]==':')) {	//Check that it's a valid line, it lists a mod, and it's got a mod symbol.
				modlines.push_back(textbuf);
			} else if (IsValidLine(textbuf) && IsMod(textbuf) && (textbuf[0]=='>' || textbuf[0]=='<')) {	//Check that it's a valid line, it lists a mod, and it's got a sort symbol.
				rulelines.push_back(textbuf);
			}
		}
		//Now iterate through modlines. If the modline starts with "?", searching masterlist for it, and note it's position in the vector for later removal.
		if (modlines.size()>0) {
			for (int i = 0; i < (int)modlines.size(); i++) {
				if (modlines[i][0]=='\?') {
					order.clear();
					order.seekg(0, order.beg);
					while (!order.eof()) {
						textbuf=ReadLine("masterlist");
						if (Tidy(textbuf) == Tidy(modlines[i].substr(1))) index.push_back(i);
					}
				}
			}
		}
		//Remove recorded positions.
		if (index.size()>0) {
			for (int i=0;i<(int)index.size();i++) {
				bosslog << "\"" << modlines[index[i]-i].substr(1) << "\" already present in masterlist.txt. Custom sorting rule skipped." << endl << endl;
				modlines.erase(modlines.begin()+index[i]-i);
				rulelines.erase(rulelines.begin()+index[i]-i);
			}
		}
		//And finally, add to the masterlist.
		//However, we can't add a line directly after the rule line match, since that mod may have comments attached.
		//We must add to before the next mod line instead.
		//We've now got to take account of differences in addition and override rules.
		ofstream outfile;
		outfile.open("BOSS\\masterlist.tmp");
		order.clear();
		order.seekg(0, order.beg);
		found = false;
		bool overr = false;
		vector<string> tbuff;
		while (!order.eof()) {
			textbuf=ReadLine("masterlist");
				if (IsValidLine(textbuf) && IsMod(textbuf) && rulelines.size() > 0) {
					if (!found) {										//If we haven't found a load after line, we can keep looking for ruleline matches in general.
						for (int i = 0; i < (int)rulelines.size(); i++) {
							if (Tidy(textbuf) == Tidy(modlines[i].substr(1))) {		//Found the modline in the masterlist. Will only occur for override rules, and only once per line.
								//Override removes original mod line, so prevent original line being written.
								//It then functions as an addition rule.
								overr = true;
								break;
							}
							if (Tidy(textbuf) == Tidy(rulelines[i].substr(1))) {	//Found the ruleline in the masterlist.
								if (rulelines[i][0] == '>') {
									tbuff.push_back(modlines[i].substr(1));							//Add line to vector string containing lines to be added before next mod line.
									textbuf2 = textbuf;												//Store till we find another mod line.
									found = true;
								} else if (rulelines[i][0] == '<') {
									outfile << modlines[i].substr(1) << endl;						//Add the modline before passing the original line.
									bosslog << "\"" << modlines[i].substr(1) << "\" added to masterlist before \"" << textbuf << "\"" << endl << endl;
								}
							}
						}
						if (!overr) outfile << textbuf << endl;			//Write the original line once only, and only if not overridden.
						else overr = false;
					} else {	//We have found a load after rule line, so we add the stored modline to the beginning of the next line containing a mod.
						for (int i=0;i<(int)tbuff.size();i++) {					//Loop through all members of tbuff.
							outfile << tbuff[i] << endl;
							bosslog << "\"" << tbuff[i] << "\" added to masterlist after \"" << textbuf2 << "\"" << endl << endl;
						}
						outfile << textbuf << endl;			//Write the original line once only.
						found = false;						//Reset conditions.
						tbuff.clear();						//Reset conditions.
					}
				} else outfile << textbuf << endl;		//Not a mod line, or there are no rules, so pass it without any changes.
			}
		outfile.close();
		order.close();
		userlist.close();
		system ("del BOSS\\masterlist.txt");
		system ("ren BOSS\\masterlist.tmp masterlist.txt");
		bosslog << "Finished importing userlist.txt sorting rules." << endl << endl;
	}
	bosslog << endl << "-----" << endl << "Notes" << endl << "-----" << endl << endl;

	//open masterlist.txt
	order.open("BOSS\\masterlist.txt");	
	if (order.fail()) {							
		bosslog << endl << "Critical Error! masterlist.txt does not exist or can't be read!" << endl; 
		bosslog <<         "! Utility will end now." << endl;
		bosslog.close();
		system ("start BOSS\\BOSSlog.txt");	//Displays the BOSSlog.txt.
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
		bosslog << endl << "Critical Error! Internal program error! modlist.txt should have been created but it wasn't." << endl;
		bosslog <<         "Make sure you are running as Administrator if using Windows Vista." << endl;
		bosslog <<         "! Utility will end now." << endl;
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

					string text = version_parse ? GetModHeader(textbuf) : textbuf;

					bosslog << endl << text << endl;		// show which mod file is being processed.

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
			if (IsValidLine(textbuf) && textbuf[0]!='\\') {		//Filter out blank lines, oblivion.esm and remark lines starting with \.
				if (IsMod(textbuf2)) if (Tidy(textbuf)==Tidy(textbuf2)) found=TRUE;		//filter out comment, blank and message lines when checking for match - speeds process up.
			}
		} // while
		if (!found && textbuf.length()>1) bosslog << "Unknown mod file: " << textbuf << endl;
	} //while

	//Let people know the program has stopped.
	bosslog <<   endl << endl << "-----------------------------------------------------------" << endl;
	bosslog << "Done.";
	bosslog.close();
	system ("start BOSS\\BOSSlog.txt");	//Displays the BOSSlog.txt.
	return (0);
}