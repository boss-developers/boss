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
//#include <wx/progdlg.h>

namespace boss {
	using namespace std;

	struct fileInfo {  //Holds the information for files to be downloaded.
		string name;
		unsigned int crc;
	};

	enum installType {  //Possible types of install the user has.
		MANUAL,
		INSTALLER
	};

	extern vector<fileInfo> updatedFiles;

	//Checks if an Internet connection is present.
	bool CheckConnection();

	//Gets the revision number of the local masterlist.
	int GetLocalMasterlistRevision();

	//Gets the revision date of the local masterlist.
	string GetLocalMasterlistDate();

	//Updates the local masterlist to the latest available online.
	int UpdateMasterlist();

	//Checks if a new release of BOSS is available or not.
	string IsBOSSUpdateAvailable();

	//Download the files in the update.
//	void DownloadBOSSUpdateFiles(int installType, wxProgressDialog *progDia);

	//Installs the downloaded update files.
	void InstallBOSSUpdateFiles();

	//Cleans up after the user cancels a download.
	void CleanUp();
}

#endif
