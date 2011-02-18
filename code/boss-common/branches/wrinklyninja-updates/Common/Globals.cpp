/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

	Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
	http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision$, $Date$
*/

#include "Globals.h"

namespace boss {
	using namespace std;

	ofstream bosslog;					//BOSSlog.txt - output file.

	const fs::path data_path			= fs::path("..") / "Data";
	const fs::path bosslog_path			= "BOSSlog.html";
	const fs::path masterlist_path		= "masterlist.txt";
	const fs::path userlist_path		= "userlist.txt";
	const fs::path curr_modlist_path	= "modlist.txt";
	const fs::path prev_modlist_path	= "modlist.old";
}