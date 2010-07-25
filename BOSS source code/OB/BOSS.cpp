/*	Better Oblivion Sorting Software
	2.0 Beta
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    	Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    	http://creativecommons.org/licenses/by-nc-nd/3.0/
*/

// Now supports master ESM files from Morrowind and Fallout 3. Notes regarding Oblivion apply equally to those games too.
// Now covers the same FOOK2 detection support for Fallout 3 as the BOSS-F does, and more!

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

/* Update masterlist requirements, but it's disabled for now, so comment these out.
#include <svn_client.h>
#include <svn_pools.h>
#include <svn_fs.h>
#pragma comment(lib, "libsvn_client-1.lib")
#pragma comment(lib, "libsvn_fs-1.lib")
#pragma comment(lib, "libsvn_subr-1.lib")
#include <urlmon.h>					//Currently required for masterlist update, but requires Windows SDK.
#pragma comment(lib, "urlmon.lib")	//Currently required for masterlist update, but requires Windows SDK.
*/

#define SIZE 26 				//used in convertion of date/time struct to a string. Has to be this length.
#define MAXLENGTH 4096			//maximum length of a file name or comment. Big arbitary number.

using namespace std;

ifstream order;						//masterlist.txt - the grand mod order list
ifstream modlist;					//modlist.txt - list of esm/esp files in oblivion/data
ofstream bosslog;					//Output file.
bool fcom;							//true if key FCOM files are found.
bool ooo;                      	 	//true if OOO esm is found.
bool bc;                        	//true if Better Cities esm is found.
bool FOOK2;							//true if key FOOK2 files are found.
bool FWE;							//true if FWE esm is found

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

void ShowMessage(string textbuf, bool fcom, bool ooo, bool bc, bool FOOK2, bool FWE) {
	switch (textbuf[0]) {	
		case '*':
			if (fcom) bosslog << "  !!! FCOM INSTALLATION ERROR: " << textbuf.substr(1) << endl;
			else if (FOOK2) bosslog << "  !!! FOOK2 INSTALLATION ERROR: " << textbuf.substr(1) << endl;
		break;
		case ':':
			bosslog << " . Requires: " << textbuf.substr(1) << endl;
		break;
		case '$':
			if (ooo) bosslog << " . OOO Specific Note: " << textbuf.substr(1) << endl;
			else if (FWE) bosslog << "  . FWE Specific Note: " << textbuf.substr(1) << endl;
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

bool UpdateMasterlist(string path, int game) {
	/*Two update methods: using URLDownloadToFile to grab the raw file from Google Code, or using SVN to Export the latest revision from Google Code.
	The former has a few downsides, in that it doesn't include SVN info, requires the Windows SDK to build (a large download for 1 function),
	and sometimes results in false positives from virus scanners.
	The latter is more complicated, but only requires the SVN development files (~6.5MB). It also seems to require a whole host of dll files too. :(
	And it causes BOSS to crash whenever you try to update, so I'll disable updating for now.
	*/
	return false;
	//The URLDownloadToFile() method:
	/*
	LPCTSTR url;
	if (game == 1) url = "http://better-oblivion-sorting-software.googlecode.com/svn/masterlist.txt";
	else if (game == 2) url = "http://better-oblivion-sorting-software.googlecode.com/svn/FO3Masterlist/masterlist.txt";
	else if (game == 3) url = "http://better-oblivion-sorting-software.googlecode.com/svn/MWmasterlist/masterlist.txt";
	HRESULT hr = URLDownloadToFile(NULL, url, path.c_str(), 0, NULL);
	if (hr == 0) return true;
	else return false;
	*/

	//The SVN method:
	/*
	svn_revnum_t *result_rev;
	char *from;
	char *to;
	svn_opt_revision_t peg_revision;
	svn_opt_revision_t revision;
	svn_client_ctx_t *ctx;
	apr_pool_t *pool;
	svn_error_t *err;

	pool = svn_pool_create (NULL);

	// Initialize the FS library.
	err = svn_fs_initialize (pool);
	if (err) {
      svn_handle_error2 (err, stderr, FALSE, "minimal_client: ");
      return false;
    }

  // Make sure the ~/.subversion run-time config files exist
  err = svn_config_ensure (NULL, pool);
  if (err)
    {
      svn_handle_error2 (err, stderr, FALSE, "minimal_client: ");
      return false;
    }

  // All clients need to fill out a client_ctx object.
  {
    // Initialize and allocate the client_ctx object.
    if ((err = svn_client_create_context (&ctx, pool)))
      {
        svn_handle_error2 (err, stderr, FALSE, "minimal_client: ");
        return false;
      }

    // Load the run-time config file into a hash
    if ((err = svn_config_get_config (&(ctx->config), NULL, pool)))
      {
        svn_handle_error2 (err, stderr, FALSE, "minimal_client: ");
        return false;
      }
  } // end of client_ctx setup

  revision.kind = svn_opt_revision_head;
  peg_revision.kind = svn_opt_revision_head;
  from = "http://better-oblivion-sorting-software.googlecode.com/svn/masterlist.txt";
  to = "masterlist.txt";

  err = svn_client_export4(result_rev,
							from,
							to,
							&peg_revision,
							&revision,
							TRUE,
							TRUE,
							svn_depth_empty,
							NULL,
							ctx,
							pool
							);

if (err) {
	svn_handle_error2 (err, stderr, FALSE, "SVN Export: ");
	return false;
}

  return true;
  */
}

//BOSS.exe [--update | -u] [[--masterlist | -m] "masterlist parent directory"] [[--plugins | -p] "plugins directory"] [[--output | -o] "output directory"] [--help | -h]
int main(int argc, char *argv[]) {					
	
	int x;							//random useful integers
	string textbuf,textbuf2;		//a line of text from a file (should usually end up being be a file name); 			
	struct __stat64 buf;			//temp buffer of info for _stat function
	struct tm esmtime;			    //the modification date/time of the main .esm file
	struct tm modfiletime;			//useful variable to store a file's date/time
	bool found;						
	char modfilechar [SIZE];		//used to convert stuff.

	string listdir;					//stores masterlist parent directory.
	string plugindir;				//stores plugins directory.
	string logdir;					//stores output files directory.
	char *rwd;						//stores working directory at runtime.
	bool update = false;			//To update masterlist or not?
	int game;						//What game's mods are we sorting? 1 = Oblivion, 2 = Fallout 3, 3 = Morrowind.

	//Parse command line arguments.
	if (argc > 1) {
		for (int i=0; i < argc; i++) {
			if (strcmp("--update", argv[i]) == 0 || strcmp("-u", argv[i]) == 0) {
				update = true;
			} else if (strcmp("--masterlist", argv[i]) == 0 || strcmp("-m", argv[i]) == 0) {
				listdir = argv[i + 1];
			} else if (strcmp("--plugins", argv[i]) == 0 || strcmp("-p", argv[i]) == 0) {
				plugindir = argv[i + 1];
			} else if (strcmp("--output", argv[i]) == 0 || strcmp("-o", argv[i]) == 0) {
				logdir = argv[i + 1];
			}else if (strcmp("--help", argv[i]) == 0 || strcmp("-h", argv[i]) == 0) {
				cout << endl << "BOSS [--update | -u] [[--masterlist | -m] \"file parent directory\"] [[--plugins | -p] \"plugins directory\"] [[--output | -o] \"output directory\"] [--help | -h]" << endl << endl;
				cout << "Better Oblivion Sorting Software is a utility that sorts the load order of TESIV: Oblivion, TESIII: Morrowind and Fallout 3 mods according to their relative positions on a frequently-updated masterlist ";
				cout << "to ensure proper load order and minimise incompatibilities between mods. If no optional parameters are set, all paths are set to BOSS's parent directory." << endl << endl;
				cout << "Optional Parameters" << endl << endl;
				cout << "NOTE: Directory paths are given using backslashes, as per normal Windows operation, ie: \"C:\\Users\\\" not \"C:/Users/\".";
				cout << "The trailing backslash must be present. Paths with spaces must be encapsulated in double quotes (\"\")." << endl << endl;
				cout << "-m" << endl << "--masterlist" << endl << "Specify the directory containing the masterlist to be used to sort the mods, and to be updated if the update parameter is set." << endl << endl;
				cout << "-o" << endl << "--output" << endl << "Specify BOSS's output directory, where modlist.txt, BOSSlog.txt and modlist.old (if modlist.txt already exists in that location) will be written to." << endl << endl;
				cout << "-p" << endl << "--plugins" << endl << "Specify the directory that contains the mod plugins to be sorted." << endl << endl;
				cout << "-u" << endl << "--update" << endl << "Automatically updates the local copy of the masterlist using the latest version available on the Google Code repository." << endl << endl;
				exit (0);
			}
		}
	} else {
		listdir = "";
		plugindir = "";
		logdir = "";
	}

	if (FileExists(plugindir+"oblivion.esm")) game = 1;
	else if (FileExists(plugindir+"fallout3.esm")) game = 2;
	else if (FileExists(plugindir+"morrowind.esm")) game = 3;

	if (update == true) {
		cout << endl << "Updating to the latest masterlist from the Google Code repository..." << endl;
		if (UpdateMasterlist(listdir+"masterlist.txt", game) == true) cout << listdir+"masterlist.txt updated!" << endl;
		else cout << "Masterlist update failed." << endl;
	}

	cout << endl << "Better Oblivion Sorting Software working..." << endl;
	
	//Check for creation of BOSSlog.txt.
	bosslog.open(logdir+"BOSSlog.txt");
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
	bosslog <<                 "   v2.0 Beta (10 July 2010)							       " << endl;
	bosslog <<                 "-----------------------------------------------------------" << endl << endl;

	//open masterlist.txt
	order.open(listdir+"masterlist.txt");	
	if (order.fail()) {							
		bosslog << endl << "Critical Error! masterlist.txt does not exist or can't be read!" << endl; 
		bosslog <<         "! Utility will end now." << endl;
		bosslog.close();
		system (("start "+logdir+"BOSSlog.txt").c_str());	//Displays the BOSSlog.txt.
		exit (1); //fail in screaming heap.
	} //if

	//get date for oblivion.esm.
	if (game == 1) _stat64((plugindir+"oblivion.esm").c_str(), &buf);
	else if (game == 2) _stat64((plugindir+"fallout3.esm").c_str(), &buf);
	else if (game == 3) _stat64((plugindir+"morrowind.esm").c_str(), &buf);
	else {				
		bosslog << endl << "Critical Error: Master .ESM file not found (or not accessible)!" << endl;
		bosslog <<         "Make sure you're running this in your Data folder." <<endl;
		bosslog <<         "! Utility will end now." << endl;
		bosslog.close();
		system (("start "+logdir+"BOSSlog.txt").c_str());	//Displays the BOSSlog.txt.
		exit (1); //fail in screaming heap.
	} //else

	_gmtime64_s(&esmtime, &buf.st_mtime);		//convert _stat64 modification date data to date/time struct.

	//Display oblivion.esm's modification date (mostly for debugging)
	_ctime64_s (modfilechar, SIZE, &buf.st_mtime);	//convert date/time to printable string for output.
	bosslog << "Master .ESM date: " << (string)modfilechar;

	if (game == 1) {
		//Check if FCOM or not
		if (fcom=FileExists(plugindir+"FCOM_Convergence.esm")) bosslog << "FCOM detected." << endl;
			else bosslog << "FCOM not detected." << endl;
		if (FileExists(plugindir+"FCOM_Convergence.esp") && !fcom) bosslog << "WARNING: FCOM_Convergence.esm seems to be missing." << endl;
		//Check if OOO or not
		if (ooo=FileExists(plugindir+"Oscuro's_Oblivion_Overhaul.esm")) bosslog << "OOO detected." << endl;
			else bosslog << "OOO not detected." << endl;
		//Check if Better Cities or not
		if (bc=FileExists(plugindir+"Better Cities Resources.esm")) bosslog << "Better Cities detected." << endl;
			else bosslog << "Better Cities not detected." << endl;
	} else if (game == 2) {
		//Check if FOOK2 or not
		if (FOOK2=FileExists(plugindir+"FOOK2 - Main.esm")) bosslog << "FOOK2 Detected" << endl;
			else bosslog << "FOOK2 not detected." << endl;
		if (FileExists(plugindir+"FOOK2 - Main.esp") && !FOOK2) bosslog << "WARNING: FOOK2.esm seems to be missing." << endl;
		//Check if FWE or not
		if (FWE=FileExists(plugindir+"FO3 Wanderers Edition - Main File.esm")) bosslog << "FWE detected." << endl;
			else bosslog << "FWE not detected." << endl;
	}
	bosslog << endl;

	//Generate list of all .esp or .esm files.
	if (FileExists (logdir+"modlist.txt")) {	//add an additional undo level just in case.
		if (FileExists (logdir+"modlist.old")) {
			system (("attrib -r "+logdir+"modlist.old").c_str());	//Clears read only attribute of modlist.old if present, so we can delete the file.
			system (("del "+logdir+"modlist.old").c_str());
		}
		system (("cd "+logdir+" && ren modlist.txt modlist.old").c_str());
	} //if
	system (("dir "+plugindir+"*.es? /a:-d /b /o:d /t:w > "+logdir+"modlist.txt").c_str()); // quick way to list the mod files: pipe them into a text file.

	//Open modlist.txt file and verify success																
	modlist.open(logdir+"modlist.txt");			
	if (modlist.fail()) {
		bosslog << endl << "Critical Error! Internal program error! modlist.txt should have been created but it wasn't." << endl;
		bosslog <<         "Make sure you are running as Administrator if using Windows Vista." << endl;
		bosslog <<         "! Utility will end now." << endl;
		bosslog.close();
		system (("start "+logdir+"BOSSlog.txt").c_str());	//Displays the BOSSlog.txt.
		exit(1); //fail in screaming heap.
	} //if

	//Save current working directory for later, change working directory to plugin directory, and remove any read only attributes from esm/esp files if present.
	rwd = _getcwd(NULL,0);
	_chdir(plugindir.c_str());
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
			else if (found) ShowMessage(textbuf, fcom, ooo, bc, FOOK2, FWE);		//Deal with message lines here.
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
	_chdir(rwd);
	system (("start "+logdir+"BOSSlog.txt").c_str());	//Displays the BOSSlog.txt.
	return (0);
}