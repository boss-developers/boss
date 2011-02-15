/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision$, $Date$
*/

#include <boost/algorithm/string/replace.hpp>
#include "Sorting.h"
#include "Globals.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>
#include "Support/Types.h"

namespace boss {
	using namespace std;

	void ShowMessage(string textbuf, int game) {
		size_t pos1,pos2,pos3;
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
			bosslog << "<li class='error'>!!! CRITICAL INSTALLATION ERROR: " << textbuf.substr(1) << "</li>" << endl;
			break;
		case '"':
			bosslog << "<li class='warn'>Warning: " << textbuf.substr(1) << "</li>" << endl;
			break;
		case ':':
			bosslog << "<li>Requires: " << textbuf.substr(1) << "</li>" << endl;
			break;
		case '%':
			pos1 = textbuf.find("{{BASH:");
			if (pos1 != string::npos) {
				pos2 = textbuf.find("}}",pos1);
				pos3 = textbuf.find(",",pos1);
				while (pos3 != string::npos && pos3 < pos2) {
					textbuf.replace(pos3,1,", ");
					pos3 = textbuf.find(",",pos3+9);
				}
			}
			boost::algorithm::ireplace_all(textbuf,"remove","<span class='error'>remove</span>");
			bosslog << "<li><span class='tags'>Bash Tag suggestion(s):</span> " << textbuf.substr(1) << "</li>" << endl;
			break;
		case '\?':
			bosslog << "<li>Note: " << textbuf.substr(1) << "</li>" << endl;
			break;
		} //switch
	}

	string ReadLine (string file) {						//Read a line from a file. Could be rewritten better.
		char cbuffer[MAXLENGTH];						//character buffer.
		string textbuf;
		order.getline(cbuffer,MAXLENGTH);				//get a line of text from the masterlist.txt text file
		textbuf=cbuffer;								//No internal error handling here.
		return textbuf;
	}
}
