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

#ifndef __BOSS_GLOBALS_H__
#define __BOSS_GLOBALS_H__

#ifndef _UNICODE
#define _UNICODE	// Tell compiler we're using Unicode, notice the _
#endif

#include <string>
#include <boost/filesystem.hpp>
#include <boost/cstdint.hpp>
#include "Common/DllDef.h"
#include "Common/Error.h"

namespace boss {
	using namespace std;
	namespace fs = boost::filesystem;

//#define BOSSGUI

	BOSS_COMMON const uint32_t BOSS_VERSION_MAJOR = 2;
	BOSS_COMMON const uint32_t BOSS_VERSION_MINOR = 0;
	BOSS_COMMON const uint32_t BOSS_VERSION_PATCH = 1;

	BOSS_COMMON extern const string gl_boss_release_date;
	
	//DO NOT CHANGE THE GAME VALUES. THEY MUST BE INVARIANT ACROSS RELEASES FOR API USERS.
	BOSS_COMMON const uint32_t AUTODETECT	= 0;
	BOSS_COMMON const uint32_t OBLIVION		= 1;
	BOSS_COMMON const uint32_t NEHRIM		= 2;
	BOSS_COMMON const uint32_t SKYRIM		= 3;
	BOSS_COMMON const uint32_t FALLOUT3		= 4;
	BOSS_COMMON const uint32_t FALLOUTNV	= 5;
	BOSS_COMMON const uint32_t MORROWIND	= 6;

	//These other uint32_t values are not (as yet) accessed by the API, so don't need to be
	//invariant across versions.
	enum : uint32_t {
		//BOSS Log Formats (for 'log_format' setting)
		HTML,
		PLAINTEXT,
		//Run types (for 'run_type' setting)
		SORT,
		UPDATE,
		UNDO
	};

	BOSS_COMMON extern uint32_t gl_current_game;  //The game that BOSS is currently running for. Matches gl_game if gl_game != AUTODETECT.

	BOSS_COMMON extern bool gl_using_local_app_data_folder;  //Set by bUseMyGamesDirectory in the game's ini file if different from default.


	///////////////////////////////
	//File/Folder Paths
	///////////////////////////////

	//These globals exist for ease-of-use, so that a Settings object doesn't need to be passed in infinity+1 functions.
	BOSS_COMMON extern const fs::path boss_path;		//Path to the BOSS folder, relative to executable (ie. '.').
	BOSS_COMMON extern		 fs::path data_path;		//Path to the data folder of the game that BOSS is currently running for, relative to executable (ie. boss_path).
	BOSS_COMMON extern const fs::path ini_path;
	BOSS_COMMON extern const fs::path old_ini_path;
	BOSS_COMMON extern const fs::path debug_log_path;
	BOSS_COMMON extern const fs::path readme_path;
	BOSS_COMMON extern const fs::path rules_readme_path;
	BOSS_COMMON extern const fs::path masterlist_doc_path;
	BOSS_COMMON extern const fs::path api_doc_path;
	BOSS_COMMON extern const fs::path licenses_path;


	///////////////////////////////
	//File/Folder Path Functions
	///////////////////////////////

	BOSS_COMMON fs::path boss_game_path();		//Path to BOSS's folder for that game.
	BOSS_COMMON fs::path bosslog_path();		//Output decided by log format.
	BOSS_COMMON fs::path masterlist_path();		//Output decided by game.
	BOSS_COMMON fs::path userlist_path();		//Output decided by game.
	BOSS_COMMON fs::path modlist_path();		//Output decided by game.
	BOSS_COMMON fs::path old_modlist_path();	//Output decided by game.

	BOSS_COMMON fs::path plugins_path();		//Path to plugins.txt.
	BOSS_COMMON fs::path loadorder_path();		//Path to loadorder.txt.
	

	///////////////////////////////
	//Ini Settings
	///////////////////////////////

	//General variables
	BOSS_COMMON extern bool		gl_do_startup_update_check;	// Whether or not to check for updates on startup.
	BOSS_COMMON extern bool		gl_use_user_rules_editor;	// Use the User Rules Editor or edit userlist.txt directly?

	//Command line variables.
	BOSS_COMMON extern string	gl_proxy_host;
	BOSS_COMMON extern string	gl_proxy_user;
	BOSS_COMMON extern string	gl_proxy_passwd;
	BOSS_COMMON extern uint32_t gl_proxy_port;
	BOSS_COMMON extern uint32_t gl_log_format;				// what format the output should be in.  Uses the enums defined above.
	BOSS_COMMON extern uint32_t gl_game;					// What game's mods are we sorting? Uses the enums defined above.
	BOSS_COMMON extern uint32_t gl_revert;					// what level to revert to
	BOSS_COMMON extern uint32_t gl_debug_verbosity;			// log levels above INFO to output
	BOSS_COMMON extern bool		gl_update;					// update the masterlist?
	BOSS_COMMON extern bool		gl_update_only;				// only update the masterlist and don't sort currently.
	BOSS_COMMON extern bool		gl_silent;					// silent mode?
	BOSS_COMMON extern bool		gl_skip_version_parse;		// enable parsing of mod's headers to look for version strings
	BOSS_COMMON extern bool		gl_debug_with_source;		// whether to include origin information in logging statements
	BOSS_COMMON extern bool		gl_show_CRCs;				// whether or not to show mod CRCs.
	BOSS_COMMON extern bool		gl_trial_run;				// If true, don't redate files.
	BOSS_COMMON extern bool		gl_log_debug_output;		// If true, logs command line output in BOSSDebugLog.txt.


	///////////////////////////////
	//Settings Functions
	///////////////////////////////
	
	BOSS_COMMON string	GetGameString		(uint32_t game);
	BOSS_COMMON string	GetGameMasterFile	(uint32_t game);
	BOSS_COMMON void SetDataPath(uint32_t game);
	BOSS_COMMON vector<uint32_t>	DetectGame			(void * parent);  //Throws exception if error.
	BOSS_COMMON time_t	GetMasterTime		();  //Throws exception if error.


	///////////////////////////////
	//Settings Class
	///////////////////////////////

	struct IniPair {
		string key;
		string value;
	};

	class BOSS_COMMON Settings {
	private:
		ParsingError errorBuffer;
		vector<IniPair> iniSettings;
		string iniFile;

		string	GetIniGameString	() const;
		string	GetLogFormatString	() const;
		void ApplyIniSettings();
	public:
		void	Load(fs::path file);		//Throws exception on fail.
		void	Save(fs::path file);		//Throws exception on fail.

		ParsingError ErrorBuffer() const;
		void ErrorBuffer(ParsingError buffer);
	};
}

#endif
