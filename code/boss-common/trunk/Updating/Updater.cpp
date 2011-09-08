/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

	Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
	http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 3188 $, $Date: 2011-08-27 08:16:41 +0100 (Sat, 27 Aug 2011) $
*/

#include "Updating/Updater.h"
#include "Common/Globals.h"
#include "Support/Helpers.h"
#include "Output/Output.h"
#include "Common/Error.h"

#include <boost/filesystem.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <stdio.h>
#include <iostream>
#include <fstream>

#ifndef CURL_STATICLIB
#define CURL_STATICLIB			//Tells the compiler to use curl as a static library.
#endif
#include <curl/curl.h>
#include <curl/easy.h>

//#include <wx/msgdlg.h>
//#include <wx/progdlg.h>

BOOST_FUSION_ADAPT_STRUCT(
    boss::fileInfo,
	(std::string, name)
    (unsigned int, crc)
)

namespace boss {
	namespace fs = boost::filesystem;
	namespace unicode = boost::spirit::unicode;
	namespace qi = boost::spirit::qi;
	using namespace std;

	using boost::algorithm::replace_all;

	string updateVersion;
	vector<fileInfo> updatedFiles;  //The updated files. These don't have the .new extension.
	int ans;						//The answer to the cancel download confirmation message. 
									//Needs to be stored externally because the function the message is in repeats often.

	//Buffer writer for downloaders.
	int writer(char *data, size_t size, size_t nmemb, void *buffer){
		string *str = (string*)buffer;
		if(str != NULL) {
			str -> append(data, size * nmemb);
			return size * nmemb;
		}
		return 0;
	} 

	//Download progress for masterlist downloader function.
	int mlist_progress_func(void *data, double dlTotal, double dlNow, double ulTotal, double ulNow) {
		double fractiondownloaded = dlNow / dlTotal;
		if (dlTotal <= 0 || dlNow <= 0)
			fractiondownloaded = 0;
		printf("%3.0f%% of %3.0f% KB\r",fractiondownloaded*100,(dlTotal/1024)+20);  //The +20 is there because for some reason there's always a 20kb difference between reported size and Windows' size.
		fflush(stdout);
		return 0;
	}

	//Download progress for BOSS update downloader function.
/*	int prog_progress_func(void *data, double dlTotal, double dlNow, double ulTotal, double ulNow) {
		double percentdownloaded = (dlNow / dlTotal) * 1000;
		int currentProgress = floor(percentdownloaded);
		if (currentProgress == 1000)
			--currentProgress; //Stop the progress bar from closing before all files are downloaded.
		
		wxProgressDialog* progDia = (wxProgressDialog*)data;
		bool cont = progDia->Update(currentProgress);
		if (!cont) {  //the user decided to cancel. Slightly temperamental, the progDia seems to hang a little sometimes and keypresses don't get registered. Can't do much about that.
			if (!ans)
				ans = wxMessageBox(wxT("Are you sure you want to cancel?"), wxT("BOSS GUI: Automatic Updater"), wxYES_NO | wxICON_EXCLAMATION, progDia);
			if (ans == wxYES)
				return 1;
			progDia->Resume();
			ans = NULL;
		}
		return 0;
	}
*/
	//Checks if an Internet connection is present.
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

		if (ret!=CURLE_OK) {
			throw boss_error() << err_detail(errbuff);
		} else {
			//Clean up and close curl handle now that it's finished with.
			curl_easy_cleanup(curl);
			return true;
		}
	}

	//Gets the revision number of the local masterlist.
	int GetLocalMasterlistRevision() {
		string line, newline = "Masterlist Revision:";
		ifstream mlist;
		char cbuffer[4096];
		size_t pos1,pos2;

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
					pos1 = line.find(": ");
					pos2 = line.find(" (");
					return atoi(line.substr(pos1+2,pos2-pos1-2).c_str());
				}
			}
		}
		mlist.close();
		return 0;  //No version found.
	}

	//Gets the revision date of the local masterlist.
	string GetLocalMasterlistDate() {
		string line, newline = "Masterlist Revision:";
		ifstream mlist;
		char cbuffer[4096];
		size_t pos1,pos2;

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
					pos1 = line.find("(");
					pos2 = line.find(")");
					return line.substr(pos1+1,pos2-pos1-1);
				}
			}
		}
		mlist.close();
		return 0;  //No version found.
	}

	//Updates the local masterlist to the latest available online.
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
		string oldline = "Masterlist Information: "+SVN_REVISION_KW+", "+SVN_DATE_KW+", "+SVN_CHANGEDBY_KW;
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

		//Month Day, Year
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
		
		newline = "Masterlist Revision: "+revision+" ("+date+")";

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
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, &mlist_progress_func);
		ret = curl_easy_perform(curl);
		if (ret!=CURLE_OK) {
			curl_easy_cleanup(curl);
			throw boss_error() << err_detail(errbuff);
		}

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

	//Checks if a new release of BOSS is available or not.
	string IsBOSSUpdateAvailable() {
		string remoteVersionStr, proxy_str;
		char errbuff[CURL_ERROR_SIZE];
		CURL *curl;									//cURL handle
		CURLcode ret;
		const char *url = "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-common/latestversion.txt";

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

		//Get page containing version number.
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writer);	
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &remoteVersionStr);
		ret = curl_easy_perform(curl);
		if (ret!=CURLE_OK) {
			curl_easy_cleanup(curl);
			throw boss_error() << err_detail(errbuff);
		}

		//Now compare versions.
		if (remoteVersionStr.compare(g_version) > 0) {
			updateVersion == remoteVersionStr;
			return remoteVersionStr;
		} else
			return "";
	}

	//Download the files in the update.
/*	void DownloadBOSSUpdateFiles(int updateType, wxProgressDialog *progDia) {
		string fileBuffer, message, proxy_str, remote_file;
		string url = "http://better-oblivion-sorting-software.googlecode.com/svn/releases/"+updateVersion+"/";
		char errbuff[CURL_ERROR_SIZE];
		CURL *curl;									//cURL handle
		CURLcode ret;
		unsigned int calcedCRC;
		
		//First decide what type of install we're updating.
		if (updateType == MANUAL)
			url += "manual/";

		//Need to reset updatedFiles because it might have been set already if the updater was run then cancelled.
		updatedFiles.clear();
		ans = 0;

		//curl will be used to get stuff from the internet, so initialise it.
		curl = curl_easy_init();
		if (!curl) {
			throw boss_error() << err_detail("Curl could not be initialised.");
		}
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

		message = "Fetching file information...";
		progDia->Update(0,message);

		//First get file list and crcs to build updatedFiles vector.
		remote_file = url+"checksums.txt";
		curl_easy_setopt(curl, CURLOPT_URL, remote_file.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writer);	
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &fileBuffer);
		ret = curl_easy_perform(curl);
		if (ret!=CURLE_OK) {
			curl_easy_cleanup(curl);
			throw boss_error() << err_detail(errbuff);
		}

		//Now parse list to extract file info.
		string::const_iterator start = fileBuffer.begin(), end = fileBuffer.end();
		bool p = qi::phrase_parse(start,end,
			(('"' >> qi::lexeme[+(unicode::char_ - '"')] >> '"' >> qi::lit(":") >> qi::hex - qi::eol) | qi::eoi) % qi::eol,
			unicode::space - qi::eol, updatedFiles);
		if (!p || start != end) {
			curl_easy_cleanup(curl);
			throw boss_error() << err_detail("Could not read remote file information.");
		}

		//Now that we've got a vector of the files, we can download them.
		//Loop through the vector and download and save each file. Use binary streams.
		for (size_t i=0;i<updatedFiles.size();i++) {
			if (updatedFiles[i].name.empty())  //Just in case.
				continue;
			fileBuffer.clear();  //Empty buffer ready for next download.
			//Set up progress info. Since we're not doing a total download progress bar, zero progress for each file.
			message = "Downloading: " + updatedFiles[i].name + " (" + IntToString(i+1) + " of " + IntToString(updatedFiles.size()) + ")";
			progDia->Update(0,message);

			string path = updatedFiles[i].name + ".new";
			ofstream ofile(path.c_str(),ios_base::binary|ios_base::out|ios_base::trunc);
			if (ofile.fail()) {
				curl_easy_cleanup(curl);
				throw boss_error() << err_detail("Could not save \""+path+"\"");
			}

			remote_file = url+updatedFiles[i].name;
			boost::replace_all(remote_file," ","%20");  //Need to put the %20s back in for the file's web address.

			curl_easy_setopt(curl, CURLOPT_URL, remote_file.c_str());
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
			curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, &prog_progress_func);
			curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, progDia);
			ret = curl_easy_perform(curl);
			if (ret == CURLE_ABORTED_BY_CALLBACK) {
				//Cancelled by user.
				curl_easy_cleanup(curl);
				throw boss_error() << err_detail("Cancelled by user.");
			} else if (ret!=CURLE_OK) {
				curl_easy_cleanup(curl);
				throw boss_error() << err_detail(errbuff);
			}
			ofile << fileBuffer;
			ofile.close();

			//Now verify file integrity.
			calcedCRC = GetCrc32(fs::path(path));
			if (calcedCRC != updatedFiles[i].crc) {
				throw boss_error() << err_detail("Downloaded file \""+updatedFiles[i].name+"\" failed verification test. Please try updating again.");
			}
		}
		curl_easy_cleanup(curl);
		progDia->Update(1000);
	}
*/
	//Installs the downloaded update files.
	void InstallBOSSUpdateFiles() {
		//First back up current BOSS.ini if it exists.
		if (fs::exists("BOSS.ini"))
				fs::rename("BOSS.ini","BOSS.ini.old");

		//Now iterate through the vector of updated files.
		//Delete the current file if it exists, and rename the downloaded updated file, removing the .new extension. Don't try updating BOSS GUI.exe.
		for (size_t i=0;i<updatedFiles.size();i++) {
			string old = updatedFiles[i].name;
			string updated = updatedFiles[i].name + ".new";

			if (old != "BOSS GUI.exe") {
				if (fs::exists(old))
					fs::remove(old);

				fs::rename(updated,old);
			}
		}
	}

	//Cleans up after the user cancels a download.
	void CleanUp() {
		//Iterate through vector of updated files. Delete any that exist locally.
		for (size_t i=0;i<updatedFiles.size();i++) {
			string file = updatedFiles[i].name + ".new";
			if (fs::exists(file))
				fs::remove(file);
		}
	}
}