/*	General User Interface for BOSS (Better Oblivion Sorting Software)
	
	Providing a graphical frontend to BOSS's functions.

    Copyright (C) 2011 WrinklyNinja & the BOSS development team.
    http://creativecommons.org/licenses/by-nc-nd/3.0/


	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#ifndef __UPDATER__HPP__
#define __UPDATER__HPP__

#include <string>
#include <vector>
#include "boost/filesystem.hpp"

namespace boss {
	extern string updateVersion;
	extern vector<fs::path> updatedFiles;  //The updated files. These don't have the .new extension.

	//Exception class for updater functions.
	struct update_error: virtual exception, virtual boost::exception {};
	typedef boost::error_info<struct tag_errno,string> err_detail;

	//Checks if there is an Internet connection present.
	bool CheckConnection();

	//Checks if a new release of BOSS is available or not.
	string IsUpdateAvailable();

	//Downloads the installer for the update, for when the current version was installed via installer.
	void DownloadUpdateInstaller();

	//Download the files in the update.
	bool DownloadUpdateFiles();

	//Installs the downloaded update files.
	bool InstallUpdateFiles();
}
#endif