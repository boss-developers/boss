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

	$Revision: 3184 $, $Date: 2011-08-26 20:52:13 +0100 (Fri, 26 Aug 2011) $
*/

#include "Common/Globals.h"

namespace boss {
	using namespace std;
	
	//BOSS version number.
	BOSS_COMMON const uint32_t BOSS_VERSION_MAJOR	= 2;
	BOSS_COMMON const uint32_t BOSS_VERSION_MINOR	= 1;
	BOSS_COMMON const uint32_t BOSS_VERSION_PATCH	= 1;
	
	//Supported game values. DO NOT CHANGE THESE VALUES. THEY MUST BE CONSTANT FOR API USERS.
	BOSS_COMMON const uint32_t AUTODETECT			= 0;
	BOSS_COMMON const uint32_t OBLIVION				= 1;
	BOSS_COMMON const uint32_t NEHRIM				= 2;
	BOSS_COMMON const uint32_t SKYRIM				= 3;
	BOSS_COMMON const uint32_t FALLOUT3				= 4;
	BOSS_COMMON const uint32_t FALLOUTNV			= 5;
	BOSS_COMMON const uint32_t MORROWIND			= 6;

	//BOSS Log formatting values.
	BOSS_COMMON const uint32_t HTML					= 0;
	BOSS_COMMON const uint32_t PLAINTEXT			= 1;

	//Language values.
	BOSS_COMMON const uint32_t ENGLISH				= 0;
	BOSS_COMMON const uint32_t SPANISH				= 1;
	BOSS_COMMON const uint32_t GERMAN				= 2;
	BOSS_COMMON const uint32_t RUSSIAN				= 3;
	BOSS_COMMON const uint32_t SIMPCHINESE			= 4;
	
	
	///////////////////////////////
	//File/Folder Paths
	///////////////////////////////

	BOSS_COMMON const fs::path boss_path			= fs::path(".");
	BOSS_COMMON const fs::path ini_path				= boss_path / "BOSS.ini";
	BOSS_COMMON const fs::path old_ini_path			= boss_path / "BOSS.ini.old";
	BOSS_COMMON const fs::path debug_log_path		= boss_path / "BOSSDebugLog.txt";
	BOSS_COMMON const fs::path readme_path			= boss_path / "Docs" / "BOSS Readme.html";
	BOSS_COMMON const fs::path rules_readme_path	= boss_path / "Docs" / "BOSS Userlist Syntax.html";
	BOSS_COMMON const fs::path masterlist_doc_path	= boss_path / "Docs" / "BOSS Masterlist Syntax.html";
	BOSS_COMMON const fs::path api_doc_path			= boss_path / "Docs" / "BOSS API Readme.html";
	BOSS_COMMON const fs::path version_history_path = boss_path / "Docs" / "BOSS Version History.html";
	BOSS_COMMON const fs::path licenses_path		= boss_path / "Docs" / "Licenses.txt";


	///////////////////////////////
	//Ini Settings
	///////////////////////////////

	//General variables
	BOSS_COMMON bool		gl_do_startup_update_check	= true;
	BOSS_COMMON	bool		gl_use_user_rules_manager	= true;
	BOSS_COMMON	bool		gl_close_gui_after_sorting  = false;
	BOSS_COMMON uint32_t	gl_language					= ENGLISH;

	//Command line variables
	BOSS_COMMON string		gl_proxy_host				= "";
	BOSS_COMMON string		gl_proxy_user				= "";
	BOSS_COMMON string		gl_proxy_passwd				= "";
	BOSS_COMMON uint32_t	gl_proxy_port				= 0;
	BOSS_COMMON uint32_t	gl_log_format				= HTML;
	BOSS_COMMON uint32_t	gl_game						= AUTODETECT;
	BOSS_COMMON uint32_t	gl_last_game				= AUTODETECT;
	BOSS_COMMON uint32_t	gl_revert					= 0;
	BOSS_COMMON uint32_t	gl_debug_verbosity			= 0;
	BOSS_COMMON bool		gl_update					= true;
	BOSS_COMMON bool		gl_update_only				= false;
	BOSS_COMMON bool		gl_silent					= false;
	BOSS_COMMON bool		gl_debug_with_source		= false;
	BOSS_COMMON bool		gl_show_CRCs				= false;
	BOSS_COMMON bool		gl_trial_run				= false;
	BOSS_COMMON bool		gl_log_debug_output			= false;
}