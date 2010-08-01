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
#include <strstream>

#include <Support/Types.h>

#include "Updater.h"

namespace boss {
	using namespace std;

	const string SVN_REVISION_KW	= "$" "Revision" "$";			// Left as separated parts to avoid keyword expansion
	const string SVN_DATE_KW		= "$" "Date" "$";				// Left as separated parts to avoid keyword expansion
	const string SVN_CHANGEDBY_KW	= "$" "LastChangedBy" "$";		// Left as separated parts to avoid keyword expansion

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
		if (game == 1) url = "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-oblivion/masterlist.txt";
		else if (game == 2) url = "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-fallout/masterlist.txt";
		else if (game == 3) url = "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-morrowind/masterlist.txt";

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
		oldline = "? Masterlist Information: " + SVN_REVISION_KW + ", " + SVN_DATE_KW + ", " + SVN_CHANGEDBY_KW;
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
};