/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

	Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
	http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision$, $Date$
*/

#ifndef __BOSS_GLOBALS_H__
#define __BOSS_GLOBALS_H__

#ifndef BOOST_FILESYSTEM_VERSION
#define BOOST_FILESYSTEM_VERSION 3
#endif

#ifndef BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_NO_DEPRECATED
#endif

#define _UNICODE	// Tell compiler we're using Unicode, notice the _

#include <string>
#include <fstream>
#include <boost/filesystem.hpp>


namespace boss {
	using namespace std;
	namespace fs = boost::filesystem;

	extern ofstream bosslog;					//BOSSlog.html output file

	extern const fs::path data_path;			// Holds the path to the data directory.
	extern const fs::path bosslog_path;			// BOSSlog full file name
	extern const fs::path masterlist_path;		// Hold both location and file name for masterlist.txt
	extern const fs::path userlist_path;		// Hold both location and file name for userlist.txt 
	extern const fs::path curr_modlist_path;	// Hold both location and file name for modlist.txt
	extern const fs::path prev_modlist_path;	// Hold both location and file name for modlist.old
}

#endif
