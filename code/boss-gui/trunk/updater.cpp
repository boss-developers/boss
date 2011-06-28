/*	General User Interface for BOSS (Better Oblivion Sorting Software)
	
	Providing a graphical frontend to BOSS's functions.

    Copyright (C) 2011 WrinklyNinja & the BOSS development team.
    http://creativecommons.org/licenses/by-nc-nd/3.0/


	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#ifndef CURL_STATICLIB
#define CURL_STATICLIB			//Tells the compiler to use curl as a static library.
#endif

#include <iostream>
#include <cmath>
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>
#include <boost/exception/exception.hpp>
#include <boost/exception/info.hpp>
#include <boost/algorithm/string.hpp>

#include "helpers.h"
#include "updater.h"

//#include <wx/msgdlg.h> 


namespace boss {
	using namespace std;
	namespace fs = boost::filesystem;

	string updateVersion = "";
	vector<fs::path> updatedFiles;

	//Buffer writer for update checker.
	int writer(char *data, size_t size, size_t nmemb, string *buffer) {
		int result = 0;
		if(buffer != NULL) {
			buffer -> append(data, size * nmemb);
			result = size * nmemb;
		}
		return result;
	}

	int stream_writer(char *data, size_t size, size_t nmemb, void *buffer) {
		ofstream *out = (ofstream*)buffer;
		*out << data;
		return size*nmemb;
	}

	//Download progress for current file function.
	int progress_func(void *data, double dlTotal, double dlNow, double ulTotal, double ulNow) {
		double percentdownloaded = (dlNow / dlTotal) * 1000;
		int currentProgress = floor(percentdownloaded);
		if (currentProgress == 1000)
			--currentProgress;
		
		wxProgressDialog* progDia = (wxProgressDialog*)data;
		bool cont = progDia->Update(currentProgress);
		//Disabled the below for now. At some point an option to cancel that would feed 
		//into a 'clean up' function that would delete the downloaded files would be good.
		//Too complicated for the moment.
	/*	if (!cont) {
            if ( wxMessageBox(wxT("Do you really want to cancel?"),
                              wxT("Automatic Updater: Exit Confirmation"),  // caption
                              wxYES_NO | wxICON_QUESTION) == wxYES ) {
				cont = true;
				progDia->Resume();
			} else {
				progDia->Update(1000);
        }*/
		return 0;
	}

	bool CheckConnection() {
		CURL *curl;									//cURL handle
		char errbuff[CURL_ERROR_SIZE];
		CURLcode ret;

		//curl will be used to get stuff from the internet, so initialise it.
		curl = curl_easy_init();
		if (!curl) 
			return false;
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuff);	//Set error buffer for curl.
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 20);		//Set connection timeout to 20s.

		//Check that there is an internet connection. Easiest way to do this is to check that the BOSS google code page exists.
		curl_easy_setopt(curl, CURLOPT_URL, "http://code.google.com/p/better-oblivion-sorting-software/");
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
		string remoteVersionStr;
		char errbuff[CURL_ERROR_SIZE];
		CURL *curl;									//cURL handle
		CURLcode ret;

		//curl will be used to get stuff from the internet, so initialise it.
		curl = curl_easy_init();
		if (!curl)
			throw update_error() << err_detail("Curl could not be initialised.");
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuff);	//Set error buffer for curl.
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 20);		//Set connection timeout to 20s.

		//Get page containing version number.
		curl_easy_setopt(curl, CURLOPT_URL, "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-common/latestversion.txt");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);	
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

	//Downloads the installer for the update, for when the current version was installed via installer.
	void DownloadUpdateInstaller(wxProgressDialog *progDia) {
		string fileBuffer;
		char errbuff[CURL_ERROR_SIZE];
		CURL *curl;									//cURL handle
		CURLcode ret;
		string path = "BOSS "+updateVersion+" installer.exe";
		string url = "http://better-oblivion-sorting-software.googlecode.com/svn/releases/"+updateVersion+"/installer.exe";

		//curl will be used to get stuff from the internet, so initialise it.
		curl = curl_easy_init();
		if (!curl) {
			progDia->Update(1000);
			throw update_error() << err_detail("Curl could not be initialised.");
		}
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuff);	//Set error buffer for curl.
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 20);		//Set connection timeout to 20s.

		//Open output file stream.
		ofstream ofile(path.c_str(),ios_base::binary|ios_base::out|ios_base::trunc);
		if (ofile.fail()) {
			progDia->Update(1000);
			curl_easy_cleanup(curl);
			throw update_error() << err_detail("Could not save "+path);
		}
		
		//Set up progress info.
		updatedFiles.push_back(fs::path(path));  //Record just in case abort cleanup is required.
		string message = "Downloading: " + path + " (1 of 1)";
		progDia->Update(0,message);
		
		//Download the installer.
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &fileBuffer);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, &progress_func);
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, progDia);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
		ret = curl_easy_perform(curl);
		if (ret!=CURLE_OK) {
			curl_easy_cleanup(curl);
			progDia->Update(1000);
			throw update_error() << err_detail(errbuff);
		}
		ofile << fileBuffer;
		ofile.close();
		//Clean up curl resources.
		curl_easy_cleanup(curl);
		progDia->Update(1000);
	}

	//Download the files in the update.
	void DownloadUpdateFiles(wxProgressDialog *progDia) {
		string url, fileBuffer, buffer;
		char errbuff[CURL_ERROR_SIZE];
		CURL *curl;									//cURL handle
		CURLcode ret;
		size_t pos1,pos2;

		//curl will be used to get stuff from the internet, so initialise it.
		curl = curl_easy_init();
		if (!curl) {
			progDia->Update(1000);
			throw update_error() << err_detail("Curl could not be initialised.");
		}
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuff);	//Set error buffer for curl.
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 20);		//Set connection timeout to 20s.

		//First connect to the folder containing the update files. The server returns a page listing the files in the folder.
		//Use this list to build the updatedFiles vector.
		url = "http://better-oblivion-sorting-software.googlecode.com/svn/releases/"+updateVersion+"/manual/";
		//Get page containing version number.
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);	
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &fileBuffer);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
		ret = curl_easy_perform(curl);
		if (ret!=CURLE_OK) {
			progDia->Update(1000);
			curl_easy_cleanup(curl);
			throw update_error() << err_detail(errbuff);
		}
		pos1 = fileBuffer.find("<li><a href=\"");  //Folder item. The first is the 'go up one directory' item, so skip.
		while (pos1 != string::npos) { //Loop through the rest of the items.
			pos1 = fileBuffer.find("<li><a href=\"", pos1+1);
			if (pos1 == string::npos)
				break;
			pos2 = fileBuffer.find("\">",pos1);
			buffer = fileBuffer.substr(pos1+13,pos2-pos1-13); //Now we need to replace any %20 strings with spaces.
			boost::replace_all(buffer,"%20"," ");  //If future files contain more special characters then this will break.
			//Now record in updateFiles.
			updatedFiles.push_back(fs::path(buffer));
		}

		//Now that we've got a vector of the files, we can download them.
		//Loop through the vector and download and save each file. Use binary streams.
		for (size_t i=0;i<updatedFiles.size();i++) {
			fileBuffer.clear();  //Empty buffer ready for next download.
			//Set up progress info. Since we're not doing a total download progress bar, zero progress for each file.
			string message = "Downloading: " + updatedFiles[i].string() + " (" + IntToString(i+1) + " of " + IntToString(updatedFiles.size()) + ")";
			progDia->Update(0,message);

			string path = updatedFiles[i].string() + ".new";
			ofstream ofile(path.c_str(),ios_base::binary|ios_base::out|ios_base::trunc);
			if (ofile.fail()) {
				progDia->Update(1000);
				curl_easy_cleanup(curl);
				throw update_error() << err_detail("Could not save "+path);
			}

			string remote_file = url+updatedFiles[i].string();
			boost::replace_all(remote_file," ","%20");  //Need to put the %20s back in for the file's web address.

			curl_easy_setopt(curl, CURLOPT_URL, remote_file);
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
			curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, &progress_func);
			curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, progDia);
			ret = curl_easy_perform(curl);
			if (ret!=CURLE_OK) {
				progDia->Update(1000);
				curl_easy_cleanup(curl);
				throw update_error() << err_detail(errbuff);
			}
			ofile << fileBuffer;
			ofile.close();
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
			fs::path updated = fs::path(updatedFiles[i].string() + ".new");
			fs::path old = updatedFiles[i];

			if (old.string() != "BOSS GUI.exe") {
				if (fs::exists(old))
					fs::remove(old);

				fs::rename(updated,old);
			}
		}
	}
}