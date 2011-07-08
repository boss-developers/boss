/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision$, $Date$
*/

#ifndef __BOSS_UPDATER_H__
#define __BOSS_UPDATER_H__

#include "Globals.h"
#include <boost/filesystem.hpp>

namespace boss {
	unsigned int GetLocalMasterlistRevision();

	unsigned int UpdateMasterlist();

	bool CheckConnection();
}

#endif
