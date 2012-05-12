/*	BOSS
	
	A "one-click" program for users that quickly optimises and avoids 
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge, 
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

    Copyright (C) 2009-2012    BOSS Development Team.

	This file is part of BOSS.

    BOSS is free software: you can redistribute 
	it and/or modify it under the terms of the GNU General Public License 
	as published by the Free Software Foundation, either version 3 of 
	the License, or (at your option) any later version.

    BOSS is distributed in the hope that it will 
	be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with BOSS.  If not, see 
	<http://www.gnu.org/licenses/>.

	$Revision: 3135 $, $Date: 2011-08-17 22:01:17 +0100 (Wed, 17 Aug 2011) $
*/

#ifndef __BOSS_GAME_H__
#define __BOSS_GAME_H__

#include <string>
#include <boost/filesystem.hpp>
#include <boost/cstdint.hpp>
#include "Common/Classes.h"

namespace boss {
	using namespace std;
	namespace fs = boost::filesystem;
 
	BOSS_COMMON uint32_t	DetectGame(vector<uint32_t>& detectedGames, vector<uint32_t>& undetectedGames);	//Throws exception if error.

	class BOSS_COMMON Game {
	public:
		Game();  //Sets game to AUTODETECT, with all other vars being empty.
		Game(uint32_t inGame, string dataFolder = "", bool noPathInit = false); //Empty dataFolder means constructor will detect its location. If noPathInit is true, then the data, active plugins list and loadorder.txt paths will not be set, and the game's BOSS subfolder will not be created.
		
		bool IsInstalled() const;
		bool IsInstalledLocally() const;
		
		uint32_t GetGame() const;
		string Name() const;  //Returns the game's name, eg. "TES IV: Oblivion".
		Item MasterFile() const;  //Returns the game's master file. To get its timestamp, use .GetModTime() on it.
		
		fs::path DataFolder() const;
		fs::path ActivePluginsFile() const;
		fs::path LoadOrderFile() const;
		fs::path Masterlist() const;
		fs::path Userlist() const;
		fs::path Modlist() const;
		fs::path OldModlist() const;
		fs::path BossLog(uint32_t format) const;
		
	private:
		uint32_t game;
		string name;
		string masterFile;
	
		string registryKey;
		string registrySubKey;
		
		string bossFolderName;
		string appdataFolderName;
		string pluginsFolderName;
		string pluginsFileName;
		
		fs::path dataPath;  //Path to the game's plugins folder.
		fs::path pluginsPath;  //Path to the file in which active plugins are listed.
		fs::path loadorderPath;  //Path to the file which lists total load order.

		//Each game also has a masterlist, userlist, modlist and BOSS Log associated with them. Make these members?

		//Can be used to get the location of the LOCALAPPDATA folder (and its Windows XP equivalent).
		fs::path GetLocalAppDataPath();
	};
}

#endif