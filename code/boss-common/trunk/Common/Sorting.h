/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision$, $Date$
*/

#ifndef __BOSS_SORTING_H__
#define __BOSS_SORTING_H__

#include <string>

#include <Support/Types.h>
#include <Support/ModFormat.h>
#include <Support/Helpers.h>

#include "Globals.h"
#include "Updater.h"
#include "Masterlist.h"

namespace boss {
	using namespace std;

	/// ShowMessage(string textbuf, ...)
	///  - Produces a message to the BOSSLOG.txt
	void ShowMessage(string textbuf, bool fcom, bool ooo, bool bc, bool fook2, bool fwe);

	/// ReadLine (string file):
	///  - Read a line from a file. Could be rewritten better.
	string ReadLine (string file); 

	/// GetModHeader(string textbuf):
	///  - Reads the header from mod file and prints a string representation which includes the version text, if found.
	string GetModHeader(const string& filename, bool ghosted);
};
#endif __BOSS_SORTING_H__