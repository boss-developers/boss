/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 1767 $, $Date: 2010-10-30 18:46:25 +0100 (Sat, 30 Oct 2010) $
*/

#ifndef __BOSS_MASTERLIST_H__
#define __BOSS_MASTERLIST_H__


#include <string>


namespace boss {
	using namespace std;

	bool IsMod(string textbuf);
	bool IsMessage(string textbuf);
	bool IsValidLine(string textbuf);
}

#endif
