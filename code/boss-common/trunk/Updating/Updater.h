/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 3184 $, $Date: 2011-08-26 20:52:13 +0100 (Fri, 26 Aug 2011) $
*/

#ifndef __BOSS_UPDATER_H__
#define __BOSS_UPDATER_H__

#ifndef BOOST_SPIRIT_UNICODE
#define BOOST_SPIRIT_UNICODE 
#endif

#include <string>
#include <vector>
#include "Common/DllDef.h"

namespace boss {
	using namespace std;

	BOSS_COMMON enum installType {  //Possible types of install the user has.
		MANUAL,
		INSTALLER,
		MASTERLIST
	};

	BOSS_COMMON struct fileInfo {  //Holds the information for files to be downloaded.
		string name;
		unsigned int crc;

		fileInfo();
		fileInfo(string str);
	};

	BOSS_COMMON struct uiStruct {
		void *p;
		bool isGUI;
		size_t fileIndex;

		uiStruct();
		uiStruct(void *GUIpoint);
	};

	BOSS_COMMON extern vector<fileInfo> updatedFiles;  //The updated files. These don't have the .new extension.

	////////////////////////
	// General Functions
	////////////////////////

	//Checks if an Internet connection is present.
	BOSS_COMMON bool CheckConnection();

	//Cleans up after the user cancels a download.
	BOSS_COMMON void CleanUp();


	////////////////////////
	// Masterlist Updating
	////////////////////////

	//Updates the local masterlist to the latest available online.
	BOSS_COMMON void UpdateMasterlist(uiStruct ui, unsigned int& localRevision, string& localDate, unsigned int& remoteRevision, string& remoteDate);


	////////////////////////
	// BOSS Updating
	////////////////////////

	//Checks if a new release of BOSS is available or not.
	BOSS_COMMON string IsBOSSUpdateAvailable();

	//Downloads and installs a BOSS update.
	BOSS_COMMON vector<string> DownloadInstallBOSSUpdate(uiStruct ui, const int updateType, const string updateVersion);
}
#endif