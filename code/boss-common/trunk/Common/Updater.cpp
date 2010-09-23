/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision$, $Date$
*/

#include <stdlib.h>
#include <fstream>
#include <sstream>
#include "Updater.h"

namespace boss {
	using namespace std;

	int writer(char *data, size_t size, size_t nmemb, string *buffer){
		int result = 0;
		if(buffer != NULL) {
			buffer -> append(data, size * nmemb);
			result = size * nmemb;
		}
		return result;
	} 

	int UpdateMasterlist(int game) {
		const char *url;									//Masterlist file url
		CURL *curl;									//Some cURL resource...
		string buffer,revision,oldline,newline;		//A bunch of strings.
		int start,end;								//Position holders for trimming strings.
		ofstream out;								//Output stream.
		const string SVN_REVISION_KW = "$" "Revision" "$";                   // Left as separated parts to avoid keyword expansion
		const string SVN_DATE_KW = "$" "Date" "$";                           // Left as separated parts to avoid keyword expansion
		const string SVN_CHANGEDBY_KW= "$" "LastChangedBy" "$";              // Left as separated parts to avoid keyword expansion

		//Which masterlist to get?
		if (game == 1) url = "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-oblivion/masterlist.txt";
		else if (game == 2) url = "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-fallout/masterlist.txt";
		else if (game == 3) url = "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-morrowind/masterlist.txt";

		//Use curl to get HEAD revision number and latest masterlist file from SVN repository.
		//Get HEAD revision number from http://better-oblivion-sorting-software.googlecode.com/svn/ page text.
		curl = curl_easy_init();
		if (curl) {
			curl_easy_setopt(curl, CURLOPT_URL, "http://better-oblivion-sorting-software.googlecode.com/svn/");
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);	
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &revision );
			curl_easy_perform(curl);
			//If download fails, curl_easy_perform != 0.
			if (curl_easy_perform(curl)!=0) return -1;
			curl_easy_setopt(curl, CURLOPT_URL, url);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
			curl_easy_setopt(curl, CURLOPT_CRLF);
			//If download fails, curl_easy_perform != 0.
			if (curl_easy_perform(curl)!=0) return -1;
			curl_easy_cleanup(curl);
		}
		//Extract revision number from page text.
		start = revision.find("Revision ");
		end = revision.find(": /");
		end = end - (start+9);
		revision = revision.substr(start+9,end);
		oldline = "? Masterlist Information: "+SVN_REVISION_KW+", "+SVN_DATE_KW+", "+SVN_CHANGEDBY_KW;
		newline = "? Masterlist Revision: "+revision;
		//Correct masterlist formatting and replace SVN keywords with revision number.
		out.open("BOSS\\masterlist.txt",ios::in|ios::trunc);
		//If the masterlist can't be opened, exit with a failure.
		if (out.fail()) return -1;
		int pos = buffer.find(oldline);
		buffer.replace(pos,oldline.length(),newline);
		out << buffer;
		out.close();
		//Return revision number.
		return atoi(revision.c_str());
	}
};