/*	BOSS
	
	A "one-click" program for users that quickly optimises and avoids 
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge, 
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

    Copyright (C) 2009-2012    BOSS Development Team.

	This file is part of BOSS.

    BOSS is free software: you can redistribute 
	it and/or modify it under the terms of the GNU General Public License 
	as published by the Free Software Foundation, either version 3 of 
	the License, or (at your option) any later version.

    BOSS is distributed in the hope that it will 
	be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with BOSS.  If not, see 
	<http://www.gnu.org/licenses/>.

	$Revision: 3188 $, $Date: 2011-08-27 08:16:41 +0100 (Sat, 27 Aug 2011) $
*/

#include "Updating/Updater.h"
#include "Common/Globals.h"
#include "Support/Helpers.h"
#include "Common/Error.h"
#include "Support/Logger.h"

#include <boost/spirit/include/qi.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

#include <stdio.h>
#include <iostream>
#include <fstream>

namespace boss {
	namespace unicode = boost::spirit::unicode;
	namespace qi = boost::spirit::qi;
	namespace phoenix = boost::phoenix;
	using namespace std;

	using boost::algorithm::replace_all;

	
	//Buffer writer for downloaders.
	int writer(char * data, size_t size, size_t nmemb, void * buffer) {
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

		if (!gl_proxy_host.empty() && gl_proxy_port != 0) {
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

	/////////////////////////////
	// Updater Class Functions
	/////////////////////////////
	
	string Updater::TargetFile() const {
		return targetFile;
	}

	void * Updater::ProgDialog() const {
		return progDialog;
	}

	void Updater::TargetFile(string file) {
		targetFile = file;
	}

	void Updater::ProgDialog(void * dialog) {
		progDialog = dialog;
	}

	//Handler for progress outputter.
	int Updater::progress_func(void * data, double dlTotal, double dlNow, double ulTotal, double ulNow) { 
		double dlFraction = 100 * dlNow / dlTotal;
		if (dlTotal <= 0 || dlNow <= 0)
			dlFraction = 0.0f;

		Updater * updater = static_cast<Updater*>(data);

		return updater->progress(updater, dlFraction, (dlTotal / 1024) + 20);
	}

	//Download the remote file to local. Throws exception on error.
	void Updater::DownloadFile(const string remote, const fs::path local) {
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
			throw boss_error(BOSS_ERROR_FILE_WRITE_FAIL, local.string());
		}
		
		//Download to buffer.
		curl_easy_setopt(curl, CURLOPT_CRLF, 1);  //This was set for masterlist only.
		curl_easy_setopt(curl, CURLOPT_URL, remote.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writer);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &fileBuffer);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, &boss::Updater::progress_func);
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, this);
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
	void Updater::InstallFile(string downloadedName, string installedName) {
		try {
			fs::rename(fs::path(downloadedName), fs::path(installedName));
		} catch (fs::filesystem_error e) {
			throw boss_error(BOSS_ERROR_FS_FILE_RENAME_FAIL, downloadedName, e.what());
		}
	}

	//Checks if an Internet connection is present.
	bool Updater::IsInternetReachable() {
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
	void Updater::CleanUp() {
		try {
			//Use a recursive directory iterator to find and delete and files with a ".new" extension.
			for (fs::directory_iterator itr(boss_path); itr!=fs::directory_iterator(); ++itr) {
				if (itr->path().extension().string() == ".new") {
					LOG_DEBUG("-- Deleting mod: '%s'", itr->path().string().c_str());
					try {
						fs::remove(itr->path());
					} catch (fs::filesystem_error e) {
						throw boss_error(BOSS_ERROR_FS_FILE_DELETE_FAIL, itr->path().string(), e.what());
					}
				}
			}
		} catch (fs::filesystem_error e) {
			throw boss_error(BOSS_ERROR_FS_ITER_DIRECTORY_FAIL, boss_path.string());
		}
	}


	///////////////////////////////////////
	// MasterlistUpdater Class Functions
	///////////////////////////////////////

	//Updates the local masterlist to the latest available online. Throws exception on error.
	void MasterlistUpdater::Update(const Game& game, fs::path file, uint32_t& localRevision, string& localDate, uint32_t& remoteRevision, string& remoteDate) {							//cURL handle
		string url, buffer, newline;		//A bunch of strings.
		ifstream mlist;								//Input stream.
		ofstream out;								//Output stream.
		const string SVN_REVISION_KW = "$" "Revision" "$";                   // Left as separated parts to avoid keyword expansion
		const string SVN_DATE_KW = "$" "Date" "$";                           // Left as separated parts to avoid keyword expansion
		const string SVN_CHANGEDBY_KW= "$" "LastChangedBy" "$";              // Left as separated parts to avoid keyword expansion
		string oldline = "Masterlist Information: "+SVN_REVISION_KW+", "+SVN_DATE_KW+", "+SVN_CHANGEDBY_KW;

		//Get local and remote masterlist info.
		GetLocalRevisionDate(file, localRevision, localDate);
		GetRemoteRevisionDate(game, remoteRevision, remoteDate);

		//Is an update available?
		if (localRevision == 0 || localRevision < remoteRevision) {
			url = string("http://better-oblivion-sorting-software.googlecode.com/svn/data/" + game.OnlineId() + "/masterlist.txt");
			TargetFile(file.string());

			//Now download and install.
			DownloadFile(url, fs::path(file.string() + ".new"));
			InstallFile(file.string() + ".new", file.string());

			//Now replace the SVN info in the downloaded file with the revision and date.
			newline = "Masterlist Revision: "+IntToString(remoteRevision)+" ("+remoteDate+")";

			fileToBuffer(file, buffer);

			size_t pos = buffer.find(oldline);
			if (pos != string::npos)
				buffer.replace(pos, oldline.length(), newline);

			out.open(file.c_str());
			if (out.fail())
				throw boss_error(BOSS_ERROR_FILE_WRITE_FAIL, file.string());
			out << buffer;
			out.close();
		}
	}

	//Gets the revision number of the local masterlist. Throws exception on error.
	void MasterlistUpdater::GetLocalRevisionDate(fs::path file, uint32_t& revision, string& date) {
		string line, newline = "Masterlist Revision:";
		ifstream mlist;
		size_t pos1,pos2,pos3;

		if (fs::exists(file)) {
			mlist.open(file.c_str());
			if (mlist.fail())
				throw boss_error(BOSS_ERROR_FILE_READ_FAIL, file.string());
			while (mlist.good()) {
				getline(mlist, line);
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
	void MasterlistUpdater::GetRemoteRevisionDate(const Game& game, uint32_t& revision, string& date) {
		char errbuff[CURL_ERROR_SIZE];
		CURL *curl;									//cURL handle
		string buffer;		//A bunch of strings.
		size_t start,end;								//Position holders for trimming strings.
		CURLcode ret;
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
		start = buffer.find("\""+ game.OnlineId() + "\":");
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
		start = buffer.find("\""+ game.OnlineId() + "\":");
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
		date = GetISO8601Date(buffer.substr(start+1,end - (start+1)));
		curl_easy_cleanup(curl);
	}

	string MasterlistUpdater::GetISO8601Date(string str) {
		string year, month, day;
		//Need format "YYYY-MM-DD".
		if (str.find("Today") != string::npos || str.find("Yesterday") != string::npos) {
			time_t currTimeTT = time(NULL);
			if (str.find("Yesterday") != string::npos)
				currTimeTT -= 86400;  //Time travel! Do this here instead of with tm_mday, because it might be the first of the month.
			tm * currTimeTM = localtime(&currTimeTT);

			year = IntToString(currTimeTM->tm_year + 1900);  //tm_year counts from 1900. (Why?) Don't need to pad.
			month = IntToString(currTimeTM->tm_mon + 1);  //tm_mon starts from zero. May need to pad so it is 2 digits long.
			if (month.length() == 1)
				month = "0" + month;
			day = IntToString(currTimeTM->tm_mday);  //May need to pad.
			if (day.length() == 1)
				day = "0" + day;
		} else {
			/* Two possible formats:
			     1. The date is in the format "Mmm (D)?D (...)". The year is the current year.
			     2. The date is in the format "Mmm (D)?D, YYYY".
			   (D)? means that the day isn't padded.
			*/
			if (str.length() > 5) {

				//First turn month word string into a number string.
				month = str.substr(0, 3);
				if (month == "Jan")
					month = "01";
				else if (month == "Feb")
					month = "02";
				else if (month == "Mar")
					month = "03";
				else if (month == "Apr")
					month = "04";
				else if (month == "May")
					month = "05";
				else if (month == "Jun")
					month = "06";
				else if (month == "Jul")
					month = "07";
				else if (month == "Aug")
					month = "08";
				else if (month == "Sep")
					month = "09";
				else if (month == "Oct")
					month = "10";
				else if (month == "Nov")
					month = "11";
				else
					month = "12";

				if (str[5] == ',' || str[5] == ' ')  //Day is one digit long, pad.
					day = "0" + str.substr(4, 1);
				else  //Day is two digits long.
					day = str.substr(4, 2);

				if (str.find("(") != string::npos) {
					//The date is in the format "Month (D)?D (...)". The year is the current year.
					//Get the current year.
					const time_t currTimeTT = time(NULL);
					tm * currTimeTM = localtime(&currTimeTT);

					year = IntToString(currTimeTM->tm_year + 1900);  //tm_year counts from 1900. (Why?) Don't need to pad.
				} else {
					//The date is in the format "Mmm (D)?D, YYYY".
					size_t pos = str.find(',');
					if (pos != string::npos)
						year = str.substr(pos + 2, 4);
				}
			}
		}
		return (year + '-' + month + '-' + day);
	}


	/////////////////////////////////
	// BOSSUpdater Class Functions
	/////////////////////////////////

	//Checks if a new release of BOSS is available or not. Throws exception on error.
	string BOSSUpdater::IsUpdateAvailable() {
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
	string BOSSUpdater::FetchReleaseNotes(const string updateVersion) {
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
		//Check result.
		long int code;
		ret = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
		if (ret!=CURLE_OK) {
			string err = errbuff;
			curl_easy_cleanup(curl);
			throw boss_error(err, BOSS_ERROR_CURL_PERFORM_FAIL);
		}
		if (code != 200)  //No release notes.
			fileBuffer.clear();
		curl_easy_cleanup(curl);

		return fileBuffer;
	}

	//Downloads and installs a BOSS update.
	void BOSSUpdater::GetUpdate(fs::path file, const string updateVersion) {
		string url = "http://better-oblivion-sorting-software.googlecode.com/svn/releases/"+updateVersion+"/BOSS%20Installer.exe";

		TargetFile(file.string());

		//Now download and install.
		DownloadFile(url, fs::path(file.string() + ".new"));
		InstallFile(file.string() + ".new", file.string());
	}
}