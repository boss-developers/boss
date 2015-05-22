/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2002 $, $Date: 2010-12-05 21:35:32 +0000 (Sun, 05 Dec 2010) $
*/

#ifndef __BOSS_MASTERLIST_H__
#define __BOSS_MASTERLIST_H__


#include <string>


namespace boss {
	using namespace std;

	bool IsMod(wstring textbuf);
	bool IsMessage(wstring textbuf);
	bool IsValidLine(wstring textbuf);
}

#endif
