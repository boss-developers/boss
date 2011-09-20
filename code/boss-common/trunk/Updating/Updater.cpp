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
#include "Parsing\Parser.h"

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

#include <wx/msgdlg.h>
#include <wx/progdlg.h>

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
		
	fileInfo::fileInfo() {
		name = "";
		crc = 0;
	}

	fileInfo::fileInfo(string str) {
		name = str;
		crc = 0;
	}

	uiStruct::uiStruct() {
		p = 0;
		isGUI = false;
		fileIndex = 0;
	}

	uiStruct::uiStruct(void *GUIpoint) {
		p = GUIpoint;
		isGUI = true;
		fileIndex = 0;
	}

	vector<fileInfo> updatedFiles;  //The updated files. These don't have the .new extension.
	string filesURL;				//The URL at which the updated files are found.
	int ans;						//The answer to the cancel download confirmation message. 


	////////////////////////
	// General Functions
	////////////////////////

	//Download progress for downloader functions.
	int progress_func(void *data, double dlTotal, double dlNow, double ulTotal, double ulNow) {
		double fractiondownloaded = dlNow / dlTotal;
		if (dlTotal <= 0 || dlNow <= 0)
			fractiondownloaded = 0.0f;

		uiStruct * ui = (uiStruct*)data;
		if (ui->isGUI) {
			int currentProgress = (int)floor(fractiondownloaded * 1000);
			if (currentProgress == 1000)
				--currentProgress; //Stop the progress bar from closing in case of multiple downloads.

			wxProgressDialog* progress = (wxProgressDialog*)ui->p;
			bool cont = progress->Update(currentProgress,"Downloading: " + updatedFiles[ui->fileIndex].name + " (" + IntToString(ui->fileIndex+1) + " of " + IntToString(updatedFiles.size()) + ")");
			if (!cont) {  //the user decided to cancel. Slightly temperamental, the progDia seems to hang a little sometimes and keypresses don't get registered. Can't do much about that.
				if (!ans)
					ans = wxMessageBox(wxT("Are you sure you want to cancel?"), wxT("BOSS: Updater"), wxYES_NO | wxICON_EXCLAMATION, progress);
				if (ans == wxYES)
					return 1;
				progress->Resume();
				ans = NULL;
			}
		} else {
			printf("Downloading: %s (%u of %u); %3.0f%% of %3.0f KB\r", updatedFiles[ui->fileIndex].name.c_str(), ui->fileIndex+1, updatedFiles.size(), fractiondownloaded*100,(dlTotal/1024)+20);  //The +20 is there because for some reason there's always a 20kb difference between reported size and Windows' size.
			fflush(stdout);
		}
		return 0;
	}

	//Buffer writer for downloaders.
	int writer(char *data, size_t size, size_t nmemb, void *buffer){
		string *str = (string*)buffer;
		if(str != NULL) {
			str -> append(data, size * nmemb);
			return size * nmemb;
		}
		return 0;
	}

	CURL * InitCurl(char * errbuff) {
		CURLcode ret;
		string proxy_str;
		CURL *curl;

		curl = curl_easy_init();
		if (!curl)
			throw boss_error() << err_detail("Curl could not be initialised.");

		ret = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuff);	//Set error buffer for curl.
		if (ret != CURLE_OK) {
			curl_easy_cleanup(curl);
			throw boss_error() << err_detail("Could not set error buffer.");
		}
		ret = curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 20);		//Set connection timeout to 20s.
		if (ret != CURLE_OK) {
			string err = errbuff;
			curl_easy_cleanup(curl);
			throw boss_error() << err_detail(err);
		}
		ret = curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1);
		if (ret != CURLE_OK) {
			string err = errbuff;
			curl_easy_cleanup(curl);
			throw boss_error() << err_detail(err);
		}
		ret = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
		if (ret != CURLE_OK) {
			string err = errbuff;
			curl_easy_cleanup(curl);
			throw boss_error() << err_detail(err);
		}

		if (proxy_type != "direct" && proxy_host != "none" && proxy_port != "0") {
			//All of the settings have potentially valid proxy-ing values.
			proxy_str = proxy_host + ":" + proxy_port;
			ret = curl_easy_setopt(curl, CURLOPT_PROXY, proxy_str.c_str());
			if (ret!=CURLE_OK) {
				curl_easy_cleanup(curl);
				throw boss_error() << err_detail("Invalid proxy hostname or port specified.");
			}

			if (proxy_type == "http")
				ret = curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
			else if (proxy_type == "http1_0")
				ret = curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP_1_0);
			else if (proxy_type == "socks4")
				ret = curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS4);
			else if (proxy_type == "socks4a")
				ret = curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS4A);
			else if (proxy_type == "socks5")
				ret = curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);
			else if (proxy_type == "socks5h")
				ret = curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5_HOSTNAME);
			else {
				curl_easy_cleanup(curl);
				throw boss_error() << err_detail("Invalid proxy type specified.");
			}
			if (ret != CURLE_OK) {
				string err = errbuff;
				curl_easy_cleanup(curl);
				throw boss_error() << err_detail(err);
			}
		}

		return curl;
	}

	//Checks if an Internet connection is present.
	bool CheckConnection() {
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
			string err = errbuff;
			curl_easy_cleanup(curl);
			throw boss_error() << err_detail(err);
		} else {
			//Clean up and close curl handle now that it's finished with.
			curl_easy_cleanup(curl);
			return true;
		}
	}

	//Downloads the files in the updatedFiles vector at filesURL.
	void DownloadFiles(uiStruct ui, const int updateType) {
		string fileBuffer, remote_file, path;
		char errbuff[CURL_ERROR_SIZE];
		CURL *curl;									//cURL handle
		CURLcode ret;
		ofstream ofile;

		//curl will be used to get stuff from the internet, so initialise it.
		curl = InitCurl(errbuff);

		//Clear cancel confirmation answer.
		ans = 0;

		//Loop through the vector and download and save each file. Use binary streams.
		size_t size = updatedFiles.size();
		for (size_t i=0;i<size;i++) {
			if (updatedFiles[i].name.empty())  //Just in case.
				continue;
			fileBuffer.clear();  //Empty buffer ready for next download.

			//Set up progress indicator structure.
			ui.fileIndex = i;

			path = updatedFiles[i].name + ".new";
			if (updateType == MASTERLIST)
				ofile.open(path.c_str(),ios_base::trunc);
			else
				ofile.open(path.c_str(),ios_base::binary|ios_base::trunc);
			if (ofile.fail()) {
				curl_easy_cleanup(curl);
				throw boss_error() << err_detail("Could not save \""+path+"\"");
			}

			remote_file = filesURL + updatedFiles[i].name;
			boost::replace_all(remote_file," ","%20");  //Need to put the %20s back in for the file's web address.

			if (updateType == MASTERLIST)
				curl_easy_setopt(curl, CURLOPT_CRLF, 1);
			curl_easy_setopt(curl, CURLOPT_URL, remote_file.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writer);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &fileBuffer);
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
			curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, &progress_func);
			curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &ui);
			ret = curl_easy_perform(curl);
			if (ret == CURLE_ABORTED_BY_CALLBACK) {
				//Cancelled by user.
				curl_easy_cleanup(curl);
				throw boss_error() << err_detail("Cancelled by user.");
			} else if (ret!=CURLE_OK) {
				string err = errbuff;
				curl_easy_cleanup(curl);
				throw boss_error() << err_detail(err);
			}
			ofile << fileBuffer;
			ofile.close();

			//Now verify file integrity, but only if a program update, masterlist updates don't get CRCs calculated.
			if ((updateType == MANUAL || updateType == INSTALLER) && GetCrc32(fs::path(path)) != updatedFiles[i].crc)
				throw boss_error() << err_detail("Downloaded file \""+updatedFiles[i].name+"\" failed verification test. Please try updating again.");
		}
		curl_easy_cleanup(curl);
	}

	//Installs the downloaded update files.
	vector<string> InstallFiles(const int updateType) {
		//First back up current BOSS.ini if it exists and the update is a BOSS program update.
		if ((updateType == MANUAL || updateType == INSTALLER) && fs::exists("BOSS.ini"))
			fs::rename("BOSS.ini","BOSS.ini.old");

		//Now iterate through the vector of updated files.
		//Delete the current file if it exists, and rename the downloaded updated file, removing the .new extension.
		//The executable for the program running cannot be replaced, so will cause an exception. This and any others should be returned in a list for feedback.
		size_t size = updatedFiles.size();
		vector<string> err;
		for (size_t i=0;i<size;i++) {
			string old = updatedFiles[i].name;
			string updated = updatedFiles[i].name + ".new";

			try {
				fs::rename(updated,old);
			} catch (fs::filesystem_error e) {
				err.push_back("\"" + updatedFiles[i].name + "\"");
			}
		}
		return err;
	}

	//Cleans up after the user cancels a download.
	void CleanUp() {
		//Iterate through vector of updated files. Delete any that exist locally.
		size_t size = updatedFiles.size();
		for (size_t i=0;i<size;i++) {
			string file = updatedFiles[i].name + ".new";

			try {
				if (fs::exists(file))
					fs::remove(file);
			} catch (fs::filesystem_error e) {
				throw boss_error() << err_detail("Cannot delete downloaded file \"" + file + "\" as part of clean-up.");
			}
		}
	}


	////////////////////////
	// Masterlist Updating
	////////////////////////

	//Gets the revision number of the local masterlist.
	void GetLocalMasterlistRevisionDate(unsigned int& revision, string& date) {
		string line, newline = "Masterlist Revision:";
		ifstream mlist;
		char cbuffer[4096];
		size_t pos1,pos2,pos3;

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
					revision = atoi(line.substr(pos1+2,pos2-pos1-2).c_str());
					pos3 = line.find(")");
					date = line.substr(pos2+2,pos3-pos2-2);
					return;
				}
			}
			mlist.close();
		}
		revision = 0;
		date = "";
		return;  //No version found.
	}

	//Gets the revision number of the online masterlist.
	void GetRemoteMasterlistRevisionDate(unsigned int& revision, string& date) {
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
		revision = atoi(buffer.substr(start,end).c_str());

		//Extract revision date from page text.
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
	}

	//Updates the local masterlist to the latest available online.
	void UpdateMasterlist(uiStruct ui, unsigned int& localRevision, string& localDate, unsigned int& remoteRevision, string& remoteDate) {							//cURL handle
		string buffer,newline;		//A bunch of strings.
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
			//Set filesURL.
			if (game == 1) filesURL = "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-oblivion/";
			else if (game == 2) filesURL = "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-fallout/";
			else if (game == 3) filesURL = "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-nehrim/";
			else if (game == 4) filesURL = "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-fallout-nv/";
			else filesURL = "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-skyrim/";

			//Put masterlist.txt in updatedFiles.
			updatedFiles.clear();
			fileInfo file("masterlist.txt");
			updatedFiles.push_back(file);

			//Now download and install.
			DownloadFiles(ui, MASTERLIST);
			InstallFiles(MASTERLIST);

			//Now replace the SVN info in the downloaded file with the revision and date.
			newline = "Masterlist Revision: "+IntToString(remoteRevision)+" ("+remoteDate+")";

			fileToBuffer(masterlist_path, buffer);

			size_t pos = buffer.find(oldline);
			if (pos != string::npos)
				buffer.replace(pos,oldline.length(),newline);

			out.open(masterlist_path.c_str());
			if (out.fail())
				throw boss_error() << err_detail("Masterlist cannot be opened.");
			out << buffer;
			out.close();
		}
	}


	////////////////////////
	// BOSS Updating
	////////////////////////

	//Populates the updatedFiles vector.
	void FetchUpdateFileList(const int updateType, const string updateVersion) {
		string fileBuffer, remote_file;
		char errbuff[CURL_ERROR_SIZE];
		CURL *curl;									//cURL handle
		CURLcode ret;

		filesURL = "http://better-oblivion-sorting-software.googlecode.com/svn/releases/"+updateVersion+"/";
		
		//First decide what type of install we're updating.
		if (updateType == MANUAL)
			filesURL += "manual/";

		//curl will be used to get stuff from the internet, so initialise it.
		curl = InitCurl(errbuff);

		//First get file list and crcs to build updatedFiles vector.
		remote_file = filesURL+"checksums.txt";
		curl_easy_setopt(curl, CURLOPT_URL, remote_file.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writer);	
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &fileBuffer);
		ret = curl_easy_perform(curl);
		if (ret!=CURLE_OK) {
			curl_easy_cleanup(curl);
			throw boss_error() << err_detail(errbuff);
		}

		//Need to reset updatedFiles because it might have been set already if the updater was run then cancelled.
		updatedFiles.clear();

		//Now parse list to extract file info.
		string::const_iterator start = fileBuffer.begin(), end = fileBuffer.end();
		bool p = qi::phrase_parse(start,end,
			(('"' >> qi::lexeme[+(unicode::char_ - '"')] >> '"' >> qi::lit(":") >> qi::hex - qi::eol) | qi::eoi) % qi::eol,
			unicode::space - qi::eol, updatedFiles);
		if (!p || start != end) {
			curl_easy_cleanup(curl);
			throw boss_error() << err_detail("Could not read remote file information.");
		}
	}

	//Checks if a new release of BOSS is available or not.
	string IsBOSSUpdateAvailable() {
		string remoteVersionStr, proxy_str;
		char errbuff[CURL_ERROR_SIZE];
		CURL *curl;									//cURL handle
		CURLcode ret;
		const char *url = "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-common/latestversion.txt";

		//curl will be used to get stuff from the internet, so initialise it.
		curl = InitCurl(errbuff);

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
			return remoteVersionStr;
		} else
			return "";
	}

	//Downloads and installs a BOSS update.
	vector<string> DownloadInstallBOSSUpdate(uiStruct ui, const int updateType, const string updateVersion) {
		FetchUpdateFileList(updateType, updateVersion);
		DownloadFiles(ui, updateType);
		return InstallFiles(updateType);
	}

	
}