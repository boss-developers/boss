/*	General User Interface for BOSS (Better Oblivion Sorting Software)
	
	Providing a graphical frontend to BOSS's functions.

    Copyright (C) 2011 WrinklyNinja & the BOSS development team.
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

//Header file for some functions that are helpful or required for the GUI to work,
//but not actually GUI-based.

#ifndef CURL_STATICLIB
#define CURL_STATICLIB			//Tells the compiler to use curl as a static library.
#endif
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

#include "helpers.h"
#include <boost/spirit/include/karma.hpp>
#include <boost/exception/exception.hpp>
#include <boost/exception/info.hpp>
#include <boost/algorithm/string.hpp>
#include "Windows.h"

namespace boss {
	using namespace std;
	namespace fs = boost::filesystem;
	namespace karma = boost::spirit::karma;

	//Version info.
	const string gui_version     = "1.7";
	const string gui_releaseDate = "June 10, 2011";

	//The run type decides on which variables are applied, not all are appropriate for all run types.
	int run					= 1;     // 1 = sort mods, 2 = only update, 3 = undo changes.
	int format				= 0;	 // what format the output should be in. 0 = HTML, 1 = plaintext.
	int verbosity           = 0;     // Command-line output verbosity.
	int game				= 0;	 // Force what game? 0 = allow autodetection, 1 = Oblivion, 2 = Fallout 3, 3 = Nehrim, 4 = Fallout: New Vegas.
	int revert              = 0;     // what level to revert to.
	bool showLog            = true;  // Auto-open BOSSlog?
	bool debug              = false; // Display debug info?
	bool update				= false; // update the masterlist
	bool showVersions		= true;  // enable parsing of mod's headers to look for version strings
	bool showCRCs			= false; // whether or not to show mod CRCs.
	bool logCL				= false; // whether or not to log the command line output to BOSSDebugLog.txt.
	

	//Returns the name of the game that BOSS is currently running for.
	string GetGame() {
		if (fs::exists("../Oblivion.exe")) {
			if (fs::exists("../Data/Nehrim.esm"))
				return "Nehrim";
			else
				return "Oblivion";
		} else if (fs::exists("../Fallout3.exe"))
			return "Fallout 3";
		else if (fs::exists("../FalloutNV.exe"))
			return "Fallout: New Vegas";
		else 
			return "Game Not Detected";
	}

	//Runs BOSS with arguments according to the settings of the run variables.
	void RunBOSS() {
		string params = "BOSS.exe";
		//Set format output params.
		if (!showLog)
			params += " -s";
		if (debug)
			params += " -d";
		if (verbosity > 0)
			params += " -v" + IntToString(verbosity);
		if (format == 1)
			params += " -f text";
		//Set run-dependent params.
		if (run == 1) {  //Sort mods.
			if (update)
				params += " -u";
			if (!showVersions)
				params += " -n";
			if (showCRCs)
				params += " -c";
		} else if (run == 2) {  //Update masterlist only.
			params += " -o";
			if (game == 1)
				params += " -g Oblivion";
			else if (game == 2)
				params += " -g Fallout3";
			else if (game == 3)
				params += " -g Nehrim";
			else if (game == 4)
				params += " -g FalloutNV";
		} else if (run == 3) {  //Undo changes.
			if (revert > 0)
				params += " -r" + IntToString(revert);
			if (!showVersions)
				params += " -n";
			if (showCRCs)
				params += " -c";

		}
		if (logCL)
			params += " > BOSSCommandLineLog.txt";
		//Now actually run BOSS.
		system(params.c_str());
		return;
	}

	//Opens the given file in the default system program.
	void OpenInSysDefault(fs::path& file) {
		string command =
#if _WIN32 || _WIN64
			"start ";
#else
			"xdg-open ";
#endif
		if (file.extension() == ".lnk\"" || file.extension() == ".html\"")
			command = "";
		command += file.string();
		system(command.c_str());
		return;
	}

	//Converts an integer to a string using BOOST's Spirit.Karma, which is apparently a lot faster than a stringstream conversion...
	string IntToString(unsigned int n) {
		string out;
		back_insert_iterator<string> sink(out);
		karma::generate(sink,karma::upper[karma::uint_],n);
		return out;
	}

	//Gets BOSS's version number.
	int GetBOSSVersion() {
		int retVal = 0;
		string ver;
		string filename = "BOSS.exe";
#if _WIN32 || _WIN64
		// WARNING - NOT VERY SAFE, SEE http://www.boost.org/doc/libs/1_46_1/libs/filesystem/v3/doc/reference.html#current_path
		fs::path pStr = fs::current_path() / filename;
		DWORD dummy = 0;
		DWORD size = GetFileVersionInfoSize(pStr.wstring().c_str(), &dummy);

		if (size > 0) {
			LPBYTE point = new BYTE[size];
			UINT uLen;
			VS_FIXEDFILEINFO *info;

			GetFileVersionInfo(pStr.wstring().c_str(),0,size,point);

			VerQueryValue(point,L"\\",(LPVOID *)&info,&uLen);

			DWORD dwLeftMost     = HIWORD(info->dwFileVersionMS);
			DWORD dwSecondLeft   = LOWORD(info->dwFileVersionMS);
			DWORD dwSecondRight  = HIWORD(info->dwFileVersionLS);
			DWORD dwRightMost    = LOWORD(info->dwFileVersionLS);
			
			delete [] point;

			ver = IntToString(dwLeftMost) + IntToString(dwSecondLeft) + IntToString(dwSecondRight) + IntToString(dwRightMost);
			retVal = atoi(ver.c_str());
		}
#else
		// ensure filename has no quote characters in it to avoid command injection attacks
        if (string::npos != filename.find('"')) {
    	    LOG_WARN("filename has embedded quotes; skipping to avoid command injection: '%s'", filename.c_str());
        } else {
            // command mostly borrowed from the gnome-exe-thumbnailer.sh script
            // wrestool is part of the icoutils package
            string cmd = "wrestool --extract --raw --type=version \"" + filename + "\" | tr '\\0, ' '\\t.\\0' | sed 's/\\t\\t/_/g' | tr -c -d '[:print:]' | sed -r 's/.*Version[^0-9]*([0-9]+(\\.[0-9]+)+).*/\\1/'";

            FILE *fp = popen(cmd.c_str(), "r");

            // read out the version string
            static const int BUFSIZE = 32;
            char buf[BUFSIZE];
            if (NULL == fgets(buf, BUFSIZE, fp)) {
    	        LOG_DEBUG("failed to extract version from '%s'", filename.c_str());
            }
            else {
                ver = string(buf);
				retVal = atoi(ver);
	   	        LOG_DEBUG("extracted version from '%s': %s", filename.c_str(), ver.c_str());
            }

            pclose(fp);
        }
#endif
		return retVal;
	}

	int versionStringToInt(string version) {
		boost::replace_all(version,".","");
		return atoi(version.c_str());
	}

	//Buffer writer for update checker.
	int writer(char *data, size_t size, size_t nmemb, string *buffer) {
		int result = 0;
		if(buffer != NULL) {
			buffer -> append(data, size * nmemb);
			result = size * nmemb;
		}
		return result;
	}

	//Checks if an update is available or not for the given item.
	//Valid items are 'BOSS' and 'masterlist'.
	string IsUpdateAvailable(string subject) {
		int localVersion, remoteVersion;
		string remoteVersionStr;
		char cbuffer[4096];
		char errbuff[CURL_ERROR_SIZE];
		CURL *curl;									//cURL handle
		CURLcode ret;
		size_t start=-1,end;								//Position holders for trimming strings.

		//curl will be used to get stuff from the internet, so initialise it.
		curl = curl_easy_init();
		if (!curl) 
			throw update_error() << err_detail("Curl could not be initialised.");
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuff);	//Set error buffer for curl.
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 20);		//Set connection timeout to 10s.

		if (subject == "masterlist") {
			fs::path masterlist_path = "masterlist.txt";
			string line;
			ifstream mlist;					//Input stream.
		
			//Get revision number from http://code.google.com/p/better-oblivion-sorting-software/source/browse/#svn page text.
			curl_easy_setopt(curl, CURLOPT_URL, "http://code.google.com/p/better-oblivion-sorting-software/source/browse/#svn");
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);	
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &remoteVersionStr );
			ret = curl_easy_perform(curl);
			if (ret!=CURLE_OK)
				throw update_error() << err_detail(errbuff);
		
			//Extract revision number from page text.
			if (GetGame() == "Oblivion") start = remoteVersionStr.find("boss-oblivion");
			else if (GetGame() == "Fallout 3") start = remoteVersionStr.find("boss-fallout");
			else if (GetGame() == "Nehrim") start = remoteVersionStr.find("boss-nehrim");
			else if (GetGame() == "Fallout: New Vegas") start = remoteVersionStr.find("boss-fallout-nv");
			else throw update_error() << err_detail("No local masterlist found. Cannot determine which masterlist needs checking.");
			if (start == string::npos)
				throw update_error() << err_detail("Cannot find online masterlist revision number.");
			start = remoteVersionStr.find("\"masterlist.txt\"", start);
			start = remoteVersionStr.find("B\",\"", start) + 4; 
			if (start == string::npos)
				throw update_error() << err_detail("Cannot find online masterlist revision number.");
			end = remoteVersionStr.find("\"",start) - start;
			if (end == string::npos)
				throw update_error() << err_detail("Cannot find online masterlist revision number.");
			remoteVersionStr = remoteVersionStr.substr(start,end);

			//Compare remote revision to current masterlist revision - if identical don't waste time/bandwidth updating it.
			if (fs::exists(masterlist_path)) {
				mlist.open(masterlist_path.c_str());
				if (mlist.fail())
					throw update_error() << err_detail("Masterlist cannot be opened.");
				while (!mlist.eof()) {
					mlist.getline(cbuffer,4096);
					line=cbuffer;
					if (line.find("? Masterlist Revision") != string::npos) {
						if (line.find(remoteVersionStr) != string::npos) {
							return "";  //Masterlist already at latest revision.
						} else 
							break;
					}
				}
				mlist.close();
			}
			return remoteVersionStr;
		} else {
			//Get page containing version number.
			curl_easy_setopt(curl, CURLOPT_URL, "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-common/latestversion.txt");
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);	
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &remoteVersionStr );
			ret = curl_easy_perform(curl);
			if (ret!=CURLE_OK)
				throw update_error() << err_detail(errbuff);

			localVersion = GetBOSSVersion();
			remoteVersion = versionStringToInt(remoteVersionStr);

			//Now compare versions.
			if (remoteVersion > localVersion)
				return remoteVersionStr;
			else
				return "";
		}
		return "";
	}
}