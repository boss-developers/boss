/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2002 $, $Date: 2010-12-05 21:35:32 +0000 (Sun, 05 Dec 2010) $
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

	void ShowMessage(wstring textbuf, int game) {
		size_t pos1,pos2,pos3;
		wstring link;
		pos1 = textbuf.find(L"http");
		while (pos1 != string::npos) {
			pos2 = textbuf.find(L" ",pos1);
			link = textbuf.substr(pos1,pos2-pos1);
			link = L"<a href='"+link+L"'>"+link+L"</a>";
			textbuf.replace(pos1,pos2-pos1,link);
			pos1 = textbuf.find(L"http",pos1 + link.length());
		}
		switch (textbuf[0]) {	
		case '*':
			if (fcom && game == 1) bosslog << L"<li class='error'>!!! FCOM INSTALLATION ERROR: " << textbuf.substr(1) << L"</li>" << endl;
			else if (fcom && game == 2) bosslog << L"<li class='error'>!!! FOOK2 INSTALLATION ERROR: " << textbuf.substr(1) << L"</li>" << endl;
			break;
		case ':':
			bosslog << L"<li>Requires: " << textbuf.substr(1) << L"</li>" << endl;
			break;
		case '$':
			if (ooo && game == 1) bosslog << L"<li>OOO Specific Note: " << textbuf.substr(1) << L"</li>" << endl;
			else if (ooo && game == 2) bosslog << L"<li>FWE Specific Note: " << textbuf.substr(1) << L"</li>" << endl;
			break;
		case '%':
			pos1 = textbuf.find(L"{{BASH:");
			if (pos1 != string::npos) {
				pos2 = textbuf.find(L"}}",pos1);
				pos3 = textbuf.find(L",",pos1);
				while (pos3 != string::npos && pos3 < pos2) {
					textbuf.replace(pos3,1,L", ");
					pos3 = textbuf.find(L",",pos3+9);
				}
			}
			boost::algorithm::ireplace_all(textbuf,L"remove",L"<span class='error'>remove</span>");
			bosslog << L"<li><t>Bash Tag suggestion(s):</t> " << textbuf.substr(1) << L"</li>" << endl;
			break;
		case '\?':
			bosslog << L"<li>Note: " << textbuf.substr(1) << L"</li>" << endl;
			break;
		case '"':
			bosslog << L"<li>Incompatible with: " << textbuf.substr(1) << L"</li>" << endl;
			break;
		case '^':
			if (game == 1) bosslog << L"<li>Better Cities Specific Note: " << textbuf.substr(1) << L"</li>" <<endl;
			break;
		} //switch
	}

	wstring ReadLine (string file) {						//Read a line from a file. Could be rewritten better.
		wchar_t cbuffer[MAXLENGTH];						//character buffer.
		wstring textbuf;

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
	wstring GetModHeader(const wstring& filename, bool ghosted) {

	//	ostringstream out;
		ModHeader header;

		// Read mod's header now...
		if (ghosted) header = ReadHeader(data_path.wstring()+L"/"+filename+L".ghost");
		else header = ReadHeader(data_path.wstring()+L"/"+filename);

		// The current mod's version if found, or empty otherwise.
		wstring version = header.Version;

		//Return the version if found, otherwise an empty string.
		return version;
	}
}
