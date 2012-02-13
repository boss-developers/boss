/*	Better Oblivion Sorting Software
	
	A "one-click" program for users that quickly optimises and avoids 
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge, 
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

    Copyright (C) 2009-2012    BOSS Development Team.

	This file is part of Better Oblivion Sorting Software.

    Better Oblivion Sorting Software is free software: you can redistribute 
	it and/or modify it under the terms of the GNU General Public License 
	as published by the Free Software Foundation, either version 3 of 
	the License, or (at your option) any later version.

    Better Oblivion Sorting Software is distributed in the hope that it will 
	be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Better Oblivion Sorting Software.  If not, see 
	<http://www.gnu.org/licenses/>.

	$Revision: 3188 $, $Date: 2011-08-27 08:16:41 +0100 (Sat, 27 Aug 2011) $
*/

#include "Updating/Updater.h"
#include "Common/Globals.h"
#include "Support/Helpers.h"
#include "Common/Error.h"
#include "Support/Logger.h"

#include <boost/filesystem.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

#include <stdio.h>
#include <iostream>
#include <fstream>

#ifndef CURL_STATICLIB
#define CURL_STATICLIB			//Tells the compiler to use curl as a static library.
#endif
#include <curl/curl.h>
#include <curl/easy.h>

//#ifdef BOSSGUI
#include <wx/msgdlg.h>
#include <wx/progdlg.h>
//#endif

namespace boss {
	namespace fs = boost::filesystem;
	namespace unicode = boost::spirit::unicode;
	namespace qi = boost::spirit::qi;
	namespace phoenix = boost::phoenix;
	using namespace std;

	using boost::algorithm::replace_all;

	//////////////////////////////////////
	// Struct Contstructors / Variables
	//////////////////////////////////////

	BOSS_COMMON uiStruct::uiStruct() {
		p = NULL;
		file = "";
	}

	BOSS_COMMON uiStruct::uiStruct(void *GUIpoint) {
		p = GUIpoint;
		file = "";
	}

	////////////////////////
	// Internal Functions
	////////////////////////

	//Download progress for downloader functions.
	int progress_func(void *data, double dlTotal, double dlNow, double ulTotal, double ulNow) {
		double fractiondownloaded = dlNow / dlTotal;
		if (dlTotal <= 0 || dlNow <= 0)
			fractiondownloaded = 0.0f;

		uiStruct * ui = (uiStruct*)data;
		if (ui->p != NULL) {
			int currentProgress = (int)floor(fractiondownloaded * 1000);
			if (currentProgress == 1000)
				--currentProgress; //Stop the progress bar from closing in case of multiple downloads.
			wxProgressDialog* progress = (wxProgressDialog*)ui->p;
			bool cont = progress->Update(currentProgress, "Downloading: " + ui->file);
			if (!cont) {  //the user decided to cancel. Slightly temperamental, the progDia seems to hang a little sometimes and keypresses don't get registered. Can't do much about that.
				uint32_t ans = wxMessageBox(wxT("Are you sure you want to cancel?"), wxT("BOSS: Updater"), wxYES_NO | wxICON_EXCLAMATION, progress);
				if (ans == wxYES)
					return 1;
				progress->Resume();
				ans = NULL;
			}
		} else {
			printf("Downloading: %s; %3.0f%% of %3.0f KB\r", ui->file.c_str(), fractiondownloaded*100, (dlTotal/1024)+20);  //The +20 is there because for some reason there's always a 20kb difference between reported size and Windows' size.
			fflush(stdout);
		}
		return 0;
	}

	//Buffer writer for downloaders.
	size_t writer(char * data, size_t size, size_t nmemb, void * buffer) {
		string *str = (string*)buffer;
		if(str != NULL) {
			str -> append(data, size * nmemb);
			return size * nmemb;
		}
		return 0;
	}

	//Initialise a curl handle. Throws exception on error.
	CURL * InitCurl(char * errbuff) {
		CURLcode ret;
		string proxy_str;
		CURL *curl;

		curl = curl_easy_init();
		if (!curl)
			throw boss_error(BOSS_ERROR_CURL_INIT_FAIL);

		ret = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuff);	//Set error buffer for curl.
		if (ret != CURLE_OK) {
			curl_easy_cleanup(curl);
			throw boss_error(BOSS_ERROR_CURL_SET_ERRBUFF_FAIL);
		}
		ret = curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 20);		//Set connection timeout to 20s.
		if (ret != CURLE_OK) {
			string err = errbuff;
			curl_easy_cleanup(curl);
			throw boss_error(err, BOSS_ERROR_CURL_SET_OPTION_FAIL);
		}
		ret = curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1);
		if (ret != CURLE_OK) {
			string err = errbuff;
			curl_easy_cleanup(curl);
			throw boss_error(err, BOSS_ERROR_CURL_SET_OPTION_FAIL);
		}
		ret = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
		if (ret != CURLE_OK) {
			string err = errbuff;
			curl_easy_cleanup(curl);
			throw boss_error(err, BOSS_ERROR_CURL_SET_OPTION_FAIL);
		}

		if (gl_proxy_host != "none" && gl_proxy_port != 0) {
			//All of the settings have potentially valid proxy-ing values.
			ret = curl_easy_setopt(curl, CURLOPT_PROXYTYPE, 
										CURLPROXY_HTTP|
										CURLPROXY_HTTP_1_0|
										CURLPROXY_SOCKS4|
										CURLPROXY_SOCKS4A|
										CURLPROXY_SOCKS5|
										CURLPROXY_SOCKS5_HOSTNAME);
			if (ret != CURLE_OK) {
				string err = errbuff;
				curl_easy_cleanup(curl);
				throw boss_error(err, BOSS_ERROR_CURL_SET_PROXY_TYPE_FAIL);
			}

			proxy_str = gl_proxy_host + ":" + IntToString(gl_proxy_port);
			ret = curl_easy_setopt(curl, CURLOPT_PROXY, proxy_str.c_str());
			if (ret!=CURLE_OK) {
				string err = errbuff;
				curl_easy_cleanup(curl);
				throw boss_error(err, BOSS_ERROR_CURL_SET_PROXY_FAIL);
			}

			if (!gl_proxy_user.empty() && !gl_proxy_passwd.empty()) {
				ret = curl_easy_setopt(curl, CURLOPT_PROXYAUTH, CURLAUTH_BASIC|
																CURLAUTH_DIGEST|
																CURLAUTH_NTLM);
				if (ret != CURLE_OK) {
					string err = errbuff;
					curl_easy_cleanup(curl);
					throw boss_error(err, BOSS_ERROR_CURL_SET_PROXY_AUTH_TYPE_FAIL);
				}

				string proxy_auth = gl_proxy_user + ":" + gl_proxy_passwd;
				ret = curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, proxy_auth.c_str());
				if (ret != CURLE_OK) {
					string err = errbuff;
					curl_easy_cleanup(curl);
					throw boss_error(err, BOSS_ERROR_CURL_SET_PROXY_AUTH_FAIL);
				}
			}
		}
		return curl;
	}

	//Download the remote file to local. Throws exception on error.
	void DownloadFile(uiStruct ui, const string remote, const string local) {
		string fileBuffer;
		char errbuff[CURL_ERROR_SIZE];
		CURL *curl;									//cURL handle
		CURLcode ret;
		ofstream ofile;

		//curl will be used to get stuff from the internet, so initialise it.
		curl = InitCurl(errbuff);

		//Open local file.
		ofile.open(local.c_str(), ios_base::binary|ios_base::trunc);  //Masterlist doesn't have binary flag, does this break if included?
		if (ofile.fail()) {
			curl_easy_cleanup(curl);
			throw boss_error(BOSS_ERROR_FILE_WRITE_FAIL, local);
		}
		
		//Download to buffer.
		curl_easy_setopt(curl, CURLOPT_CRLF, 1);  //This was set for masterlist only.
		curl_easy_setopt(curl, CURLOPT_URL, remote.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writer);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &fileBuffer);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, &progress_func);
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &ui);
		ret = curl_easy_perform(curl);
		if (ret == CURLE_ABORTED_BY_CALLBACK) {  //Cancelled by user.
			curl_easy_cleanup(curl);
			throw boss_error(BOSS_ERROR_CURL_USER_CANCEL);
		} else if (ret!=CURLE_OK) {
			string err = errbuff;
			curl_easy_cleanup(curl);
			throw boss_error(err, BOSS_ERROR_CURL_PERFORM_FAIL);
		}

		//Fill file with buffer.
		ofile << fileBuffer;
		ofile.close();

		curl_easy_cleanup(curl);
	}

	//Install file by renaming it. Throws exception on error.
	void InstallFile(string downloadedName, string installedName) {
		try {
			fs::rename(boss_path / downloadedName, boss_path / installedName);
		} catch (fs::filesystem_error e) {
			throw boss_error(BOSS_ERROR_FS_FILE_RENAME_FAIL, downloadedName, e.what());
		}
	}

	//Gets a filename and a crc from a text file containing a single line of the form:
	//"File" : "CRC"
	//Throws exception on error.
	void GetBOSSFileInfo(const string remoteInfoFile, string& file, uint32_t& crc) {
		string fileBuffer;
		char errbuff[CURL_ERROR_SIZE];
		CURL *curl;									//cURL handle
		CURLcode ret;

		//curl will be used to get stuff from the internet, so initialise it.
		curl = InitCurl(errbuff);

		//First get file list and crcs to build updatedFiles vector.
		curl_easy_setopt(curl, CURLOPT_URL, remoteInfoFile.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writer);	
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &fileBuffer);
		ret = curl_easy_perform(curl);
		if (ret!=CURLE_OK) {
			string err = errbuff;
			curl_easy_cleanup(curl);
			throw boss_error(err, BOSS_ERROR_CURL_PERFORM_FAIL);
		}
		curl_easy_cleanup(curl);

		//Now parse list to extract file info.
		string::const_iterator start = fileBuffer.begin(), end = fileBuffer.end();
		bool p = qi::phrase_parse(start,end,
			'"' 
			>> qi::lexeme[+(unicode::char_ - '"')[phoenix::ref(file) = qi::_1]]
			>> '"' 
			>> (
				(qi::lit(":") >> qi::hex - qi::eol)[phoenix::ref(crc) = qi::_1]
				| qi::eps[phoenix::ref(crc) = 0]
			)
			, unicode::space);

		if (!p || start != end) {
			throw boss_error(BOSS_ERROR_READ_UPDATE_FILE_LIST_FAIL);
		}
	}

	//Gets the revision number of the local masterlist. Throws exception on error.
	void GetLocalMasterlistRevisionDate(uint32_t& revision, string& date) {
		string line, newline = "Masterlist Revision:";
		ifstream mlist;
		char cbuffer[MAXLENGTH];
		size_t pos1,pos2,pos3;

		if (fs::exists(masterlist_path())) {
			mlist.open(masterlist_path().c_str());
			if (mlist.fail())
				throw boss_error(BOSS_ERROR_FILE_READ_FAIL, masterlist_path().string());
			while (!mlist.eof()) {
				mlist.getline(cbuffer,sizeof(cbuffer));
				line=cbuffer;
				if (line.find(newline) != string::npos) {
					//Hooray, we've found the line.
					pos1 = line.find(": ");
					pos2 = line.find(" (");
					revision = atoi(line.substr(pos1+2,pos2-pos1-2).c_str());
					pos3 = line.find(")");
					date = line.substr(pos2+2,pos3-pos2-2);
					return;
				}
			}
			mlist.close();
		}
		revision = 0;
		date.clear();
		return;  //No version found.
	}

	//Gets the revision number of the online masterlist. Throws exception on error.
	void GetRemoteMasterlistRevisionDate(uint32_t& revision, string& date) {
		char errbuff[CURL_ERROR_SIZE];
		CURL *curl;									//cURL handle
		string buffer;		//A bunch of strings.
		size_t start,end;								//Position holders for trimming strings.
		CURLcode ret;
		const string SVN_REVISION_KW = "$" "Revision" "$";                   // Left as separated parts to avoid keyword expansion
		const string SVN_DATE_KW = "$" "Date" "$";                           // Left as separated parts to avoid keyword expansion
		const string SVN_CHANGEDBY_KW= "$" "LastChangedBy" "$";              // Left as separated parts to avoid keyword expansion
		string oldline = "Masterlist Information: "+SVN_REVISION_KW+", "+SVN_DATE_KW+", "+SVN_CHANGEDBY_KW;
		const char *revision_url = "http://code.google.com/p/better-oblivion-sorting-software/source/browse/#svn";

		//curl will be used to get stuff from the internet, so initialise it.
		curl = InitCurl(errbuff);

		//Get revision number and date from http://code.google.com/p/better-oblivion-sorting-software/source/browse/#svn page text.
		curl_easy_setopt(curl, CURLOPT_URL, revision_url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writer);	
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer );
		ret = curl_easy_perform(curl);
		if (ret!=CURLE_OK) {
			string err = errbuff;
			curl_easy_cleanup(curl);
			throw boss_error(err, BOSS_ERROR_CURL_PERFORM_FAIL);
		}
		
		//Extract revision number from page text.
		switch (gl_current_game) {
		case OBLIVION:
			start = buffer.find("\"boss-oblivion\":");
			break;
		case NEHRIM:
			start = buffer.find("\"boss-nehrim\":");
			break;
		case SKYRIM:
			start = buffer.find("\"boss-skyrim\":");
			break;
		case FALLOUT3:
			start = buffer.find("\"boss-fallout\":");
			break;
		case FALLOUTNV:
			start = buffer.find("\"boss-fallout-nv\":");
			break;
		default:
			curl_easy_cleanup(curl);
			throw boss_error(BOSS_ERROR_NO_GAME_DETECTED);
		}
		if (start == string::npos) {
			curl_easy_cleanup(curl);
			throw boss_error(BOSS_ERROR_FIND_ONLINE_MASTERLIST_REVISION_FAIL);
		}
		start = buffer.find("\"masterlist.txt\"", start);
		if (start == string::npos) {
			curl_easy_cleanup(curl);
			throw boss_error(BOSS_ERROR_FIND_ONLINE_MASTERLIST_REVISION_FAIL);
		}
		start = buffer.find("B\",\"", start) + 4; 
		if (start == string::npos) {
			curl_easy_cleanup(curl);
			throw boss_error(BOSS_ERROR_FIND_ONLINE_MASTERLIST_REVISION_FAIL);
		}
		end = buffer.find("\"",start) - start;
		if (end == string::npos) {
			curl_easy_cleanup(curl);
			throw boss_error(BOSS_ERROR_FIND_ONLINE_MASTERLIST_REVISION_FAIL);
		}
		revision = atoi(buffer.substr(start,end).c_str());

		//Extract revision date from page text.
		switch (gl_current_game) {
		case OBLIVION:
			start = buffer.find("\"boss-oblivion\":");
			break;
		case NEHRIM:
			start = buffer.find("\"boss-nehrim\":");
			break;
		case SKYRIM:
			start = buffer.find("\"boss-skyrim\":");
			break;
		case FALLOUT3:
			start = buffer.find("\"boss-fallout\":");
			break;
		case FALLOUTNV:
			start = buffer.find("\"boss-fallout-nv\":");
			break;
		default:
			curl_easy_cleanup(curl);
			throw boss_error(BOSS_ERROR_NO_GAME_DETECTED);
		}
		if (start == string::npos) {
			curl_easy_cleanup(curl);
			throw boss_error(BOSS_ERROR_FIND_ONLINE_MASTERLIST_DATE_FAIL);
		}
		start = buffer.find("\"masterlist.txt\"", start) + 1;
		if (start == string::npos) {
			curl_easy_cleanup(curl);
			throw boss_error(BOSS_ERROR_FIND_ONLINE_MASTERLIST_DATE_FAIL);
		}
		//There are 5 quote marks between the m in masterlist.txt and quote mark at the start of the date. 
		//Run through them and record the sixth.
		for (size_t i=0; i<6; i++)  {
			start = buffer.find("\"", start+1); 
			if (start == string::npos) {
				curl_easy_cleanup(curl);
			throw boss_error(BOSS_ERROR_FIND_ONLINE_MASTERLIST_DATE_FAIL);
			}
		}  
		//Now start is the first character of the date string.
		end = buffer.find("\"",start+1);  //end is the position of the first character after the date string.
		if (end == string::npos) {
			curl_easy_cleanup(curl);
			throw boss_error(BOSS_ERROR_FIND_ONLINE_MASTERLIST_DATE_FAIL);
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
	}


	////////////////////////
	// General Functions
	////////////////////////

	//Checks if an Internet connection is present.
	BOSS_COMMON bool CheckConnection() {
		CURL *curl;									//cURL handle
		char errbuff[CURL_ERROR_SIZE];
		CURLcode ret;
		string proxy_str;
		const char *url = "http://code.google.com/p/better-oblivion-sorting-software/";

		//curl will be used to get stuff from the internet, so initialise it.
		curl = InitCurl(errbuff);

		//Check that there is an internet connection. Easiest way to do this is to check that the BOSS google code page exists.
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 1);	
		ret = curl_easy_perform(curl);

		if (ret!=CURLE_OK) {
			LOG_INFO("Failed to find Internet connection: %s",errbuff);
			curl_easy_cleanup(curl);
			return false;
		} else {
			//Clean up and close curl handle now that it's finished with.
			curl_easy_cleanup(curl);
			return true;
		}
	}

	//Cleans up after the user cancels a download. Throws exception on error.
	BOSS_COMMON void CleanUp() {
		try {
			//Use a recursive directory iterator to find and delete and files with a ".new" extension.
			for (fs::directory_iterator itr(boss_path); itr!=fs::directory_iterator(); ++itr) {
				if (itr->path().extension().string() == ".new") {
					LOG_DEBUG("-- Deleting mod: '%s'", itr->path().string().c_str());
					try {
						fs::remove(itr->path());
					} catch (fs::filesystem_error e) {
						throw boss_error(BOSS_ERROR_FS_FILE_DELETE_FAIL, itr->path().string());
					}
				}
			}
		} catch (fs::filesystem_error e) {
			throw boss_error(BOSS_ERROR_FS_ITER_DIRECTORY_FAIL, boss_path.string());
		}
	}


	////////////////////////
	// Masterlist Updating
	////////////////////////

	//Updates the local masterlist to the latest available online. Throws exception on error.
	BOSS_COMMON void UpdateMasterlist(uiStruct ui, uint32_t& localRevision, string& localDate, uint32_t& remoteRevision, string& remoteDate) {							//cURL handle
		string url, buffer, newline;		//A bunch of strings.
		ifstream mlist;								//Input stream.
		ofstream out;								//Output stream.
		const string SVN_REVISION_KW = "$" "Revision" "$";                   // Left as separated parts to avoid keyword expansion
		const string SVN_DATE_KW = "$" "Date" "$";                           // Left as separated parts to avoid keyword expansion
		const string SVN_CHANGEDBY_KW= "$" "LastChangedBy" "$";              // Left as separated parts to avoid keyword expansion
		string oldline = "Masterlist Information: "+SVN_REVISION_KW+", "+SVN_DATE_KW+", "+SVN_CHANGEDBY_KW;

		//Get local and remote masterlist info.
		GetLocalMasterlistRevisionDate(localRevision, localDate);
		GetRemoteMasterlistRevisionDate(remoteRevision, remoteDate);

		//Is an update available?
		if (localRevision == 0 || localRevision < remoteRevision) {
			//Set url.
			switch (gl_current_game) {
			case OBLIVION:
				url = "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-oblivion/masterlist.txt";
				break;
			case NEHRIM:
				url = "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-nehrim/masterlist.txt";
				break;
			case SKYRIM:
				url = "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-skyrim/masterlist.txt";
				break;
			case FALLOUT3:
				url = "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-fallout/masterlist.txt";
				break;
			case FALLOUTNV:
				url = "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-fallout-nv/masterlist.txt";
				break;
			default:
				throw boss_error(BOSS_ERROR_NO_GAME_DETECTED);
			}

			//Set file.
			ui.file = masterlist_path().string();

			//Now download and install.
			DownloadFile(ui, url, masterlist_path().string() + ".new");
			InstallFile(masterlist_path().string() + ".new", masterlist_path().string());

			//Now replace the SVN info in the downloaded file with the revision and date.
			newline = "Masterlist Revision: "+IntToString(remoteRevision)+" ("+remoteDate+")";

			fileToBuffer(masterlist_path(), buffer);

			size_t pos = buffer.find(oldline);
			if (pos != string::npos)
				buffer.replace(pos,oldline.length(),newline);

			out.open(masterlist_path().c_str());
			if (out.fail())
				throw boss_error(BOSS_ERROR_FILE_WRITE_FAIL, masterlist_path().string());
			out << buffer;
			out.close();
		}
	}


	////////////////////////
	// BOSS Updating
	////////////////////////

	//Checks if a new release of BOSS is available or not. Throws exception on error.
	BOSS_COMMON string IsBOSSUpdateAvailable() {
		string ver, proxy_str;
		uint32_t majorV=0, minorV=0, patchV=0;
		char errbuff[CURL_ERROR_SIZE];
		CURL *curl;									//cURL handle
		CURLcode ret;
		const char *url = "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-common/latestversion.txt";

		//curl will be used to get stuff from the internet, so initialise it.
		curl = InitCurl(errbuff);

		//Get page containing version number.
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writer);	
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ver);
		ret = curl_easy_perform(curl);
		if (ret!=CURLE_OK) {
			string err = errbuff;
			curl_easy_cleanup(curl);
			throw boss_error(err, BOSS_ERROR_CURL_PERFORM_FAIL);
		}
		
		//Need to exract major, minor and patch versions from string.
		size_t pos1, pos2;
		pos1 = ver.find('.');
		if (pos1 != string::npos)
			majorV = atoi(ver.substr(0,pos1).c_str());
		if (pos1 + 1 != ver.length()) {
			pos2 = ver.find('.',pos1+1);
			if (pos2 != string::npos) {
				minorV = atoi(ver.substr(pos1+1,pos2).c_str());
				if (pos2 + 1 != ver.length())
					patchV = atoi(ver.substr(pos2+1).c_str());
			}
		}

		//Now compare versions.
		if (majorV > BOSS_VERSION_MAJOR || (majorV == BOSS_VERSION_MAJOR && minorV > BOSS_VERSION_MINOR) || (majorV == BOSS_VERSION_MAJOR && minorV == BOSS_VERSION_MINOR && patchV > BOSS_VERSION_PATCH))
			return ver;
		else
			return "";
	}

	//Gets the release notes for the update. Throws exception on error.
	string FetchReleaseNotes(const string updateVersion) {
		string url, fileBuffer;
		char errbuff[CURL_ERROR_SIZE];
		CURL *curl;									//cURL handle
		CURLcode ret;

		//Set release notes url.
		url = "http://better-oblivion-sorting-software.googlecode.com/svn/releases/"+updateVersion+"/releasenotes.txt";

		//curl will be used to get stuff from the internet, so initialise it.
		curl = InitCurl(errbuff);

		//Get release notes.
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writer);	
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &fileBuffer);
		ret = curl_easy_perform(curl);
		if (ret!=CURLE_OK) {
			string err = errbuff;
			curl_easy_cleanup(curl);
			throw boss_error(err, BOSS_ERROR_CURL_PERFORM_FAIL);
		}
		if (fileBuffer.substr(0,9) == "<!DOCTYPE")  //No release notes.
			fileBuffer.clear();
		curl_easy_cleanup(curl);

		return fileBuffer;
	}

	//Downloads and installs a BOSS update.
	BOSS_COMMON string DownloadInstallBOSSUpdate(uiStruct ui, const string updateVersion) {
		string file, releaseURL;
		uint32_t crc;
		releaseURL = "http://better-oblivion-sorting-software.googlecode.com/svn/releases/"+updateVersion+"/";

		//Get file info.
		GetBOSSFileInfo(releaseURL + "checksum.txt", file, crc);
			
		//Set file.
		ui.file = boss_path.string() + '/' + file;

		//Download file.
		string remote_file = releaseURL + file;
		string dest_file = boss_path.string() + '/' + file + ".new";
		boost::replace_all(remote_file, " ", "%20");  //Need to put the %20s back in for the file's web address.
		DownloadFile(ui, remote_file, dest_file);
		
		//Check if file is valid.
		if (GetCrc32(fs::path(dest_file)) != crc)
			throw boss_error(BOSS_ERROR_FILE_CRC_MISMATCH, dest_file);
		
		//Now install file.
		InstallFile(dest_file, (boss_path / file).string());

		return (boss_path / file).string();
	}
}