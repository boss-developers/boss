/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision$, $Date$
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>
#include <sys/stat.h>
#include <sys/utime.h>

#include "Globals.h"
#include "Sorting.h"

namespace boss {
	using namespace std;

	void ChangeFileDate(string textbuf, tm modfiletime) {
		//changes a file's modification date
		struct __utimbuf64 ut;							//way to change time data for Windows _utime function
		int a;											//holds result of file changes

		ut.actime = _mkgmtime64(&modfiletime);			//set up ut structure for _utime64.
		ut.modtime = _mkgmtime64(&modfiletime);	
		a = _utime64(textbuf.c_str(), &ut);				//finally, change the file date.

		if (a != 0) {
			bosslog << endl << "Program error - file " << textbuf << " could not have its date changed, code " << a << endl;
		}
	}

	void ShowMessage(string textbuf, bool fcom, bool ooo, bool bc, bool fook2, bool fwe) {
		switch (textbuf[0]) {	
		case '*':
			if (fcom) bosslog << "<li style='color:red;'>!!! FCOM INSTALLATION ERROR: " << textbuf.substr(1) << "</li>" << endl;
			else if (fook2) bosslog << "<li style='color:red;'>!!! FOOK2 INSTALLATION ERROR: " << textbuf.substr(1) << "</li>" << endl;
			break;
		case ':':
			bosslog << "<li>Requires: " << textbuf.substr(1) << "</li>" << endl;
			break;
		case '$':
			if (ooo) bosslog << "<li>OOO Specific Note: " << textbuf.substr(1) << "</li>" << endl;
			else if (fwe) bosslog << "<li>FWE Specific Note: " << textbuf.substr(1) << "</li>" << endl;
			break;
		case '%':
			bosslog << "<li>Bashed Patch tag suggestion: " << textbuf.substr(1) << "</li>" << endl;
			break;
		case '\?':
			bosslog << "<li>Note: " << textbuf.substr(1) << "</li>" << endl;
			break;
		case '"':
			bosslog << "<li>Incompatible with: " << textbuf.substr(1) << "</li>" << endl;
			break;
		case '^':
			bosslog << "<li>Better Cities Specific Note: " << textbuf.substr(1) << "</li>" << endl;
			break;
		} //switch
	}

	string ReadLine (string file) {						//Read a line from a file. Could be rewritten better.
		char cbuffer[MAXLENGTH];						//character buffer.
		string textbuf;

		if (file=="order") order.getline(cbuffer,MAXLENGTH);				//get a line of text from the masterlist.txt text file
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

	/// GetModHeader(string textbuf):
	///  - Reads the header from mod file and prints a string representation which includes the version text, if found.
	///
	string GetModHeader(const string& filename, bool ghosted) {

		ostringstream out;
		ModHeader header;

		// Read mod's header now...
		if (ghosted) header = ReadHeader(filename+".ghost");
		else header = ReadHeader(filename);

		// The current mod's version if found, or empty otherwise.
		string version = header.Version;

		// Output the mod information...
		out << endl << filename;	// show which mod file is being processed.

		// If version's found the show it...
		if (! version.empty()) out << " [Version " << version << "]";

		return out.str();
	}
};
