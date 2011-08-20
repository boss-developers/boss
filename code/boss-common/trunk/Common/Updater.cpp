/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

	Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
	http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision$, $Date$
*/

#include "Updater.h"

#include <stdio.h>
#include <iostream>
#include <fstream>

#ifndef CURL_STATICLIB
#define CURL_STATICLIB			//Tells the compiler to use curl as a static library.
#endif
#include <curl/curl.h>
#include <curl/easy.h>
#include "Error.h"

namespace boss {
	namespace fs = boost::filesystem;
	using namespace std;

	int writer(char *data, size_t size, size_t nmemb, void *buffer){
		string *str = (string*)buffer;
		if(str != NULL) {
			str -> append(data, size * nmemb);
			return size * nmemb;
		}
		return 0;
	} 

	//Download progress for current file function.
	int progress_func(void *data, double dlTotal, double dlNow, double ulTotal, double ulNow) {
		double fractiondownloaded = dlNow / dlTotal;
		printf("%3.0f%% of %3.0f% KB\r",fractiondownloaded*100,dlTotal/1024);
		fflush(stdout);
		return 0;
	}

	int GetLocalMasterlistRevision() {
		string line, newline = "? Masterlist Revision: ";
		ifstream mlist;
		char cbuffer[4096];
		size_t pos1;

		//Compare remote revision to current masterlist revision - if identical don't waste time/bandwidth updating it.
		if (fs::exists(masterlist_path)) {
			mlist.open(masterlist_path.c_str());
			if (mlist.fail())
				throw boss_error() << err_detail("Masterlist cannot be opened.");
			while (!mlist.eof()) {
				mlist.getline(cbuffer,4096);
				line=cbuffer;
				if (line.find(newline) != string::npos) {
					//Hooray, we've found the line.
					pos1 = line.find(" (",23);
					line = line.substr(23,pos1-23);
					return atoi(line.c_str());  //Masterlist already at latest revision.
				}
			}
		}
		mlist.close();
		return 0;  //No version found.
	}

	int UpdateMasterlist() {
		const char *url;							//Masterlist file url
		char cbuffer[4096];
		char errbuff[CURL_ERROR_SIZE];
		CURL *curl;									//cURL handle
		string buffer,revision,newline,line,proxy_str;		//A bunch of strings.
		size_t start,end;								//Position holders for trimming strings.
		CURLcode ret;
		ifstream mlist;								//Input stream.
		ofstream out;								//Output stream.
		const string SVN_REVISION_KW = "$" "Revision" "$";                   // Left as separated parts to avoid keyword expansion
		const string SVN_DATE_KW = "$" "Date" "$";                           // Left as separated parts to avoid keyword expansion
		const string SVN_CHANGEDBY_KW= "$" "LastChangedBy" "$";              // Left as separated parts to avoid keyword expansion
		string oldline = "? Masterlist Information: "+SVN_REVISION_KW+", "+SVN_DATE_KW+", "+SVN_CHANGEDBY_KW;
		const char *revision_url = "http://code.google.com/p/better-oblivion-sorting-software/source/browse/#svn";

		//Which masterlist to get?
		if (game == 1) url = "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-oblivion/masterlist.txt";
		else if (game == 2) url = "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-fallout/masterlist.txt";
		else if (game == 3) url = "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-nehrim/masterlist.txt";
		else if (game == 4) url = "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-fallout-nv/masterlist.txt";
		else url = "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-skyrim/masterlist.txt";
		
		//curl will be used to get stuff from the internet, so initialise it.
		curl = curl_easy_init();
		if (!curl) 
			throw boss_error() << err_detail("Curl could not be initialised.");
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuff);	//Set error buffer for curl.
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 20);		//Set connection timeout to 20s.
		curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
		
		//Set up proxy stuff.
		if (proxy_type != "direct" && proxy_host != "none" && proxy_port != "0") {
			//All of the settings have potentially valid proxy-ing values.
			proxy_str = proxy_host + ":" + proxy_port;
			ret = curl_easy_setopt(curl, CURLOPT_PROXY, proxy_str.c_str());
			if (ret!=CURLE_OK) {
				curl_easy_cleanup(curl);
				throw boss_error() << err_detail("Invalid proxy hostname or port specified.");
			}

			if (proxy_type == "http")
				curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
			else if (proxy_type == "http1_0")
				curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP_1_0);
			else if (proxy_type == "socks4")
				curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS4);
			else if (proxy_type == "socks4a")
				curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS4A);
			else if (proxy_type == "socks5")
				curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);
			else if (proxy_type == "socks5h")
				curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5_HOSTNAME);
			else {
				curl_easy_cleanup(curl);
				throw boss_error() << err_detail("Invalid proxy type specified.");
			}
		}

		//Get revision number from http://code.google.com/p/better-oblivion-sorting-software/source/browse/#svn page text.
		curl_easy_setopt(curl, CURLOPT_URL, revision_url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writer);	
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer );
		ret = curl_easy_perform(curl);
		if (ret!=CURLE_OK) {
			curl_easy_cleanup(curl);
			throw boss_error() << err_detail(errbuff);
		}
		
		//Extract revision number from page text.
		if (game == 1) start = buffer.find("boss-oblivion");
		else if (game == 2) start = buffer.find("boss-fallout");
		else if (game == 3) start = buffer.find("boss-nehrim");
		else if (game == 4) start = buffer.find("boss-fallout-nv");
		else if (game == 5) start = buffer.find("boss-skyrim");
		else {
			curl_easy_cleanup(curl);
			throw boss_error() << err_detail("None of the supported games were detected.");
		}
		if (start == string::npos) {
			curl_easy_cleanup(curl);
			throw boss_error() << err_detail("Cannot find online masterlist revision number.");
		}
		start = buffer.find("\"masterlist.txt\"", start);
		if (start == string::npos) {
			curl_easy_cleanup(curl);
			throw boss_error() << err_detail("Cannot find online masterlist revision number.");
		}
		start = buffer.find("B\",\"", start) + 4; 
		if (start == string::npos) {
			curl_easy_cleanup(curl);
			throw boss_error() << err_detail("Cannot find online masterlist revision number.");
		}
		end = buffer.find("\"",start) - start;
		if (end == string::npos) {
			curl_easy_cleanup(curl);
			throw boss_error() << err_detail("Cannot find online masterlist revision number.");
		}
		revision = buffer.substr(start,end);

		//Extract revision date from page text.
		string date;
		if (game == 1) start = buffer.find("boss-oblivion");
		else if (game == 2) start = buffer.find("boss-fallout");
		else if (game == 3) start = buffer.find("boss-nehrim");
		else if (game == 4) start = buffer.find("boss-fallout-nv");
		else if (game == 4) start = buffer.find("boss-skyrim");
		if (start == string::npos) {
			curl_easy_cleanup(curl);
			throw boss_error() << err_detail("Cannot find online masterlist revision date.");
		}
		start = buffer.find("\"masterlist.txt\"", start) + 1;
		if (start == string::npos) {
			curl_easy_cleanup(curl);
			throw boss_error() << err_detail("Cannot find online masterlist revision date.");
		}
		//There are 5 quote marks between the m in masterlist.txt and quote mark at the start of the date. 
		//Run through them and record the sixth.
		for (size_t i=0; i<6; i++)  {
			start = buffer.find("\"", start+1); 
			if (start == string::npos) {
				curl_easy_cleanup(curl);
				throw boss_error() << err_detail("Cannot find online masterlist revision date.");
			}
		}  
		//Now start is the first character of the date string.
		end = buffer.find("\"",start+1);  //end is the position of the first character after the date string.
		if (end == string::npos) {
			curl_easy_cleanup(curl);
			throw boss_error() << err_detail("Cannot find online masterlist revision date.");
		}
		date = buffer.substr(start+1,end - (start+1));  //Date string recorded.
		buffer.clear();

		//Now we need to get the date string extracted turned into a sensible absolute date.
		if (date.find("Today") != string::npos) {
			const time_t currTime = time(NULL);
			char * time = ctime(&currTime);
			date = time;
			string monthday = date.substr(4,6);
			string year = date.substr(date.length()-5,4);
			date = monthday + ", " + year;
		} else if (date.find("Yesterday") != string::npos) {
			//Same as for Today, but we need to turn back one day.
			time_t currTime = time(NULL);
			currTime = currTime - 86400;  //Time travel!
			char * time = ctime(&currTime);
			date = time;
			string monthday = date.substr(4,6);
			string year = date.substr(date.length()-5,4);
			date = monthday + ", " + year;
		} else if (date.find("(") != string::npos) {
			//Need to go from "Month Day (...)" to "Month Day, Year".
			//Remove everything from the space before the bracket onwards.
			start = date.find("(");
			date.resize(start);
			//Now get year from system and append.
			const time_t currTime = time(NULL);
			char * time = ctime(&currTime);
			string year = time;
			date += ", " + year.substr(year.length()-5,4);
		} //Otherwise it's already in a sensible format.
		
		newline = "? Masterlist Revision: "+revision+" ("+date+")";

		//Compare remote revision to current masterlist revision - if identical don't waste time/bandwidth updating it.
		if (GetLocalMasterlistRevision() == atoi(revision.c_str()))
			return 0;
		
		if (fs::exists(masterlist_path)) {
			mlist.open(masterlist_path.c_str());
			if (mlist.fail()) {
				curl_easy_cleanup(curl);
				throw boss_error() << err_detail("Masterlist cannot be opened.");
			}
			while (!mlist.eof()) {
				mlist.getline(cbuffer,4096);
				line=cbuffer;
				if (line.find("? Masterlist") != string::npos) {
					if (line.find(newline) != string::npos) {
						curl_easy_cleanup(curl);
						return 0;  //Masterlist already at latest revision.
					} else break;
				}
			}
			mlist.close();
		}

		// Use curl to get latest masterlist file from SVN repository
		//Change url and data structure settings, writefunction setting is retained.
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
		curl_easy_setopt(curl, CURLOPT_CRLF, 1);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, &progress_func);
		ret = curl_easy_perform(curl);
		if (ret!=CURLE_OK) {
			curl_easy_cleanup(curl);
			throw boss_error() << err_detail(errbuff);
		}
		cout << endl;

		//Clean up and close curl handle now that it's finished with.
		curl_easy_cleanup(curl);

		//Replace SVN keywords with revision number and replace current masterlist, or write a new one if it doesn't already exist.
		size_t pos = buffer.find(oldline);
		if (pos != string::npos)
			buffer.replace(pos,oldline.length(),newline);
		out.open(masterlist_path.c_str(), ios_base::trunc);
		if (out.fail())
			throw boss_error() << err_detail("Masterlist cannot be opened.");
		out << buffer;
		out.close();

		//Return revision number.
		return atoi(revision.c_str());
	}

	bool CheckConnection() {
		CURL *curl;									//cURL handle
		char errbuff[CURL_ERROR_SIZE];
		CURLcode ret;
		string proxy_str;
		const char *url = "http://code.google.com/p/better-oblivion-sorting-software/";

		//curl will be used to get stuff from the internet, so initialise it.
		curl = curl_easy_init();
		if (!curl)
			throw boss_error() << err_detail("Curl could not be initialised.");
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuff);	//Set error buffer for curl.
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 20);		//Set connection timeout to 20s.
		curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

		//Set up proxy stuff.
		if (proxy_type != "direct" && proxy_host != "none" && proxy_port != "0") {
			//All of the settings have potentially valid proxy-ing values.
			proxy_str = proxy_host + ":" + proxy_port;
			ret = curl_easy_setopt(curl, CURLOPT_PROXY, proxy_str.c_str());
			if (ret!=CURLE_OK) {
				curl_easy_cleanup(curl);
				throw boss_error() << err_detail("Invalid proxy hostname or port specified.");
			}

			if (proxy_type == "http")
				curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
			else if (proxy_type == "http1_0")
				curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP_1_0);
			else if (proxy_type == "socks4")
				curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS4);
			else if (proxy_type == "socks4a")
				curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS4A);
			else if (proxy_type == "socks5")
				curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);
			else if (proxy_type == "socks5h")
				curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5_HOSTNAME);
			else {
				curl_easy_cleanup(curl);
				throw boss_error() << err_detail("Invalid proxy type specified.");
			}
		}

		//Check that there is an internet connection. Easiest way to do this is to check that the BOSS google code page exists.
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 1);	
		ret = curl_easy_perform(curl);
		//Clean up and close curl handle now that it's finished with.
		curl_easy_cleanup(curl);

		if (ret!=CURLE_OK)
			throw boss_error() << err_detail(errbuff);
		else
			return true;
	}
}