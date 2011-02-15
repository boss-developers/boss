/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

//Contains modlist class.

#ifndef __BOSS_MODLIST_H__
#define __BOSS_MODLIST_H__

#ifndef BOOST_FILESYSTEM_VERSION
#define BOOST_FILESYSTEM_VERSION 3
#endif

#ifndef BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_NO_DEPRECATED
#endif

#include <string>
#include <vector>
#include "boost/filesystem.hpp"

namespace fs = boost::filesystem;

namespace boss {
	using namespace std;

	//Date comparison, used for sorting mods in modlist class.
	bool SortByDate(fs::path mod1, fs::path mod2);

	//Modlist class.
	class Mods {
	public:
		vector<fs::path> mods;					//Stores the mods in your data folder.
		vector< vector<string> > modmessages;	//Stores the messages attached to each mod. First dimension matches up with the mods vector, then second lists the messages attached to that mod.
		void AddMods();							//Adds mods in directory to modlist in date order (AKA load order).
		void SaveModList();						//Save mod list to modlist.txt. Backs up old modlist.txt as modlist.old first.
		int GetModIndex(fs::path mod);			//Look for a mod in the modlist, even if the mod in question is ghosted.
	};
}

#endif