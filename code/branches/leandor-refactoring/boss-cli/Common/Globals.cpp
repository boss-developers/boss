/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

	Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
	http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 1827 $, $Date: 2010-11-06 20:48:12 +0000 (Sat, 06 Nov 2010) $
*/

#include "Globals.h"

namespace boss {
	using namespace std;

	ifstream order;						//masterlist.txt - the grand mod order list
	ofstream bosslog;					//BOSSlog.txt - output file.
	bool fcom;							//true if key FCOM or FOOK2 files are found.
	bool ooo;							//true if OOO or FWE esm is found.
	bool bc;							//true if Better Cities esm is found.

	const fs::path boss_path			= "BOSS";	
	const fs::path bosslog_path			= boss_path / "BOSSlog.html";
	const fs::path masterlist_path		= boss_path / "masterlist.txt";
	const fs::path userlist_path		= boss_path / "userlist.txt";
	const fs::path curr_modlist_path	= boss_path / "modlist.txt";
	const fs::path prev_modlist_path	= boss_path / "modlist.old";
}