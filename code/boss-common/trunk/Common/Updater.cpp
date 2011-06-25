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
#include <curl/types.h>
#include <curl/easy.h>
#include "Error.h"
#include "proxy.h"

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

	unsigned int UpdateMasterlist(int game) {
		const char *url;							//Masterlist file url
		char cbuffer[4096];
		char errbuff[CURL_ERROR_SIZE];
		CURL *curl;									//cURL handle
		string buffer,revision,newline,line;		//A bunch of strings.
		size_t start=-1,end;								//Position holders for trimming strings.
		CURLcode ret;
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
		else url = "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-fallout-nv/masterlist.txt";
		
		//curl will be used to get stuff from the internet, so initialise it.
		curl = curl_easy_init();
		if (!curl) 
			throw boss_error() << err_detail("Curl could not be initialised.");
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuff);	//Set error buffer for curl.
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 20);		//Set connection timeout to 10s.

		//Now set up proxy stuff.
		pxProxyFactory *pf = px_proxy_factory_new();
		if (!pf) {
			curl_easy_cleanup(curl);
			throw boss_error() << err_detail("Proxy support could not be initialised.");
		}
		//Find out which proxies must be used to get a connection.
		char **proxies = px_proxy_factory_get_proxies(pf, "http://code.google.com/p/better-oblivion-sorting-software/source/browse/#svn");

		//Now we iterate through the proxies when actually getting the data. We should do this every time we need to download,
		//but we can just do it for the first bit. If we come across a proxy that works, we then stop the loop.

		//Get revision number from http://code.google.com/p/better-oblivion-sorting-software/source/browse/#svn page text.
		//First set the constant settings.
		curl_easy_setopt(curl, CURLOPT_URL, "http://code.google.com/p/better-oblivion-sorting-software/source/browse/#svn");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);	
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer );
		for (int i=0;proxies[i];i++) {
			//Set up proxy.
			curl_easy_setopt(curl, CURLOPT_PROXY, proxies[i]);
			/*
			//Check if it's an HTML proxy.
			if (!strncmp("http", proxies[i], 4))
				curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
			//Check if it's a SOCKS proxy.
			else if (!strncmp("socks4", proxies[i], 6))
				curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS4);
			else if (!strncmp("socks5", proxies[i], 6))
				curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);
			else if (!strncmp("socks", proxies[i], 5))
				curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);
				*/
			//Try getting the page.
			ret = curl_easy_perform(curl);

			//Exit loop if it works to ensure the correct setting is retained.
			if (ret == CURLE_OK)
				break;
		}
		//If after trying all proxies, things still didn't work, quit.
		if (ret!=CURLE_OK) {
			curl_easy_cleanup(curl);
			px_proxy_factory_free(pf);
			throw boss_error() << err_detail(errbuff);
		}
		
		//Extract revision number from page text.
		if (game == 1) start = buffer.find("boss-oblivion");
		else if (game == 2) start = buffer.find("boss-fallout");
		else if (game == 3) start = buffer.find("boss-nehrim");
		else if (game == 4) start = buffer.find("boss-fallout-nv");
		if (start == string::npos) {
			curl_easy_cleanup(curl);
			px_proxy_factory_free(pf);
			throw boss_error() << err_detail("Cannot find online masterlist revision number.");
		}
		start = buffer.find("\"masterlist.txt\"", start);
		start = buffer.find("B\",\"", start) + 4; 
		if (start == string::npos) {
			curl_easy_cleanup(curl);
			px_proxy_factory_free(pf);
			throw boss_error() << err_detail("Cannot find online masterlist revision number.");
		}
		end = buffer.find("\"",start) - start;
		if (end == string::npos) {
			curl_easy_cleanup(curl);
			px_proxy_factory_free(pf);
			throw boss_error() << err_detail("Cannot find online masterlist revision number.");
		}
		revision = buffer.substr(start,end);
		//buffer.clear();

		//Extract revision date from page text.
		string date;
		if (game == 1) start = buffer.find("boss-oblivion");
		else if (game == 2) start = buffer.find("boss-fallout");
		else if (game == 3) start = buffer.find("boss-nehrim");
		else if (game == 4) start = buffer.find("boss-fallout-nv");
		if (start == string::npos) {
			curl_easy_cleanup(curl);
			px_proxy_factory_free(pf);
			throw boss_error() << err_detail("Cannot find online masterlist revision date.");
		}
		start = buffer.find("\"masterlist.txt\"", start) + 1;
		if (start == string::npos) {
			curl_easy_cleanup(curl);
			px_proxy_factory_free(pf);
			throw boss_error() << err_detail("Cannot find online masterlist revision date.");
		}
		//There are 5 quote marks between the m in masterlist.txt and quote mark at the start of the date. 
		//Run through them and record the sixth.
		for (size_t i=0; i<6; i++)  {
			start = buffer.find("\"", start+1); 
			if (start == string::npos) {
				curl_easy_cleanup(curl);
				px_proxy_factory_free(pf);
				throw boss_error() << err_detail("Cannot find online masterlist revision date.");
			}
		}  
		//Now start is the first character of the date string.
		end = buffer.find("\"",start+1);  //end is the position of the first character after the date string.
		if (end == string::npos) {
			curl_easy_cleanup(curl);
			px_proxy_factory_free(pf);
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
		if (fs::exists(masterlist_path)) {
			mlist.open(masterlist_path.c_str());
			if (mlist.fail()) {
				curl_easy_cleanup(curl);
				px_proxy_factory_free(pf);
				throw boss_error() << err_detail("Masterlist cannot be opened.");
			}
			while (!mlist.eof()) {
				mlist.getline(cbuffer,4096);
				line=cbuffer;
				if (line.find("? Masterlist") != string::npos) {
					if (line.find(newline) != string::npos) {
						curl_easy_cleanup(curl);
						px_proxy_factory_free(pf);
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
		ret = curl_easy_perform(curl);
		if (ret!=CURLE_OK) {
			curl_easy_cleanup(curl);
			px_proxy_factory_free(pf);
			throw boss_error() << err_detail(errbuff);
		}

		//Clean up and close curl handle now that it's finished with. Also free proxy resources.
		curl_easy_cleanup(curl);
		px_proxy_factory_free(pf);

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
}