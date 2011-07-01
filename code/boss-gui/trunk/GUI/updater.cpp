/*	General User Interface for BOSS (Better Oblivion Sorting Software)
	
	Providing a graphical frontend to BOSS's functions.

    Copyright (C) 2011 WrinklyNinja & the BOSS development team.
    http://creativecommons.org/licenses/by-nc-nd/3.0/


	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#ifndef CURL_STATICLIB
#define CURL_STATICLIB			//Tells the compiler to use curl as a static library.
#endif

#ifndef BOOST_SPIRIT_UNICODE
#define BOOST_SPIRIT_UNICODE 
#endif

#include <iostream>
#include <cmath>
#include <curl/curl.h>
#include <curl/easy.h>
#include <boost/exception/exception.hpp>
#include <boost/exception/info.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <wx/msgdlg.h>

#include "Helpers/helpers.h"
#include "GUI/updater.h"

BOOST_FUSION_ADAPT_STRUCT(
    boss::fileInfo,
	(std::string, name)
    (unsigned int, crc)
)

namespace boss {
	using namespace std;
	namespace fs = boost::filesystem;
	namespace qi = boost::spirit::qi;
	namespace unicode = boost::spirit::unicode;

	using qi::eol;
	using qi::eoi;
	using qi::eps;
	using qi::lit;
	using qi::hex;
	using qi::lexeme;
	using unicode::char_;
	using unicode::space;

	string updateVersion = "";
	vector<fileInfo> updatedFiles;

	//Buffer writer for update checker.
	size_t writer(char *data, size_t size, size_t nmemb, void *buffer) {
		string *str = (string*)buffer;
		if(str != NULL) {
			str -> append(data, size * nmemb);
			return size * nmemb;
		}
		return 0;
	}

	//Download progress for current file function.
	int progress_func(void *data, double dlTotal, double dlNow, double ulTotal, double ulNow) {
		double percentdownloaded = (dlNow / dlTotal) * 1000;
		int currentProgress = floor(percentdownloaded);
		if (currentProgress == 1000)
			--currentProgress; //Stop the progress bar from closing before all files are downloaded.
		
		wxProgressDialog* progDia = (wxProgressDialog*)data;
		progDia->Update(currentProgress);
		if (progDia->WasCancelled())  //the user decided to cancel. This is really temperamental - most of the time the clicks don't seem to register.
			return 1;
		return 0;
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
			throw update_error() << err_detail("Curl could not be initialised.");
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
				throw update_error() << err_detail("Invalid proxy hostname or port specified.");
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
				throw update_error() << err_detail("Invalid proxy type specified.");
			}
		}

		//Check that there is an internet connection. Easiest way to do this is to check that the BOSS google code page exists.
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 1);	
		ret = curl_easy_perform(curl);
		//Clean up and close curl handle now that it's finished with.
		curl_easy_cleanup(curl);

		if (ret!=CURLE_OK)
			return false;
		else
			return true;
	}

	//Checks if a new release of BOSS is available or not.
	string IsUpdateAvailable() {
		int localVersion, remoteVersion;
		string remoteVersionStr, proxy_str;
		char errbuff[CURL_ERROR_SIZE];
		CURL *curl;									//cURL handle
		CURLcode ret;
		const char *url = "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-common/latestversion.txt";

		//curl will be used to get stuff from the internet, so initialise it.
		curl = curl_easy_init();
		if (!curl)
			throw update_error() << err_detail("Curl could not be initialised.");
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
				throw update_error() << err_detail("Invalid proxy hostname or port specified.");
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
				throw update_error() << err_detail("Invalid proxy type specified.");
			}
		}

		//Get page containing version number.
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writer);	
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &remoteVersionStr);
		ret = curl_easy_perform(curl);
		if (ret!=CURLE_OK) {
			curl_easy_cleanup(curl);
			throw update_error() << err_detail(errbuff);
		}

		localVersion = versionStringToInt(boss_version);
		remoteVersion = versionStringToInt(remoteVersionStr);

		//Now compare versions.
		if (remoteVersion > localVersion) {
			updateVersion = remoteVersionStr;
			return updateVersion;
		} else
			return "";
	}

	//Download the files in the update.
	void DownloadUpdateFiles(int updateType, wxProgressDialog *progDia) {
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

		//curl will be used to get stuff from the internet, so initialise it.
		curl = curl_easy_init();
		if (!curl) {
			throw update_error() << err_detail("Curl could not be initialised.");
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
				throw update_error() << err_detail("Invalid proxy hostname or port specified.");
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
				throw update_error() << err_detail("Invalid proxy type specified.");
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
			throw update_error() << err_detail(errbuff);
		}

		//Now parse list to extract file info.
		string::const_iterator start = fileBuffer.begin(), end = fileBuffer.end();
		bool p = qi::phrase_parse(start,end,
			(("\"" >> lexeme[+(char_ - lit("\""))] >> "\"" >> lit(":") >> hex - eol) | eoi) % eol,
			space - eol, updatedFiles);
		if (!p || start != end) {
			curl_easy_cleanup(curl);
			throw update_error() << err_detail("Could not read remote file information.");
		}

		//Now that we've got a vector of the files, we can download them.
		//Loop through the vector and download and save each file. Use binary streams.
		for (size_t i=0;i<updatedFiles.size();i++) {
			if (updatedFiles[i].name.empty())
				continue;
			fileBuffer.clear();  //Empty buffer ready for next download.
			//Set up progress info. Since we're not doing a total download progress bar, zero progress for each file.
			message = "Downloading: " + updatedFiles[i].name + " (" + IntToString(i+1) + " of " + IntToString(updatedFiles.size()) + ")";
			progDia->Update(0,message);

			string path = updatedFiles[i].name + ".new";
			ofstream ofile(path.c_str(),ios_base::binary|ios_base::out|ios_base::trunc);
			if (ofile.fail()) {
				curl_easy_cleanup(curl);
				throw update_error() << err_detail("Could not save \""+path+"\"");
			}

			remote_file = url+updatedFiles[i].name;
			boost::replace_all(remote_file," ","%20");  //Need to put the %20s back in for the file's web address.

			curl_easy_setopt(curl, CURLOPT_URL, remote_file.c_str());
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
			curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, &progress_func);
			curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, progDia);
			ret = curl_easy_perform(curl);
			if (ret == CURLE_ABORTED_BY_CALLBACK) {
				//Cancelled by user.
				curl_easy_cleanup(curl);
				throw update_error() << err_detail("Cancelled by user.");
			} else if (ret!=CURLE_OK) {
				curl_easy_cleanup(curl);
				throw update_error() << err_detail(errbuff);
			}
			ofile << fileBuffer;
			ofile.close();

			//Now verify file integrity.
			calcedCRC = GetCrc32(fs::path(path));
			if (calcedCRC != updatedFiles[i].crc) {
				throw update_error() << err_detail("Downloaded file \""+updatedFiles[i].name+"\" failed verification test. Please try updating again.");
			}
		}
		curl_easy_cleanup(curl);
		progDia->Update(1000);
	}

	//Installs the downloaded update files.
	void InstallUpdateFiles() {
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