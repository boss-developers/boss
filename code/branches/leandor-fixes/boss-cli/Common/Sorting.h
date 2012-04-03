/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 1767 $, $Date: 2010-10-30 18:46:25 +0100 (Sat, 30 Oct 2010) $
*/

#ifndef __BOSS_SORTING_H__
#define __BOSS_SORTING_H__


#include "Globals.h"
#include "Updater.h"
#include "Masterlist.h"

#include "Support/Types.h"
#include "Support/ModFormat.h"
#include "Support/Helpers.h"

#include <string>


namespace boss {
	using namespace std;

	/// ShowMessage(string textbuf, ...)
	///  - Produces a message to the BOSSLOG.txt
	void ShowMessage(string textbuf, int game);

	/// ReadLine (string file):
	///  - Read a line from a file. Could be rewritten better.
	string ReadLine (string file); 

	/// GetModHeader(string textbuf):
	///  - Reads the header from mod file and prints a string representation which includes the version text, if found.
	string GetModHeader(const string& filename, bool ghosted);
}

#endif
