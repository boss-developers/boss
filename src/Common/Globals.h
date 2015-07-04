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

#ifndef COMMON_GLOBALS_H_
#define COMMON_GLOBALS_H_

#ifndef _UNICODE
#define _UNICODE  // Tell compiler we're using Unicode, notice the _
#endif

#include <string>

#include <boost/cstdint.hpp>
#include <boost/filesystem.hpp>

#include "Common/DllDef.h"
#include "Common/Game.h"

namespace boss {

using namespace std;
namespace fs = boost::filesystem;

// BOSS version number.
BOSS_COMMON extern const uint32_t BOSS_VERSION_MAJOR;
BOSS_COMMON extern const uint32_t BOSS_VERSION_MINOR;
BOSS_COMMON extern const uint32_t BOSS_VERSION_PATCH;

/*
 * MCP Note: Possibly change these to enums?
 * For all these notes, need to fully investigate the scope.
 */
// Supported game values.
BOSS_COMMON extern const uint32_t AUTODETECT;
BOSS_COMMON extern const uint32_t OBLIVION;
BOSS_COMMON extern const uint32_t NEHRIM;
BOSS_COMMON extern const uint32_t SKYRIM;
BOSS_COMMON extern const uint32_t FALLOUT3;
BOSS_COMMON extern const uint32_t FALLOUTNV;

// BOSS Log formatting values.
BOSS_COMMON extern const uint32_t HTML;
BOSS_COMMON extern const uint32_t PLAINTEXT;

// Language values.
BOSS_COMMON extern const uint32_t ENGLISH;
BOSS_COMMON extern const uint32_t SPANISH;
BOSS_COMMON extern const uint32_t GERMAN;
BOSS_COMMON extern const uint32_t RUSSIAN;
BOSS_COMMON extern const uint32_t SIMPCHINESE;


///////////////////////////////
// File/Folder Paths
///////////////////////////////

// Paths that are game-invariant.
BOSS_COMMON extern const fs::path boss_path;  // Path to the BOSS folder, relative to executable (ie. '.').
BOSS_COMMON extern const fs::path ini_path;
BOSS_COMMON extern const fs::path old_ini_path;
BOSS_COMMON extern const fs::path debug_log_path;
BOSS_COMMON extern const fs::path readme_path;
BOSS_COMMON extern const fs::path rules_readme_path;
BOSS_COMMON extern const fs::path masterlist_doc_path;
BOSS_COMMON extern const fs::path api_doc_path;
BOSS_COMMON extern const fs::path version_history_path;
BOSS_COMMON extern const fs::path licenses_path;
BOSS_COMMON extern const fs::path l10n_path;


///////////////////////////////
// Ini Settings
///////////////////////////////
// These globals exist for ease-of-use, so that a Settings object doesn't need to be passed in infinity+1 functions.

// General variables
BOSS_COMMON extern bool gl_use_user_rules_manager;   // Use the User Rules Editor or edit userlist.txt directly?
BOSS_COMMON extern bool gl_close_gui_after_sorting;  // Close the GUI after BOSS has finished running or not.
BOSS_COMMON extern uint32_t gl_language;             // What language to run BOSS in?

// Command line variables
BOSS_COMMON extern uint32_t gl_log_format;       // What format the output should be in.  Uses the enums defined above.
BOSS_COMMON extern uint32_t gl_game;             // What game's mods are we sorting? Uses the enums defined above.
BOSS_COMMON extern uint32_t gl_last_game;        // What game was last run? Only affects GUI behaviour.
BOSS_COMMON extern uint32_t gl_revert;           // What level to revert to
BOSS_COMMON extern uint32_t gl_debug_verbosity;  // Log levels above INFO to output
BOSS_COMMON extern bool gl_update;               // Update the masterlist?
BOSS_COMMON extern bool gl_update_only;          // Only update the masterlist and don't sort currently.
BOSS_COMMON extern bool gl_silent;               // Silent mode?
BOSS_COMMON extern bool gl_show_CRCs;            // Whether or not to show mod CRCs.
BOSS_COMMON extern bool gl_trial_run;            // If true, don't redate files.

// Repository URLs
BOSS_COMMON extern std::string gl_oblivion_repo_url;
BOSS_COMMON extern std::string gl_nehrim_repo_url;
BOSS_COMMON extern std::string gl_skyrim_repo_url;
BOSS_COMMON extern std::string gl_fallout3_repo_url;
BOSS_COMMON extern std::string gl_falloutnv_repo_url;

}  // namespace boss
#endif  // COMMON_GLOBALS_H_
