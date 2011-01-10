/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

	Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
	http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision$, $Date$
*/

#ifndef __BOSS_GLOBALS_H__
#define __BOSS_GLOBALS_H__

#define BOOST_FILESYSTEM_VERSION 3
#define BOOST_FILESYSTEM_NO_DEPRECATED
#define _UNICODE	// Tell C we're using Unicode, notice the _

#include <string>
#include <fstream>
#include <boost/filesystem.hpp>
#include "../utf8/source/utf8.h"


namespace boss {

	using namespace std;
	namespace fs = boost::filesystem;

	extern ifstream order;						//masterlist.txt - the grand mod order list
	extern ofstream bosslog;					//BOSSlog.txt - output file.
	extern bool fcom;							//true if key FCOM or FOOK2 files are found.
	extern bool ooo;							//true if OOO or FWE esm is found.
	extern bool bc;								//true if Better Cities esm is found.

	extern const fs::path data_path;			// Holds the path to the data directory.
	extern const fs::path bosslog_path;			// BOSSlog full file name
	extern const fs::path masterlist_path;		// Hold both location and file name for masterlist.txt
	extern const fs::path userlist_path;		// Hold both location and file name for userlist.txt 
	extern const fs::path curr_modlist_path;	// Hold both location and file name for modlist.txt
	extern const fs::path prev_modlist_path;	// Hold both location and file name for modlist.old
}

#endif
