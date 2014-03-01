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

	$Revision: 3184 $, $Date: 2011-08-26 20:52:13 +0100 (Fri, 26 Aug 2011) $
*/

#ifndef __BOSS_UPDATER_H__
#define __BOSS_UPDATER_H__

#ifndef BOOST_SPIRIT_UNICODE
#define BOOST_SPIRIT_UNICODE 
#endif

#include <string>
#include <vector>
#include <boost/cstdint.hpp>
#include <boost/filesystem.hpp>
#include "Common/DllDef.h"
#include "Common/Game.h"

#ifndef CURL_STATICLIB
#define CURL_STATICLIB			//Tells the compiler to use curl as a static library.
#endif
#include <curl/curl.h>
#include <curl/easy.h>

namespace boss {
	using namespace std;
	namespace fs = boost::filesystem;

	//Initalise a curl handle.
	CURL * InitCurl(char * errbuff);

	//Data writer.
	int writer(char * data, size_t size, size_t nmemb, void * buffer);

	class BOSS_COMMON Updater {
	public:
		string TargetFile() const;
		void * ProgDialog() const;

		void TargetFile(string file);
		void ProgDialog(void * dialog);

		//Checks if an Internet connection is present.
		bool IsInternetReachable();

		//Cleans up after the user cancels a download. Throws boss_error exception on fail.
		void CleanUp();

	protected:
		//Handler for progress outputter.
		static int progress_func(void *data, double dlTotal, double dlNow, double ulTotal, double ulNow);
		
		//Download progress for downloader functions.
		virtual int progress(Updater * updater, double dlFraction, double dlTotal) = 0;

		//Download the remote file to local. Throws exception on error.
		void DownloadFile(const string remote, const fs::path local);

		//Install file by renaming it. Throws exception on error.
		void InstallFile(string downloadedName, string installedName);
	private:
		string targetFile;
		void * progDialog;
	};

	class BOSS_COMMON MasterlistUpdater : public Updater {
	public:
		//Updates the local masterlist to the latest available online. Throws boss_error exception on fail.
		void Update(const Game& game, fs::path file, uint32_t& localRevision, string& localDate, uint32_t& remoteRevision, string& remoteDate);

	private:
		//Gets the revision number of the local masterlist. Throws exception on error.
		void GetLocalRevisionDate(fs::path file, uint32_t& revision, string& date);

		//Gets the revision number of the online masterlist. Throws exception on error.
		void GetRemoteRevisionDate(const Game& game, uint32_t& revision, string& date);

		string GetISO8601Date(string str);
	};

	class BOSS_COMMON BOSSUpdater : public Updater {
	public:
		//Checks if a new release of BOSS is available or not. Throws boss_error exception on fail.
		string IsUpdateAvailable();

		//Gets the release notes for the update. Throws boss_error exception on fail.
		string FetchReleaseNotes(const string updateVersion);

		//Downloads and installs a BOSS update. Throws boss_error exception on fail.
		void GetUpdate(fs::path file, const string updateVersion);
	};
}
#endif