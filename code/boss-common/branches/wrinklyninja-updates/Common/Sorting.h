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

namespace boss {
	using namespace std;

	/// ShowMessage(string textbuf, ...)
	///  - Produces a message to the BOSSLOG.txt
	void ShowMessage(string textbuf, int game);

	/// ReadLine (string file):
	///  - Read a line from a file. Could be rewritten better.
	string ReadLine (string file); 
}

#endif
