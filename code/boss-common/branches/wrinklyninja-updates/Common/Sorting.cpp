/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision$, $Date$
*/

#include <boost/algorithm/string/replace.hpp>
#include "Globals.h"
#include "Sorting.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>


namespace boss {
	using namespace std;

	void ShowMessage(string textbuf, int game) {
		size_t pos1,pos2;
		string link;
		pos1 = textbuf.find("http");
		while (pos1 != string::npos) {
			pos2 = textbuf.find(" ",pos1);
			link = textbuf.substr(pos1,pos2-pos1);
			link = "<a href='"+link+"'>"+link+"</a>";
			textbuf.replace(pos1,pos2-pos1,link);
			pos1 = textbuf.find("http",pos1 + link.length());
		}
		switch (textbuf[0]) {	
		case '*':
			if (fcom && game == 1) bosslog << "<li class='error'>!!! FCOM INSTALLATION ERROR: " << textbuf.substr(1) << "</li>" << endl;
			else if (fcom && game == 2) bosslog << "<li class='error'>!!! FOOK2 INSTALLATION ERROR: " << textbuf.substr(1) << "</li>" << endl;
			break;
		case ':':
			bosslog << "<li>Requires: " << textbuf.substr(1) << "</li>" << endl;
			break;
		case '$':
			if (ooo && game == 1) bosslog << "<li>OOO Specific Note: " << textbuf.substr(1) << "</li>" << endl;
			else if (ooo && game == 2) bosslog << "<li>FWE Specific Note: " << textbuf.substr(1) << "</li>" << endl;
			break;
		case '%':
			pos1 = textbuf.find("{{BASH:");
			if (pos1 != string::npos) {
				pos2 = textbuf.find("}}",pos1);
				size_t pos3;
				pos3 = textbuf.find(",",pos1);
				while (pos3 != string::npos && pos3 < pos2) {
					textbuf.replace(pos3,1,", ");
					pos3 = textbuf.find(",",pos3+9);
				}
			}
			boost::algorithm::ireplace_all(textbuf,"remove","<span class='error'>remove</span>");
			bosslog << "<li><t>Bash Tag suggestion(s):</t> " << textbuf.substr(1) << "</li>" << endl;
			break;
		case '\?':
			bosslog << "<li>Note: " << textbuf.substr(1) << "</li>" << endl;
			break;
		case '"':
			bosslog << "<li>Incompatible with: " << textbuf.substr(1) << "</li>" << endl;
			break;
		case '^':
			if (game == 1) bosslog << "<li>Better Cities Specific Note: " << textbuf.substr(1) << "</li>" <<endl;
			break;
		} //switch
	}

	string ReadLine (string file) {						//Read a line from a file. Could be rewritten better.
		char cbuffer[MAXLENGTH];						//character buffer.
		string textbuf;

		if (file=="order") order.getline(cbuffer,MAXLENGTH);				//get a line of text from the masterlist.txt text file
		//No internal error handling here.
		textbuf=cbuffer;
		if (textbuf.length() > 0)
			if (file=="order") {		//If parsing masterlist.txt, parse only lines that start with > or < depending on FCOM installation. Allows both FCOM and nonFCOM differentiaton.
				if ((textbuf[0]=='>') && (fcom)) textbuf.erase(0,1);
				else if ((textbuf[0]=='>') && (!fcom)) textbuf='\\';
				else if ((textbuf[0]=='<') && (!fcom)) textbuf.erase(0,1);
				else if ((textbuf[0]=='<') && (fcom)) textbuf='\\';
			} //if
		return (textbuf);
	}

	/// GetModHeader(string textbuf):
	///  - Reads the header from mod file and prints a string representation which includes the version text, if found.
	///
	string GetModHeader(const string& filename, bool ghosted) {

	//	ostringstream out;
		ModHeader header;

		// Read mod's header now...
		if (ghosted) header = ReadHeader(data_path.string()+"/"+filename+".ghost");
		else header = ReadHeader(data_path.string()+"/"+filename);

		// The current mod's version if found, or empty otherwise.
		string version = header.Version;

		//Return the version if found, otherwise an empty string.
		return version;
	}
}
