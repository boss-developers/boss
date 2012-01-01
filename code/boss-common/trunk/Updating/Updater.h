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
#include "Common/DllDef.h"

namespace boss {
	using namespace std;

	BOSS_COMMON_EXP enum installType : uint32_t {  //Possible types of install the user has.
		MANUAL,
		INSTALLER,
		MASTERLIST
	};

	struct fileInfo {  //Holds the information for files to be downloaded, and also folders to be created/destroyed.
		bool toDelete;
		string name;
		uint32_t crc;

		fileInfo();
		fileInfo(string str);
	};

	struct uiStruct {
		void *p;
		bool isGUI;
		size_t fileIndex;

		BOSS_COMMON_EXP uiStruct();
		BOSS_COMMON_EXP uiStruct(void *GUIpoint);
	};

	BOSS_COMMON_EXP extern vector<fileInfo> updatedFiles;  //The updated files. These don't have the .new extension.

	////////////////////////
	// General Functions
	////////////////////////

	//Checks if an Internet connection is present.
	BOSS_COMMON_EXP bool CheckConnection();

	//Cleans up after the user cancels a download.
	//Throws boss_error exception on fail.
	BOSS_COMMON_EXP void CleanUp();


	////////////////////////
	// Masterlist Updating
	////////////////////////

	//Updates the local masterlist to the latest available online.
	//Throws boss_error exception on fail.
	BOSS_COMMON_EXP void UpdateMasterlist(uiStruct ui, uint32_t& localRevision, string& localDate, uint32_t& remoteRevision, string& remoteDate);


	////////////////////////
	// BOSS Updating
	////////////////////////

	//Checks if a new release of BOSS is available or not.
	//Throws boss_error exception on fail.
	BOSS_COMMON_EXP string IsBOSSUpdateAvailable();

	//Gets the release notes for the update.
	//Throws boss_error exception on fail.
	BOSS_COMMON_EXP string FetchReleaseNotes(const string updateVersion);

	//Downloads and installs a BOSS update.
	//Throws boss_error exception on fail.
	BOSS_COMMON_EXP vector<string> DownloadInstallBOSSUpdate(uiStruct ui, const uint32_t updateType, const string updateVersion);
}
#endif