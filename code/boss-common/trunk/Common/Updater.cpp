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
	namespace fs = boost::filesystem;
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
		const char *url;							//Masterlist file url
		const char *revurl;                         //Revision number url
		char cbuffer[4096];
		CURL *curl;									//cURL handle
		string buffer,revision,newline,line;		//A bunch of strings.
		int start,end;								//Position holders for trimming strings.
		ifstream mlist;								//Input stream.
		ofstream out;								//Output stream.
		const string SVN_REVISION_KW = "$" "Revision" "$";                   // Left as separated parts to avoid keyword expansion
		const string SVN_DATE_KW = "$" "Date" "$";                           // Left as separated parts to avoid keyword expansion
		const string SVN_CHANGEDBY_KW= "$" "LastChangedBy" "$";              // Left as separated parts to avoid keyword expansion
		string oldline = "? Masterlist Information: "+SVN_REVISION_KW+", "+SVN_DATE_KW+", "+SVN_CHANGEDBY_KW;

		//Which masterlist to get?
		if (game == 1) url = "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-oblivion/masterlist.txt";
		else if (game == 2) url = "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-fallout/masterlist.txt";
		else if (game == 3) url = "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-nehrim/masterlist.txt";

		//Which Revision number to get
		if (game == 1) revurl = "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-oblivion/masterlist.txt";
		else if (game == 2) revurl = "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-fallout/masterlist.txt";
		else if (game == 3) revurl = "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-nehrim/masterlist.txt";

		//curl will be used to get stuff from the internet, so initialise it.
		curl = curl_easy_init();
		if (!curl) return -1;		//If curl is null, resource failed to be initialised so exit with error.

		//Get revision number from revurl page text.
		curl_easy_setopt(curl, CURLOPT_URL, "http://code.google.com/p/better-oblivion-sorting-software/source/browse/#svn");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);	
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &revision );
		if (curl_easy_perform(curl)!=0) return -1;		//If download fails, exit with a failure.

		//Extract revision number from page text.
		if (game == 1) start = revision.find("boss-oblivion");
		else if (game == 2) start = revision.find("boss-fallout");
		else if (game == 3) start = revision.find("boss-nehrim");
		start = revision.find("KB\",\"", start) + 5; 
		end = revision.find("\"",start) - start;
		revision = revision.substr(start,end);
		newline = "? Masterlist Revision: "+revision;

		//Compare remote revision to current masterlist revision - if identical don't waste time/bandwidth updating it.
		if (fs::exists("BOSS\\masterlist.txt")) {
			mlist.open("BOSS\\masterlist.txt");
			if (mlist.fail()) return -1;	//If the masterlist can't be opened, exit with a failure.
			while (!mlist.eof()) {
				mlist.getline(cbuffer,4096);
				line=cbuffer;
				if (line.find("? Masterlist") != string::npos) {
					if (line.find(newline) != string::npos) return 0;
					else break;
				}
			}
			mlist.close();
		}

		// Use curl to get latest masterlist file from SVN repository
		//Change url and data structure settings, writefunction setting is retained.
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
		curl_easy_setopt(curl, CURLOPT_CRLF);
		if (curl_easy_perform(curl)!=0) return -1; //If download fails, exit with a failure.

		//Clean up and close curl handle now that it's finished with.
		curl_easy_cleanup(curl);

		//Replace SVN keywords with revision number and replace current masterlist, or write a new one if it doesn't already exist.
		int pos = buffer.find(oldline);
		buffer.replace(pos,oldline.length(),newline);
		out.open("BOSS\\masterlist.txt", ios_base::trunc);
		if (out.fail()) return -1;	//If the masterlist can't be opened, exit with a failure.
		out << buffer;
		out.close();

		//Return revision number.
		return atoi(revision.c_str());
	}
};