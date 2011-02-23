/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/
//Header file for userlist parser functions.

#ifndef __BOSS_USERLIST_PARSER_H__
#define __BOSS_USERLIST_PARSER_H__

#ifndef BOOST_FILESYSTEM_VERSION
#define BOOST_FILESYSTEM_VERSION 3
#endif

#ifndef BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_NO_DEPRECATED
#endif

#include <string>
#include "boost/filesystem.hpp"
#include "Common/Lists.h"

namespace boss {
	using namespace std;
	namespace fs = boost::filesystem;

	void fileToBuffer(fs::path file, string& buffer);

	bool parseUserlist(fs::path file, userlist& list);

	void RuleCheck(rule ruleToCheck);
}
#endif