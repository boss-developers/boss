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

#define CURL_STATICLIB			//Tells the compiler to use curl as a static library.
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

namespace boss {

	void UpdateMasterlist(int game);

}

#endif
